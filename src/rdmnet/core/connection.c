/******************************************************************************
 * Copyright 2019 ETC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************
 * This file is a part of RDMnet. For more information, go to:
 * https://github.com/ETCLabs/RDMnet
 *****************************************************************************/

#include "rdmnet/core/connection.h"

#include <stdint.h>
#include "etcpal/common.h"
#include "etcpal/lock.h"
#include "etcpal/rbtree.h"
#include "etcpal/socket.h"
#include "rdmnet/core/message.h"
#include "rdmnet/core/broker_prot.h"
#include "rdmnet/core/common.h"
#include "rdmnet/core/message.h"
#include "rdmnet/core/util.h"
#include "rdmnet/defs.h"
#include "rdmnet/private/opts.h"

#if RDMNET_DYNAMIC_MEM
#include <stdlib.h>
#else
#include "etcpal/mempool.h"
#endif

#if RDMNET_USE_TICK_THREAD
#include "etcpal/thread.h"
#endif

/*************************** Private constants *******************************/

#define RDMNET_CONN_MAX_SOCKETS ETCPAL_SOCKET_MAX_POLL_SIZE

#define INITIAL_CONNECTION_CAPACITY 8

/***************************** Private types ********************************/

typedef enum
{
  kRCConnEventNone,
  kRCConnEventConnected,
  kRCConnEventConnectFailed,
  kRCConnEventDisconnected,
  kRCConnEventMsgReceived
} rc_conn_event_t;

typedef struct RCConnEvent
{
  rc_conn_event_t which;

  union
  {
    RCConnectedInfo connected;
    RCConnectFailedInfo connect_failed;
    RCDisconnectedInfo disconnected;
    RdmnetMessage* message;
  } arg;
} RCConnEvent;

#define RC_CONN_EVENT_INIT \
  {                        \
    kRCConnEventNone       \
  }

typedef struct ConnectionLists
{
#ifdef RDMNET_DYNAMIC_MEM
  RCClientConnection** pending;
  size_t pending_capacity;
  RCClientConnection** active;
  size_t active_capacity;
  RCClientConnection** to_destroy;
  size_t to_destroy_capacity;
#else
  RCClientConnection* pending[RDMNET_MAX_CONNECTIONS];
  RCClientConnection* active[RDMNET_MAX_CONNECTIONS];
  RCClientConnection* to_destroy[RDMNET_MAX_CONNECTIONS];
#endif
  size_t num_pending;
  size_t num_active;
  size_t num_to_destroy;
} ConnectionLists;

/***************************** Private macros ********************************/

#if RDMNET_DYNAMIC_MEM
#define ADD_CONNECTION(list_name, conn_ptr)                                                                      \
  add_connection_dynamic(connections.list_name, &connections.list_name##_capacity, &connections.num_##list_name, \
                         (conn_ptr))
#define REMOVE_CONNECTION(list_name, conn_ptr)                                                                      \
  remove_connection_dynamic(connections.list_name, &connections.list_name##_capacity, &connections.num_##list_name, \
                            (conn_ptr))
