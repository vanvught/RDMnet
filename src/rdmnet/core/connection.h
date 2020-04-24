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

/*!
 * \file rdmnet/core/connection.h
 * \brief RDMnet Connection API definitions
 *
 * Functions and definitions for the \ref rdmnet_conn "RDMnet Connection API" are contained in this
 * header.
 */

#ifndef RDMNET_CORE_CONNECTION_H_
#define RDMNET_CORE_CONNECTION_H_

#include <stdbool.h>
#include <stddef.h>
#include "etcpal/log.h"
#include "etcpal/error.h"
#include "etcpal/inet.h"
#include "etcpal/timer.h"
#include "etcpal/socket.h"
#include "rdmnet/core/common.h"
#include "rdmnet/core/message.h"
#include "rdmnet/core/msg_buf.h"

/*
 * \defgroup rdmnet_conn Connection
 * \ingroup rdmnet_core_lib
 * \brief Handle a connection between a Client and a Broker in RDMnet.
 *
 * In E1.33, the behavior of this module is dictated by the Broker Protocol (&sect; 6).
 *
 * Basic functionality for an RDMnet Client: Initialize the library using rdmnet_init(). Create a
 * new connection using rdmnet_connection_create(). Connect to a Broker using rdmnet_connect().
 * Depending on the value of #RDMNET_USE_TICK_THREAD, may need to call rdmnet_tick() at regular
 * intervals. Send data over the Broker connection using rdmnet_send(), and receive data over the
 * Broker connection using rdmnet_recv().
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RCConnection RCConnection;

/*! Information about a successful RDMnet connection. */
typedef struct RCConnectedInfo
{
  /*! The broker's UID. */
  RdmUid broker_uid;
  /*! The client's UID (relevant if assigned dynamically) */
  RdmUid client_uid;
  /*!
   * The remote address to which we are connected. This could be different from the original
   * address requested in the case of a redirect.
   */
  EtcPalSockAddr connected_addr;
} RCConnectedInfo;

/*! Information about an unsuccessful RDMnet connection. */
typedef struct RCConnectFailedInfo
{
  /*! The high-level reason that this connection failed. */
  rdmnet_connect_fail_event_t event;
  /*!
   * The system error code associated with the failure; valid if event is
   * kRdmnetConnectFailSocketFailure or kRdmnetConnectFailTcpLevel.
   */
  etcpal_error_t socket_err;
  /*!
   * The reason given in the RDMnet-level connection refuse message. Valid if event is
   * kRdmnetConnectFailRejected.
   */
  rdmnet_connect_status_t rdmnet_reason;
} RCConnectFailedInfo;

/*! Information about an RDMnet connection that disconnected after a successful connection. */
typedef struct RCDisconnectedInfo
{
  /*! The high-level reason for the disconnect. */
  rdmnet_disconnect_event_t event;
  /*!
   * The system error code associated with the disconnect; valid if event is
   * kRdmnetDisconnectAbruptClose.
   */
  etcpal_error_t socket_err;
  /*!
   * The reason given in the RDMnet-level disconnect message. Valid if event is
   * kRdmnetDisconnectGracefulRemoteInitiated.
   */
  rdmnet_disconnect_reason_t rdmnet_reason;
} RCDisconnectedInfo;

/*
 * \brief An RDMnet connection has connected successfully.
 * \param[in] handle Handle to connection which has connected.
 * \param[in] connect_info More information about the successful connection.
 * \param[in] context Context pointer that was given at the creation of the connection.
 */
typedef void (*RCConnectedCallback)(RCConnection* conn, const RCConnectedInfo* connect_info);

/*
 * \brief An RDMnet connection attempt failed.
 * \param[in] handle Handle to connection which has failed.
 * \param[in] failed_info More information about the connect failure event.
 * \param[in] context Context pointer that was given at the creation of the connection.
 */
typedef void (*RCConnectFailedCallback)(RCConnection* conn, const RCConnectFailedInfo* failed_info);

/*
 * \brief A previously-connected RDMnet connection has disconnected.
 * \param[in] handle Handle to connection which has been disconnected.
 * \param[in] disconn_info More information about the disconnect event.
 * \param[in] context Context pointer that was given at the creation of the connection.
 */
typedef void (*RCDisconnectedCallback)(RCConnection* conn, const RCDisconnectedInfo* disconn_info);