#else
#define ADD_CONNECTION(list_name, conn_ptr) \
  add_connection_static(connections.list_name, &connections.list_name##_size, (conn_ptr))
#define REMOVE_CONNECTION(list_name, conn_ptr) \
  remove_connection_static(connections.list_name, &connections.list_name##_size, (conn_ptr))
#endif

#define FOR_EACH_PENDING_CONN()                                                        \
  size_t i = 0;                                                                        \
  for (RCClientConnection* conn = connections.pending[i]; i < connections.num_pending; \
       ++i, conn = connections.pending[i])
#define FOR_EACH_ACTIVE_CONN() \
  size_t i = 0;                \
  for (RCClientConnection* conn = connections.active[i]; i < connections.num_active; ++i, conn = connections.active[i])
#define FOR_EACH_CONN_TO_DESTROY()                                                           \
  size_t i = 0;                                                                              \
  for (RCClientConnection* conn = connections.to_destroy[i]; i < connections.num_to_destroy; \
       ++i, conn = connections.to_destroy[i])

#define RC_CONN_LOCK(conn_ptr) etcpal_mutex_lock((conn_ptr)->lock)
#define RC_CONN_UNLOCK(conn_ptr) etcpal_mutex_unlock((conn_ptr)->lock)

/**************************** Private variables ******************************/

static ConnectionLists connections;

/*********************** Private function prototypes *************************/

static uint32_t update_backoff(uint32_t previous_backoff);
static void start_tcp_connection(RCClientConnection* conn, RCConnEvent* event);
static void start_rdmnet_connection(RCClientConnection* conn);
static void reset_connection(RCClientConnection* conn);
static void retry_connection(RCClientConnection* conn);

static void destroy_marked_connections(void);
static void add_pending_connections(void);
static void process_all_connection_state(void);
static void destroy_all_connections(void);

static void socket_activity_callback(const EtcPalPollEvent* event, PolledSocketOpaqueData data);

static void handle_socket_error(RCClientConnection* conn, etcpal_error_t socket_err);
static void handle_socket_data_received(RCClientConnection* conn, const uint8_t* data, size_t data_size);
static etcpal_error_t parse_single_message(RCClientConnection* conn, const uint8_t* data, size_t data_size);
static void handle_rdmnet_message(RCClientConnection* conn, RdmnetMessage* msg, RCConnEvent* cb);
static void handle_rdmnet_connect_result(RCClientConnection* conn, RdmnetMessage* msg, RCConnEvent* cb);
static void deliver_event_callback(RCClientConnection* conn, RCConnEvent* event);

// Connection management, lookup, destruction
static void cleanup_connection_resources(RCClientConnection* conn);

#if RDMNET_DYNAMIC_MEM
static bool add_connection_dynamic(RCClientConnection** list, size_t* list_capacity, size_t* list_size,
                                   RCClientConnection* to_add);
static void remove_connection_dynamic(RCClientConnection** list, size_t* list_capacity, size_t* list_size,
                                      RCClientConnection* to_remove);
#else
static bool add_connection_static(RCClientConnection** list, size_t* list_size, RCClientConnection* to_add);
static void remove_connection_static(RCClientConnection** list, size_t* list_size, RCClientConnection* to_remove);
#endif
static int find_connection_index(RCClientConnection* conn, const RCClientConnection** list, size_t list_size);

/*************************** Function definitions ****************************/

/*
 * Initialize the RDMnet Connection module. Do all necessary initialization before other RDMnet
 * Connection API functions can be called. This private function is called from rdmnet_core_init().
 */
etcpal_error_t rc_conn_module_init(void)
{
  etcpal_error_t res = kEtcPalErrOk;

#if RDMNET_DYNAMIC_MEM
  connections.pending = calloc(INITIAL_CONNECTION_CAPACITY, sizeof(RCClientConnection*));
  connections.active = calloc(INITIAL_CONNECTION_CAPACITY, sizeof(RCClientConnection*));
  connections.to_destroy = calloc(INITIAL_CONNECTION_CAPACITY, sizeof(RCClientConnection*));

  if (!connections.pending || !connections.active || !connections.to_destroy)
  {
    res = kEtcPalErrNoMem;

    if (connections.pending)
      free(connections.pending);
    if (connections.active)
      free(connections.active);
    if (connections.to_destroy)
      free(connections.to_destroy);

    memset(&connections, 0, sizeof connections);
  }
  else
  {
    connections.active_capacity = INITIAL_CONNECTION_CAPACITY;
    connections.pending_capacity = INITIAL_CONNECTION_CAPACITY;
    connections.to_destroy_capacity = INITIAL_CONNECTION_CAPACITY;
    connections.num_active = 0;
    connections.num_pending = 0;
    connections.num_to_destroy = 0;
  }
#endif

  return res;
}

static void destroy_all_connections(void)
{
  FOR_EACH_PENDING_CONN() { cleanup_connection_resources(conn); }
}

/*
 * Deinitialize the RDMnet Connection module, setting it back to an uninitialized state. All
 * existing connections will be closed/disconnected. Calls to other RDMnet Connection API
 * functions will fail until rdmnet_init() is called again. This private function is called from
 * rdmnet_core_deinit().
 */
void rc_conn_module_deinit()
{
  destroy_all_connections();

#if RDMNET_DYNAMIC_MEM
  if (connections.pending)
    free(connections.pending);
  if (connections.active)
    free(connections.active);
  if (connections.to_destroy)
    free(connections.to_destroy);
#endif

  memset(&connections, 0, sizeof connections);
}

/*!
 * \brief Create a new handle to use for an RDMnet Connection.
 *
 * This function simply allocates a connection handle - use rdmnet_connect() to actually start the
 * connection process.
 *
 * \param[in] config Configuration parameters for the connection to be created.
 * \param[out] handle Handle to the newly-created connection
 * \return #kEtcPalErrOk: Handle created successfully.
 * \return #kEtcPalErrInvalid: Invalid argument provided.
 * \return #kEtcPalErrNotInit: Module not initialized.
 * \return #kEtcPalErrNoMem: No room to allocate additional connection.
 * \return #kEtcPalErrSys: An internal library or system call error occurred.
 */
etcpal_error_t rc_client_conn_register(RCClientConnection* conn)
{
  if (!rdmnet_core_initialized())
    return kEtcPalErrNotInit;

  if (!ADD_CONNECTION(pending, conn))
    return kEtcPalErrNoMem;

  conn->is_blocking = true;
  conn->poll_info.callback = socket_activity_callback;
  conn->poll_info.data.ptr = conn;

  conn->state = kRCConnStateNotStarted;
  etcpal_timer_start(&conn->backoff_timer, 0);
  conn->rdmnet_conn_failed = false;

  conn->core.local_cid = kEtcPalNullUuid;
  conn->core.sock = ETCPAL_SOCKET_INVALID;
  ETCPAL_IP_SET_INVALID(&conn->core.remote_addr.ip);
  rc_conn_init(&conn->core);

  return kEtcPalErrOk;
}

void rc_conn_init(RCConnection* conn)
{
  etcpal_timer_start(&conn->send_timer, E133_TCP_HEARTBEAT_INTERVAL_SEC);
  etcpal_timer_start(&conn->hb_timer, E133_HEARTBEAT_TIMEOUT_SEC);
  rc_msg_buf_init(&conn->recv_buf);
}

/*
 * Connect to an RDMnet Broker.
 *
 * Starts the connection process from the background thread. Handles redirections automatically. On
 * failure, calling this function again on the same connection will wait for the backoff time
 * required by the standard before reconnecting.
 *
 * conn: Connection instance to connect. Must have been previously registered using
 *       rc_connection_register().
 * remote_addr: Broker's IP address and port.
 * connect_data: The information about this client that will be sent to the Broker as part of the
 *               connection handshake.
 * \return #kEtcPalErrOk: Connection started successfully.
 * \return #kEtcPalErrInvalid: Invalid argument provided.
 * \return #kEtcPalErrIsConn: Connection already in progress.
 */
etcpal_error_t rc_client_conn_connect(RCClientConnection* conn, const EtcPalUuid* local_cid,
                                      const EtcPalSockAddr* remote_addr, const BrokerClientConnectMsg* connect_data)
{
  RDMNET_ASSERT(conn);
  RDMNET_ASSERT(remote_addr);
  RDMNET_ASSERT(connect_data);

  if (conn->state != kRCConnStateNotStarted)
    return kEtcPalErrIsConn;

  // Set the data - the connect will be initiated from the background thread.
  conn->core.local_cid = *local_cid;
  conn->core.remote_addr = *remote_addr;
  conn->conn_data = *connect_data;
  conn->state = kRCConnStatePending;

  return kEtcPalErrOk;
}

/*!
 * \brief ADVANCED USAGE: Attach an RDMnet connection handle to an already-connected system socket.
 *
 * This function is typically only used by brokers. The RDMnet connection is assumed to have
 * already completed and be at the Heartbeat stage.
 *
 * \param[in] handle Connection handle to attach the socket to. Must have been previously created
 *                   using rdmnet_connection_create().
 * \param[in] sock System socket to attach to the connection handle. Must be an already-connected
 *                 stream socket.
 * \param[in] remote_addr The remote network address to which the socket is currently connected.
 * \return #kEtcPalErrOk: Socket was attached successfully.
 * \return #kEtcPalErrInvalid: Invalid argument provided.
 * \return #kEtcPalErrNotInit: Module not initialized.
 * \return #kEtcPalErrIsConn: The connection handle provided is already connected using another socket.
 * \return #kEtcPalErrNotImpl: RDMnet has been compiled with #RDMNET_ALLOW_EXTERNALLY_MANAGED_SOCKETS=0
 *         and thus this function is not available.
 * \return #kEtcPalErrSys: An internal library or system call error occurred.
 */
// etcpal_error_t rdmnet_attach_existing_socket(rc_connection_t handle, etcpal_socket_t sock,
//                                             const EtcPalSockAddr* remote_addr)
//{
//#if RDMNET_ALLOW_EXTERNALLY_MANAGED_SOCKETS
//  if (sock == ETCPAL_SOCKET_INVALID || !remote_addr)
//    return kEtcPalErrInvalid;
//
//  RCClientConnection* conn;
//  etcpal_error_t res = get_conn(handle, &conn);
//  if (res == kEtcPalErrOk)
//  {
//    if (conn->state != kRCConnStateNotStarted)
//    {
//      res = kEtcPalErrIsConn;
//    }
//    else
//    {
//      conn->sock = sock;
//      conn->remote_addr = *remote_addr;
//      conn->state = kRCConnStateHeartbeat;
//      conn->external_socket_attached = true;
//      etcpal_timer_start(&conn->send_timer, E133_TCP_HEARTBEAT_INTERVAL_SEC * 1000);
//      etcpal_timer_start(&conn->hb_timer, E133_HEARTBEAT_TIMEOUT_SEC * 1000);
//    }
//    release_conn(conn);
//  }
//  return res;
//#else   // RDMNET_ALLOW_EXTERNALLY_MANAGED_SOCKETS
//  ETCPAL_UNUSED_ARG(handle);
//  ETCPAL_UNUSED_ARG(sock);
//  ETCPAL_UNUSED_ARG(remote_addr);
//  return kEtcPalErrNotImpl;
//#endif  // RDMNET_ALLOW_EXTERNALLY_MANAGED_SOCKETS
//}

/*!
 * \brief Destroy an RDMnet connection handle.
 *
 * If the connection is currently healthy, call rdmnet_disconnect() first to do a graceful
 * RDMnet-level disconnect.
 *
 * \param[in] handle Connection handle to destroy.
 * \param[in] disconnect_reason If not NULL, an RDMnet Disconnect message will be sent with this
 *                              reason code. This is the proper way to gracefully close a
 *                              connection in RDMnet.
 */
void rc_client_conn_unregister(RCClientConnection* conn, const rdmnet_disconnect_reason_t* disconnect_reason)
{
  int index = find_connection_index(conn, connections.active, connections.num_active);
  if (index >= 0)
  {
    if (conn->state == kRCConnStateHeartbeat && disconnect_reason)
    {
      BrokerDisconnectMsg dm;
      dm.disconnect_reason = *disconnect_reason;
      rc_broker_send_disconnect(&conn->core, &dm);
    }
    conn->state = kRCConnStateMarkedForDestruction;
    ADD_CONNECTION(to_destroy, conn);
  }
}

/*!
 * \brief Send data on an RDMnet connection.
 *
 * Thin wrapper over the underlying socket send function. Use rdmnet_set_blocking() to control the
 * blocking behavior of this send.
 *
 * \param[in] handle Connection handle on which to send.
 * \param[in] data Data buffer to send.
 * \param[in] size Size of data buffer.
 * \return Number of bytes sent (success)
 * \return #kEtcPalErrInvalid: Invalid argument provided.
 * \return #kEtcPalErrNotInit: Module not initialized.
 * \return #kEtcPalErrNotConn: The connection handle has not been successfully connected.
 * \return #kEtcPalErrSys: An internal library or system call error occurred.
 * \return Note: Other error codes might be propagated from underlying socket calls.
 */
int rc_client_conn_send(RCClientConnection* conn, const uint8_t* data, size_t size)
{
  RDMNET_ASSERT(conn);
  RDMNET_ASSERT(data);
  RDMNET_ASSERT(size != 0);

  if (conn->state != kRCConnStateHeartbeat)
    return kEtcPalErrNotConn;
  else
    return etcpal_send(conn->core.sock, data, size, 0);
}

void start_rdmnet_connection(RCClientConnection* conn)
{
  if (conn->is_blocking)
    etcpal_setblocking(conn->core.sock, true);

  // Update state
  conn->state = kRCConnStateRDMnetConnPending;
  rdmnet_core_modify_polled_socket(conn->core.sock, ETCPAL_POLL_IN, &conn->poll_info);
  rc_broker_send_client_connect(&conn->core, &conn->conn_data);
  etcpal_timer_start(&conn->core.hb_timer, E133_HEARTBEAT_TIMEOUT_SEC * 1000);
  etcpal_timer_start(&conn->core.send_timer, E133_TCP_HEARTBEAT_INTERVAL_SEC * 1000);
}

void start_tcp_connection(RCClientConnection* conn, RCConnEvent* event)
{
  bool ok = true;

  etcpal_error_t res = etcpal_socket(ETCPAL_IP_IS_V6(&conn->core.remote_addr.ip) ? ETCPAL_AF_INET6 : ETCPAL_AF_INET,
                                     ETCPAL_STREAM, &conn->core.sock);
  if (res != kEtcPalErrOk)
  {
    ok = false;
    event->which = kRCConnEventConnectFailed;
    event->arg.connect_failed.event = kRdmnetConnectFailSocketFailure;
    event->arg.connect_failed.socket_err = res;
  }

  if (ok)
  {
    res = etcpal_setblocking(conn->core.sock, false);
    if (res != kEtcPalErrOk)
    {
      ok = false;
      event->which = kRCConnEventConnectFailed;
      event->arg.connect_failed.event = kRdmnetConnectFailSocketFailure;
      event->arg.connect_failed.socket_err = res;
    }
  }

  if (ok)
  {
    conn->rdmnet_conn_failed = false;
    res = etcpal_connect(conn->core.sock, &conn->core.remote_addr);
    if (res == kEtcPalErrOk)
    {
      // Fast connect condition
      start_rdmnet_connection(conn);
    }
    else if (res == kEtcPalErrInProgress || res == kEtcPalErrWouldBlock)
    {
      conn->state = kRCConnStateTCPConnPending;
      etcpal_error_t add_res = rdmnet_core_add_polled_socket(conn->core.sock, ETCPAL_POLL_CONNECT, &conn->poll_info);
      if (add_res != kEtcPalErrOk)
      {
        ok = false;
        event->which = kRCConnEventConnectFailed;
        event->arg.connect_failed.event = kRdmnetConnectFailSocketFailure;
        event->arg.connect_failed.socket_err = add_res;
      }
    }
    else
    {
      ok = false;
      event->which = kRCConnEventConnectFailed;

      // EHOSTUNREACH is sometimes reported synchronously even for a non-blocking connect.
      if (res == kEtcPalErrHostUnreach)
        event->arg.connect_failed.event = kRdmnetConnectFailTcpLevel;
      else
        event->arg.connect_failed.event = kRdmnetConnectFailSocketFailure;
      event->arg.connect_failed.socket_err = res;
    }
  }

  if (!ok)
    reset_connection(conn);
}

/*
 * Handle periodic RDMnet connection functionality.
 */
void rc_conn_module_tick()
{
  if (!rdmnet_core_initialized())
    return;

  if (rdmnet_writelock())
  {
    destroy_marked_connections();
    add_pending_connections();
    rdmnet_writeunlock();
  }

  process_all_connection_state();
}

void destroy_marked_connections(void)
{
  FOR_EACH_CONN_TO_DESTROY() { cleanup_connection_resources(conn); }
  connections.num_to_destroy = 0;
}

void add_pending_connections(void)
{
  // TODO
}

void process_all_connection_state(void)
{
  FOR_EACH_ACTIVE_CONN()
  {
    if (RC_CONN_LOCK(conn))
    {
      RCConnEvent event = RC_CONN_EVENT_INIT;

      switch (conn->state)
      {
        case kRCConnStatePending:
          if (conn->rdmnet_conn_failed || conn->backoff_timer.interval != 0)
          {
            if (conn->rdmnet_conn_failed)
              etcpal_timer_start(&conn->backoff_timer, update_backoff(conn->backoff_timer.interval));
            conn->state = kRCConnStateBackoff;
          }
          else
          {
            start_tcp_connection(conn, &event);
          }
          break;
        case kRCConnStateBackoff:
          if (etcpal_timer_is_expired(&conn->backoff_timer))
          {
            start_tcp_connection(conn, &event);
          }
          break;
        case kRCConnStateHeartbeat:
          if (!rc_conn_process_heartbeats(&conn->core))
          {
            // Heartbeat timeout! Disconnect the connection.
            event.which = kRCConnEventDisconnected;
            event.arg.disconnected.event = kRdmnetDisconnectNoHeartbeat;
            event.arg.disconnected.socket_err = kEtcPalErrOk;
            reset_connection(conn);
          }
          break;
        default:
          break;
      }

      RC_CONN_UNLOCK(conn);
      deliver_event_callback(conn, &event);
    }
  }
}

bool rc_conn_process_heartbeats(RCConnection* conn)
{
  if (etcpal_timer_is_expired(&conn->hb_timer))
  {
    return false;
  }
  else if (etcpal_timer_is_expired(&conn->send_timer))
  {
    rc_broker_send_null(conn);
    etcpal_timer_reset(&conn->send_timer);
  }
  return true;
}

void tcp_connection_established(RCClientConnection* conn)
{
  if (RC_CONN_LOCK(conn))
  {
    // connected successfully!
    start_rdmnet_connection(conn);
    RC_CONN_UNLOCK(conn);
  }
}

void handle_socket_error(RCClientConnection* conn, etcpal_error_t socket_err)
{
  if (RC_CONN_LOCK(conn))
  {
    RCConnEvent event = RC_CONN_EVENT_INIT;

    if (conn->state == kRCConnStateTCPConnPending || conn->state == kRCConnStateRDMnetConnPending)
    {
      event.which = kRCConnEventConnectFailed;
      event.arg.connect_failed.event = kRdmnetConnectFailTcpLevel;
      event.arg.connect_failed.socket_err = socket_err;
      if (conn->state == kRCConnStateRDMnetConnPending)
        conn->rdmnet_conn_failed = true;

      reset_connection(conn);
    }
    else if (conn->state == kRCConnStateHeartbeat)
    {
      event.which = kRCConnEventDisconnected;
      event.arg.disconnected.event = kRdmnetDisconnectAbruptClose;
      event.arg.disconnected.socket_err = socket_err;

      reset_connection(conn);
    }
    RC_CONN_UNLOCK(conn);
    deliver_event_callback(conn, &event);
  }
}

void handle_rdmnet_connect_result(RCClientConnection* conn, RdmnetMessage* msg, RCConnEvent* event)
{
  if (RDMNET_GET_BROKER_MSG(msg))
  {
    BrokerMessage* bmsg = RDMNET_GET_BROKER_MSG(msg);
    if (BROKER_IS_CONNECT_REPLY_MSG(bmsg))
    {
      BrokerConnectReplyMsg* reply = BROKER_GET_CONNECT_REPLY_MSG(bmsg);
      switch (reply->connect_status)
      {
        case kRdmnetConnectOk:
          // TODO check version
          conn->state = kRCConnStateHeartbeat;
          etcpal_timer_start(&conn->backoff_timer, 0);
          event->which = kRCConnEventConnected;

          RCConnectedInfo* connect_info = &event->arg.connected;
          connect_info->broker_uid = reply->broker_uid;
          connect_info->client_uid = reply->client_uid;
          connect_info->connected_addr = conn->core.remote_addr;
          break;
        default:
        {
          event->which = kRCConnEventConnectFailed;

          RCConnectFailedInfo* failed_info = &event->arg.connect_failed;
          failed_info->event = kRdmnetConnectFailRejected;
          failed_info->socket_err = kEtcPalErrOk;
          failed_info->rdmnet_reason = reply->connect_status;

          reset_connection(conn);
          conn->rdmnet_conn_failed = true;
          break;
        }
      }
    }
    else if (BROKER_IS_CLIENT_REDIRECT_MSG(bmsg))
    {
      conn->core.remote_addr = BROKER_GET_CLIENT_REDIRECT_MSG(bmsg)->new_addr;
      retry_connection(conn);
    }
  }
  rc_free_message_resources(msg);
}

void handle_rdmnet_message(RCClientConnection* conn, RdmnetMessage* msg, RCConnEvent* cb)
{
  // We handle some Broker messages internally
  bool deliver_message = false;
  if (RDMNET_GET_BROKER_MSG(msg))
  {
    BrokerMessage* bmsg = RDMNET_GET_BROKER_MSG(msg);
    switch (bmsg->vector)
    {
      case VECTOR_BROKER_CONNECT_REPLY:
      case VECTOR_BROKER_NULL:
        break;
      case VECTOR_BROKER_DISCONNECT:
        cb->which = kRCConnEventDisconnected;
        cb->arg.disconnected.event = kRdmnetDisconnectGracefulRemoteInitiated;
        cb->arg.disconnected.socket_err = kEtcPalErrOk;
        cb->arg.disconnected.rdmnet_reason = BROKER_GET_DISCONNECT_MSG(bmsg)->disconnect_reason;

        reset_connection(conn);
        break;
      default:
        deliver_message = true;
        break;
    }
  }
  else
  {
    deliver_message = true;
  }

  if (deliver_message)
  {
    cb->which = kRCConnEventMsgReceived;
    cb->arg.message = msg;
  }
  else
  {
    rc_free_message_resources(msg);
  }
}

etcpal_error_t rc_conn_parse_message(RCConnection* conn, const uint8_t* data, size_t data_size)
{
  etcpal_error_t res = rc_msg_buf_recv(&conn->recv_buf, data, data_size);
  if (res == kEtcPalErrOk)
    etcpal_timer_reset(&conn->hb_timer);
  return res;
}

etcpal_error_t parse_single_message(RCClientConnection* conn, const uint8_t* data, size_t data_size)
{
  etcpal_error_t res = kEtcPalErrSys;

  if (RC_CONN_LOCK(conn))
  {
    RCConnEvent cb;
    if (conn->state == kRCConnStateHeartbeat || conn->state == kRCConnStateRDMnetConnPending)
    {
      res = rc_conn_parse_message(&conn->core, data, data_size);
      if (res == kEtcPalErrOk)
      {
        if (conn->state == kRCConnStateRDMnetConnPending)
        {
          handle_rdmnet_connect_result(conn, &conn->core.recv_buf.msg, &cb);
        }
        else
        {
          handle_rdmnet_message(conn, &conn->core.recv_buf.msg, &cb);
        }
      }
    }
    else
    {
      res = kEtcPalErrInvalid;
    }
    RC_CONN_UNLOCK(conn);
    deliver_event_callback(conn, &cb);
  }
  return res;
}

void deliver_event_callback(RCClientConnection* conn, RCConnEvent* event)
{
  switch (event->which)
  {
    case kRCConnEventConnected:
      if (conn->callbacks.connected)
        conn->callbacks.connected(conn, &event->arg.connected);
      break;
    case kRCConnEventConnectFailed:
      if (conn->callbacks.connect_failed)
        conn->callbacks.connect_failed(conn, &event->arg.connect_failed);
      break;
    case kRCConnEventDisconnected:
      if (conn->callbacks.disconnected)
        conn->callbacks.disconnected(conn, &event->arg.disconnected);
      break;
    case kRCConnEventMsgReceived:
      if (conn->callbacks.message_received)
        conn->callbacks.message_received(conn, event->arg.message);
      rc_free_message_resources(event->arg.message);
      break;
    case kRCConnEventNone:
    default:
      break;
  }
}

void socket_activity_callback(const EtcPalPollEvent* event, PolledSocketOpaqueData data)
{
  static uint8_t rdmnet_poll_recv_buf[RDMNET_RECV_DATA_MAX_SIZE];
  RCClientConnection* conn = (RCClientConnection*)data.ptr;

  if (event->events & ETCPAL_POLL_ERR)
  {
    handle_socket_error(conn, event->err);
  }
  else if (event->events & ETCPAL_POLL_IN)
  {
    int recv_res = etcpal_recv(event->socket, rdmnet_poll_recv_buf, RDMNET_RECV_DATA_MAX_SIZE, 0);
    if (recv_res <= 0)
    {
      handle_socket_error(conn, recv_res);
    }
    else
    {
      etcpal_error_t res = parse_single_message(conn, rdmnet_poll_recv_buf, (size_t)recv_res);
      while (res == kEtcPalErrOk)
      {
        res = parse_single_message(conn, NULL, 0);
      }
    }
  }
  else if (event->events & ETCPAL_POLL_CONNECT)
  {
    tcp_connection_established(conn);
  }
}

// Internal function to update a backoff timer value using the algorithm specified in E1.33.
// Returns the new value.
uint32_t update_backoff(uint32_t previous_backoff)
{
  uint32_t result = (uint32_t)(((rand() % 4001) + 1000));
  result += previous_backoff;
  // 30 second interval is the max
  if (result > 30000u)
    return 30000u;
  return result;
}

void reset_connection(RCClientConnection* conn)
{
  cleanup_connection_resources(conn);
  conn->state = kRCConnStateNotStarted;
}

void retry_connection(RCClientConnection* conn)
{
  cleanup_connection_resources(conn);
  conn->state = kRCConnStatePending;
}

void cleanup_connection_resources(RCClientConnection* conn)
{
  if (conn)
  {
    if (conn->core.sock != ETCPAL_SOCKET_INVALID)
    {
      rdmnet_core_remove_polled_socket(conn->core.sock);
      etcpal_close(conn->core.sock);
      conn->core.sock = ETCPAL_SOCKET_INVALID;
    }
  }
}

#if RDMNET_DYNAMIC_MEM
bool add_connection_dynamic(RCClientConnection** list, size_t* list_capacity, size_t* list_size,
                            RCClientConnection* to_add)
{
  // TODO
  return false;
}

void remove_connection_dynamic(RCClientConnection** list, size_t* list_capacity, size_t* list_size,
                               RCClientConnection* to_remove)
{
  // TODO
}
#else
bool add_connection_static(RCClientConnection** list, size_t* list_size, RCClientConnection* to_add)
{
  // TODO
  return false;
}

void remove_connection_static(RCClientConnection** list, size_t* list_size, RCClientConnection* to_remove)
{
  // TODO
}
#endif

int find_connection_index(RCClientConnection* conn, const RCClientConnection** list, size_t list_size)
{
  // TODO
  return -1;
}