/*
 * \brief A message has been received on an RDMnet connection.
 *
 * Broker Protocol messages that affect connection status are consumed internally by the
 * connection library and thus will not result in this callback. All other valid messages will be
 * delivered.
 *
 * \param[in] handle Handle to connection on which the message has been received.
 * \param[in] message Contains the message received. Use the macros in \ref rdmnet_message to
 *                    decode.
 * \param[in] context Context pointer that was given at creation of the connection.
 */
typedef void (*RCMessageReceivedCallback)(RCConnection* conn, const RdmnetMessage* message);

/*! A set of callbacks which are called with notifications about RDMnet connections. */
typedef struct RCConnectionCallbacks
{
  RCConnectedCallback connected;
  RCConnectFailedCallback connect_failed;
  RCDisconnectedCallback disconnected;
  RCMessageReceivedCallback message_received;
} RCConnectionCallbacks;

typedef enum
{
  kRCConnStateNotStarted,
  kRCConnStatePending,
  kRCConnStateBackoff,
  kRCConnStateTCPConnPending,
  kRCConnStateRDMnetConnPending,
  kRCConnStateHeartbeat,
  kRCConnStateMarkedForDestruction
} rc_conn_state_t;

struct RCConnection
{
  EtcPalUuid local_cid;
  // Synchronization
  etcpal_mutex_t* lock;
  RCConnectionCallbacks callbacks;

  // Underlying socket connection
  etcpal_socket_t sock;
  EtcPalSockAddr remote_addr;
  bool external_socket_attached;
  bool is_blocking;
  PolledSocketInfo poll_info;

  // Connection state
  rc_conn_state_t state;
  BrokerClientConnectMsg conn_data;
  EtcPalTimer send_timer;
  EtcPalTimer hb_timer;
  EtcPalTimer backoff_timer;
  bool rdmnet_conn_failed;

  // Send and receive tracking
  RCMsgBuf recv_buf;

  // Destruction
  // RdmnetConnection* next_to_destroy;
};

/*
typedef enum
{
  kConnCallbackNone,
  kConnCallbackConnected,
  kConnCallbackConnectFailed,
  kConnCallbackDisconnected,
  kConnCallbackMsgReceived
} conn_callback_t;

typedef struct ConnConnectedArgs
{
  RdmnetConnectedInfo connect_info;
} ConnConnectedArgs;

typedef struct ConnConnectFailedArgs
{
  RdmnetConnectFailedInfo failed_info;
} ConnConnectFailedArgs;

typedef struct ConnDisconnectedArgs
{
  RdmnetDisconnectedInfo disconn_info;
} ConnDisconnectedArgs;

typedef struct ConnMsgReceivedArgs
{
  RdmnetMessage message;
} ConnMsgReceivedArgs;

typedef struct ConnCallbackDispatchInfo
{
  rdmnet_conn_t handle;
  RdmnetConnCallbacks cbs;
  void* context;

  conn_callback_t which;
  union
  {
    ConnConnectedArgs connected;
    ConnConnectFailedArgs connect_failed;
    ConnDisconnectedArgs disconnected;
    ConnMsgReceivedArgs msg_received;
  } args;
} ConnCallbackDispatchInfo;
*/

etcpal_error_t rc_connection_register(RCConnection* conn);
etcpal_error_t rc_connection_unregister(RCConnection* conn, const rdmnet_disconnect_reason_t* disconnect_reason);
etcpal_error_t rc_connection_connect(RCConnection* conn, const EtcPalSockAddr* remote_addr,
                                     const BrokerClientConnectMsg* connect_data);
etcpal_error_t rc_connection_set_blocking(RCConnection* conn, bool blocking);

int rc_connection_send(RCConnection* conn, const uint8_t* data, size_t size);

/*!
 * \name Externally managed socket functions.
 *
 * These functions are for advanced usage and are generally only used by broker apps.
 * @{
 */
// etcpal_error_t rdmnet_attach_existing_socket(rdmnet_conn_t handle, etcpal_socket_t sock,
//                                             const EtcPalSockAddr* remote_addr);
// void rdmnet_socket_data_received(rdmnet_conn_t handle, const uint8_t* data, size_t data_size);
// void rdmnet_socket_error(rdmnet_conn_t handle, etcpal_error_t socket_err);
/*!
 * @}
 */

etcpal_error_t rc_connection_init(void);
void rc_connection_deinit(void);

// etcpal_error_t rc_connection_start_message(RdmnetConnection* conn);
// etcpal_error_t rc_connection_end_message(RdmnetConnection* conn);

void rc_connection_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* RDMNET_CORE_CONNECTION_H_ */
