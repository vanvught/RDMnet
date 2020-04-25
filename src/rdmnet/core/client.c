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

#include "rdmnet/core/client.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "etcpal/common.h"
#include "rdm/controller.h"
#include "rdm/responder.h"
#include "rdmnet/core/client_entry.h"
#include "rdmnet/core/connection.h"
#include "rdmnet/core/util.h"
//#include "rdmnet/discovery.h"
#include "rdmnet/private/opts.h"
#include "rdmnet/core/util.h"

#if RDMNET_DYNAMIC_MEM
#include <stdlib.h>
#endif

/*************************** Private constants *******************************/

#if RDMNET_ALLOW_EXTERNALLY_MANAGED_SOCKETS
#define CB_STORAGE_CLASS
#else
#define CB_STORAGE_CLASS static
#endif

#define INITIAL_SCOPES_CAPACITY 5

/***************************** Private macros ********************************/

#define GET_CLIENT_FROM_LLRP_TARGET(targetptr) (RCClient*)((char*)(targetptr)-offsetof(RCClient, llrp_target))
#define GET_CLIENT_SCOPE_FROM_CONN(connptr) (RCClientScope*)((char*)(connptr)-offsetof(RCClientScope, conn))
#define GET_SCOPE_HANDLE(clientptr, scopeptr) (rdmnet_client_scope_t)((scopeptr) - (clientptr)->scopes)

// Macros for dynamic vs static allocation. Static allocation is done using etcpal_mempool for
// client tracking information, and via a static buffer for RDM responses.
#if RDMNET_DYNAMIC_MEM

#define ALLOC_CLIENT_RDM_RESPONSE_ARRAY(num_responses) calloc((num_responses), sizeof(RdmResponse))
#define FREE_CLIENT_RDM_RESPONSE_ARRAY(ptr) free(ptr)

#else

#define ALLOC_CLIENT_RDM_RESPONSE_ARRAY(num_responses) \
  (RDMNET_ASSERT(num_responses <= RDMNET_MAX_RECEIVED_ACK_OVERFLOW_RESPONSES), client_rdm_response_buf)
#define FREE_CLIENT_RDM_RESPONSE_ARRAY(ptr)

#endif

// #define INIT_CALLBACK_INFO(cbptr) ((cbptr)->which = kClientCallbackNone)

/**************************** Private variables ******************************/

//#if !RDMNET_DYNAMIC_MEM
// static RdmResponse client_rdm_response_buf[RDMNET_MAX_RECEIVED_ACK_OVERFLOW_RESPONSES];
//#endif

// static void monitorcb_broker_found(rdmnet_scope_monitor_t handle, const RdmnetBrokerDiscInfo* broker_info,
//                                    void* context);
// static void monitorcb_broker_lost(rdmnet_scope_monitor_t handle, const char* scope, const char* service_name,
//                                   void* context);
//
// // clang-format off
// static const RdmnetScopeMonitorCallbacks disc_callbacks =
// {
//   monitorcb_broker_found,
//   monitorcb_broker_lost,
// };
// // clang-format on

static rc_lock_state_t conncb_connected(RCConnection* conn, const RCConnectedInfo* connected_info);
static rc_lock_state_t conncb_connect_failed(RCConnection* conn, const RCConnectFailedInfo* failed_info);
static rc_lock_state_t conncb_disconnected(RCConnection* conn, const RCDisconnectedInfo* disconn_info);
static rc_lock_state_t conncb_msg_received(RCConnection* conn, const RdmnetMessage* message);

// clang-format off
static const RCConnectionCallbacks conn_callbacks =
{
  conncb_connected,
  conncb_connect_failed,
  conncb_disconnected,
  conncb_msg_received
};
// clang-format on

static rc_lock_state_t llrpcb_rdm_cmd_received(RCLlrpTarget* target, const LlrpRdmCommand* cmd,
                                               RdmnetSyncRdmResponse* response);

/*********************** Private function prototypes *************************/

// Create and destroy clients and scopes
// static etcpal_error_t validate_rpt_client_config(const RdmnetRptClientConfig* config);
// static etcpal_error_t new_rpt_client(const RdmnetRptClientConfig* config, rdmnet_client_t* handle);
// static void destroy_client(RdmnetClient* cli, rdmnet_disconnect_reason_t reason);
static etcpal_error_t create_and_add_scope_entry(RCClient* client, const RdmnetScopeConfig* config,
                                                 int* new_entry_index);
static bool scope_exists_in_list(RCClient* client, const char* scope);
static RCClientScope* get_unused_scope_entry(RCClient* client);
static void cleanup_scope_entry(RCClient* client, RCClientScope* scope_entry);

static etcpal_error_t start_scope_discovery(RCClientScope* scope_entry, const char* search_domain);
static void attempt_connection_on_listen_addrs(RCClientScope* scope_entry);
static etcpal_error_t start_connection_for_scope(RCClientScope* scope_entry, const EtcPalSockAddr* broker_addr);
//
//// Find clients and scopes
// static etcpal_error_t get_client(rdmnet_client_t handle, RdmnetClient** client);
// static RdmnetClient* get_client_by_llrp_handle(llrp_target_t handle);
// static void release_client(const RdmnetClient* client);
// static RCClientScope* get_scope(rdmnet_client_scope_t handle);
// static RCClientScope* get_scope_by_disc_handle(rdmnet_scope_monitor_t handle);
// static void release_scope(const RCClientScope* scope_entry);
// static etcpal_error_t get_client_and_scope(rdmnet_client_t handle, rdmnet_client_scope_t scope_handle,
//                                           RdmnetClient** client, RCClientScope** scope_entry);
// static void release_client_and_scope(const RdmnetClient* client, const RCClientScope* scope_entry);
//
//// Manage callbacks
static bool connect_failed_will_retry(rdmnet_connect_fail_event_t event, rdmnet_connect_status_t status);
static bool disconnected_will_retry(rdmnet_disconnect_event_t event, rdmnet_disconnect_reason_t reason);

//// Message handling
// static void free_rpt_client_message(RptClientMessage* msg);
// static void free_ept_client_message(EptClientMessage* msg);
// static bool handle_rpt_request(const RptMessage* rmsg, RptClientMessage* msg_out);
// static bool handle_rpt_notification(const RptMessage* rmsg, RptClientMessage* msg_out);
// static bool handle_rpt_status(const RptMessage* rmsg, RptClientMessage* msg_out);
// static bool handle_rpt_message(const RdmnetClient* cli, const RCClientScope* scope_entry, const RptMessage* rmsg,
//                               RptMsgReceivedArgs* cb_args);

/*************************** Function definitions ****************************/

etcpal_error_t rc_client_init(void)
{
  return kEtcPalErrOk;
}

/*
static void client_dealloc(const EtcPalRbTree* self, EtcPalRbNode* node)
{
  ETCPAL_UNUSED_ARG(self);

  RdmnetClient* cli = (RdmnetClient*)node->value;
  if (cli)
    destroy_client(cli, kRdmnetDisconnectShutdown);
  client_node_free(node);
}
*/

void rc_client_deinit(void)
{
  // etcpal_rbtree_clear_with_cb(&state.clients, client_dealloc);
}

/*
 * Initialize a new RdmnetClient structure.
 *
 * Initialize the items marked in the struct before passing it to this function. The LLRP
 * information will be used to create an associated LLRP target.
 */
etcpal_error_t rc_rpt_client_register(RCClient* client, bool create_llrp_target,
                                      const RdmnetMcastNetintId* llrp_netints, size_t num_llrp_netints)
{
  RDMNET_ASSERT(client);

  if (!rdmnet_core_initialized())
    return kEtcPalErrNotInit;

  if (create_llrp_target)
  {
    RCLlrpTarget* target = &client->llrp_target;
    target->cid = client->cid;
    target->uid = RC_RPT_CLIENT_DATA(client)->uid;
    target->component_type =
        (RC_RPT_CLIENT_DATA(client)->type == kRPTClientTypeController ? kLlrpCompRptController : kLlrpCompRptDevice);
    target->rdm_cmd_received_cb = llrpcb_rdm_cmd_received;
    target->lock = client->lock;

    etcpal_error_t res = rc_llrp_target_register(&client->llrp_target, llrp_netints, num_llrp_netints);
    if (res != kEtcPalErrOk)
      return res;
  }

  RDMNET_INIT_BUF(client, scopes);
  client->num_scopes = 0;

#if RDMNET_DYNAMIC_MEM
  if (client->type == kClientProtocolRPT && RC_RPT_CLIENT_DATA(client)->type == kRPTClientTypeDevice)
  {
    client->scopes = (RCClientScope*)malloc(sizeof(RCClientScope));
    client->scopes_capacity = 1;
    client->scopes[0].valid = false;
  }
  else
  {
    client->scopes = (RCClientScope*)calloc(INITIAL_SCOPES_CAPACITY, sizeof(RCClientScope));
    client->scopes_capacity = INITIAL_SCOPES_CAPACITY;
    for (RCClientScope* scope = client->scopes; scope < client->scopes + INITIAL_SCOPES_CAPACITY; ++scope)
      scope->valid = false;
  }
#else
  for (RCClientScope* scope = client->scopes; scope < client->scopes + RDMNET_MAX_SCOPES_PER_CLIENT; ++scope)
    scope->valid = false;
#endif
  return kEtcPalErrOk;
}

/*
 * \brief Destroy an RDMnet client instance.
 *
 * Will disconnect from all brokers to which this client is currently connected, sending the
 * disconnect reason provided in the disconnect_reason parameter.
 *
 * \param[in] handle Handle to client to destroy, no longer valid after this function returns.
 * \param[in] disconnect_reason Disconnect reason code to send on all connected scopes.
 */
etcpal_error_t rc_client_unregister(RCClient* client, rdmnet_disconnect_reason_t disconnect_reason)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(disconnect_reason);
  return kEtcPalErrNotImpl;
  //  if (!rdmnet_core_initialized())
  //    return kEtcPalErrNotInit;
  //
  //  RdmnetClient* cli;
  //  etcpal_error_t res = get_client(handle, &cli);
  //  if (res != kEtcPalErrOk)
  //    return res;
  //
  //  etcpal_rbtree_remove(&state.clients, cli);
  //  destroy_client(cli, reason);
  //  RDMNET_CLIENT_UNLOCK();
  //  return res;
}

/*
 * \brief Add a new scope to a client instance.
 *
 * The library will attempt to discover and connect to a broker for the scope (or just connect if a
 * static broker address is given); the status of these attempts will be communicated via the
 * callbacks associated with the client instance.
 *
 * \param[in] handle Handle to client to which to add a new scope.
 * \param[in] scope_config Configuration parameters for the new scope.
 * \param[out] scope_handle Filled in on success with a handle to the new scope.
 */
etcpal_error_t rc_client_add_scope(RCClient* client, const RdmnetScopeConfig* scope_config,
                                   rdmnet_client_scope_t* scope_handle)
{
  RDMNET_ASSERT(client);
  RDMNET_ASSERT(scope_config);
  RDMNET_ASSERT(scope_handle);

  if (scope_exists_in_list(client, scope_config->scope))
    return kEtcPalErrExists;

  int new_entry_index;
  etcpal_error_t res = create_and_add_scope_entry(client, scope_config, &new_entry_index);
  if (res == kEtcPalErrOk)
  {
    RCClientScope* new_entry = &client->scopes[new_entry_index];
    // Start discovery or connection on the new scope (depending on whether a static broker was
    // configured)
    if (new_entry->state == kRCScopeStateDiscovery)
      res = start_scope_discovery(new_entry, client->search_domain);
    else if (new_entry->state == kRCScopeStateConnecting)
      res = start_connection_for_scope(new_entry, &new_entry->static_broker_addr);

    if (res == kEtcPalErrOk)
    {
      *scope_handle = new_entry_index;
    }
    else
    {
      cleanup_scope_entry(client, new_entry);
    }
  }

  return res;
}

/*
 * \brief Remove a previously-added scope from a client instance.
 *
 * After this call completes, scope_handle will no longer be valid.
 *
 * \param[in] handle Handle to the client from which to remove a scope.
 * \param[in] scope_handle Handle to scope to remove.
 * \param[in] disconnect_reason RDMnet protocol disconnect reason to send to the connected broker.
 */
etcpal_error_t rc_client_remove_scope(RCClient* client, rdmnet_client_scope_t scope_handle,
                                      rdmnet_disconnect_reason_t reason)
{
  /*
  if (handle < 0 || scope_handle < 0)
    return kEtcPalErrInvalid;

  RdmnetClient* cli;
  RCClientScope* scope_entry;
  etcpal_error_t res = get_client_and_scope(handle, scope_handle, &cli, &scope_entry);
  if (res != kEtcPalErrOk)
    return res;

  if (scope_entry->monitor_handle)
  {
    rdmnet_disc_stop_monitoring(scope_entry->monitor_handle);
    etcpal_rbtree_remove(&state.scopes_by_disc_handle, scope_entry);
  }
  rdmnet_connection_destroy(scope_entry->handle, &reason);
  remove_scope_from_list(&cli->scope_list, scope_entry);
  etcpal_rbtree_remove(&state.scopes_by_handle, scope_entry);
  FREE_CLIENT_SCOPE(scope_entry);

  release_client(cli);
  return res;
  */
  return kEtcPalErrNotImpl;
}

/*
 * \brief Change the settings of a previously-added scope.
 *
 * Changed settings will cause the client to disconnect from any connected broker for the old
 * scope.
 *
 * \param[in] handle Handle to the client for which to change a scope.
 * \param[in] scope_handle Handle to scope to change.
 * \param[in] new_scope_config New configuration settings to use for the scope.
 * \param[in] disconnect_reason RDMnet protocol disconnect reason to send to the connected broker.
 */
etcpal_error_t rc_client_change_scope(RCClient* client, rdmnet_client_scope_t scope_handle,
                                      const RdmnetScopeConfig* new_scope_config,
                                      rdmnet_disconnect_reason_t disconnect_reason)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(new_scope_config);
  ETCPAL_UNUSED_ARG(disconnect_reason);
  return kEtcPalErrNotImpl;
}

/*
 * \brief Retrieve the scope string of a previously-added scope.
 *
 * \param[in] handle Handle to the client from which to retrieve the scope string.
 * \param[in] scope_handle Handle to scope for which to retrieve the scope string.
 * \param[out] scope_str_buf Filled in on success with the scope string. Must be at least of length
 *                           #E133_SCOPE_STRING_PADDED_LENGTH.
 */
etcpal_error_t rc_client_get_scope(RCClient* client, rdmnet_client_scope_t scope_handle, char* scope_str_buf,
                                   EtcPalSockAddr* static_broker_addr)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(scope_str_buf);
  ETCPAL_UNUSED_ARG(static_broker_addr);
  return kEtcPalErrNotImpl;
}

/*
 * \brief Change the search domain setting of a client.
 *
 * A changed domain will cause the client to disconnect from any connected broker for which dynamic
 * discovery is configured and restart the discovery process.
 *
 * \param[in] handle Handle to the client for which to change the search domain.
 * \param[in] new_search_domain New search domain to use for DNS discovery.
 * \param[in] disconnect_reason RDMnet protocol disconnect reason to send to the connected broker.
 */
etcpal_error_t rc_client_change_search_domain(RCClient* client, const char* new_search_domain,
                                              rdmnet_disconnect_reason_t reason)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(new_search_domain);
  ETCPAL_UNUSED_ARG(reason);
  // TODO
  return kEtcPalErrNotImpl;
}

/*
 * \brief Send a message requesting an RDMnet client list from a broker on a given scope.
 *
 * The response will be delivered via an RdmnetClientBrokerMsgReceivedCb containing a ClientList
 * broker message.
 *
 * \param[in] handle Handle to the client from which to request a client list.
 * \param[in] scope_handle Scope handle on which to request a client list.
 */
etcpal_error_t rc_client_request_client_list(RCClient* client, rdmnet_client_scope_t scope_handle)
{
  /*
  if (handle < 0 || scope_handle < 0)
    return kEtcPalErrInvalid;

  RdmnetClient* cli;
  RCClientScope* scope_entry;
  etcpal_error_t res = get_client_and_scope(handle, scope_handle, &cli, &scope_entry);
  if (res != kEtcPalErrOk)
    return res;

  res = broker_send_fetch_client_list(scope_handle, &cli->cid);

  release_client_and_scope(cli, scope_entry);
  return res;
  */
  return kEtcPalErrNotImpl;
}

/*
 * \brief Send a message requesting one or more dynamic UIDs from a broker on a given scope.
 *
 * The response will be delivered via an RdmnetClientBrokerMsgReceivedCb containing a
 * BrokerDynamicUidAssignmentList broker message.
 *
 * \param[in] handle Handle to the client from which to request dynamic UIDs.
 * \param[in] scope_handle Scope handle on which to request dynamic UIDs.
 * \param[in] requests Array of Dynamic UID Request structures representing requested dynamic UIDs.
 * \param[in] num_requests Size of requests array.
 */
etcpal_error_t rc_client_request_dynamic_uids(RCClient* client, rdmnet_client_scope_t scope_handle,
                                              const EtcPalUuid* responder_ids, size_t num_responders)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(responder_ids);
  ETCPAL_UNUSED_ARG(num_responders);
  return kEtcPalErrNotImpl;
}

/*
 * \brief Send a message requesting the mapping of one or more dynamic UIDs to RIDs from a broker
 *        on a given scope.
 *
 * The response will be delivered via an RdmnetClientBrokerMsgReceivedCb containing a
 * BrokerDynamicUidAssignmentList broker message.
 *
 * \param[in] handle Handle to the client from which to request dynamic UID mappings.
 * \param[in] scope_handle Scope handle on which to request dynamic UID mappings.
 * \param[in] uids Array of UIDs for which to request the mapped RIDs.
 * \param[in] num_uids Size of uids array.
 */
etcpal_error_t rc_client_request_responder_ids(RCClient* client, rdmnet_client_scope_t scope_handle, const RdmUid* uids,
                                               size_t num_uids)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(uids);
  ETCPAL_UNUSED_ARG(num_uids);
  return kEtcPalErrNotImpl;
}

/*
 * \brief Send an RDM command from an RPT client on a scope.
 *
 * The response will be delivered via an RptClientMsgReceivedCb containing an
 * RdmnetRemoteRdmResponse.
 *
 * \param[in] handle Handle to the client from which to send the RDM command.
 * \param[in] scope_handle Handle to the scope on which to send the RDM command.
 * \param[in] cmd The RDM command data to send, including its addressing information.
 * \param[out] seq_num Filled in on success with a sequence number which can be used to match the
 *                     command with a response.
 */
etcpal_error_t rc_client_send_rdm_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination,
                                          rdmnet_command_class_t command_class, uint16_t param_id, const uint8_t* data,
                                          uint8_t data_len, uint32_t* seq_num)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(destination);
  ETCPAL_UNUSED_ARG(command_class);
  ETCPAL_UNUSED_ARG(param_id);
  ETCPAL_UNUSED_ARG(data);
  ETCPAL_UNUSED_ARG(data_len);
  ETCPAL_UNUSED_ARG(seq_num);
  return kEtcPalErrNotImpl;
  //  if (handle < 0 || scope_handle < 0 || !cmd)
  //    return kEtcPalErrInvalid;
  //
  //  RdmnetClient* cli;
  //  RCClientScope* scope_entry;
  //  etcpal_error_t res = get_client_and_scope(handle, scope_handle, &cli, &scope_entry);
  //  if (res != kEtcPalErrOk)
  //    return res;
  //
  //  RptHeader header;
  //  header.source_uid = scope_entry->uid;
  //  header.source_endpoint_id = E133_NULL_ENDPOINT;
  //  header.dest_uid = cmd->rdmnet_dest_uid;
  //  header.dest_endpoint_id = cmd->dest_endpoint;
  //  header.seqnum = scope_entry->send_seq_num++;
  //
  //  RdmCommand rdm_to_send;
  //  rdm_to_send.source_uid = scope_entry->uid;
  //  rdm_to_send.dest_uid = cmd->rdm_dest_uid;
  //  rdm_to_send.port_id = 1;
  //  rdm_to_send.transaction_num = (uint8_t)(header.seqnum & 0xffu);
  //  rdm_to_send.subdevice = cmd->subdevice;
  //  rdm_to_send.command_class = cmd->command_class;
  //  rdm_to_send.param_id = cmd->param_id;
  //  rdm_to_send.data_len = cmd->data_len;
  //  memcpy(rdm_to_send.data, cmd->data, rdm_to_send.data_len);
  //
  //  RdmBuffer buf_to_send;
  //  res = rdmctl_pack_command(&rdm_to_send, &buf_to_send);
  //  if (res == kEtcPalErrOk)
  //  {
  //    res = rpt_send_request(scope_handle, &cli->cid, &header, &buf_to_send);
  //    if (res == kEtcPalErrOk && seq_num)
  //      *seq_num = header.seqnum;
  //  }
  //
  //  release_client_and_scope(cli, scope_entry);
  //  return res;
}

etcpal_error_t rc_client_send_get_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination, uint16_t param_id,
                                          const uint8_t* data, uint8_t data_len, uint32_t* seq_num)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(destination);
  ETCPAL_UNUSED_ARG(param_id);
  ETCPAL_UNUSED_ARG(data);
  ETCPAL_UNUSED_ARG(data_len);
  ETCPAL_UNUSED_ARG(seq_num);
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_set_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination, uint16_t param_id,
                                          const uint8_t* data, uint8_t data_len, uint32_t* seq_num)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(destination);
  ETCPAL_UNUSED_ARG(param_id);
  ETCPAL_UNUSED_ARG(data);
  ETCPAL_UNUSED_ARG(data_len);
  ETCPAL_UNUSED_ARG(seq_num);
  return kEtcPalErrNotImpl;
}

/*
 * \brief Send an RDM ACK response from an RPT client.
 *
 * \param[in] handle Handle to the device from which to send the RDM ACK response.
 * \param[in] scope_handle Handle to the scope from which to send the RDM ACK response.
 * \param[in] received_cmd Previously-received command that the ACK is a response to.
 * \param[in] response_data Parameter data that goes with this ACK, or NULL if no data.
 * \param[in] response_data_len Length in bytes of response_data, or 0 if no data.
 */
etcpal_error_t rc_rpt_client_send_rdm_ack(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetSavedRdmCommand* received_cmd, const uint8_t* response_data,
                                          size_t response_data_len)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(received_cmd);
  ETCPAL_UNUSED_ARG(response_data);
  ETCPAL_UNUSED_ARG(response_data_len);
  return kEtcPalErrNotImpl;
  //  if (handle < 0 || scope_handle < 0 || !resp)
  //    return kEtcPalErrInvalid;
  //
  //  RdmnetClient* cli;
  //  RCClientScope* scope_entry;
  //  etcpal_error_t res = get_client_and_scope(handle, scope_handle, &cli, &scope_entry);
  //  if (res != kEtcPalErrOk)
  //    return res;
  //
  //  size_t resp_buf_size = (resp->original_command_included ? resp->num_responses + 1 : resp->num_responses);
  //#if RDMNET_DYNAMIC_MEM
  //  RdmBuffer* resp_buf = (RdmBuffer*)calloc(resp_buf_size, sizeof(RdmBuffer));
  //  if (!resp_buf)
  //  {
  //    release_client_and_scope(cli, scope_entry);
  //    return kEtcPalErrNoMem;
  //  }
  //#else
  //  static RdmBuffer resp_buf[RDMNET_MAX_SENT_ACK_OVERFLOW_RESPONSES + 1];
  //  if (resp->num_responses > RDMNET_MAX_SENT_ACK_OVERFLOW_RESPONSES)
  //  {
  //    release_client_and_scope(cli, scope_entry);
  //    return kEtcPalErrMsgSize;
  //  }
  //#endif
  //
  //  RptHeader header;
  //  header.source_uid = scope_entry->uid;
  //  header.source_endpoint_id = resp->source_endpoint;
  //  header.dest_uid = resp->rdmnet_dest_uid;
  //  header.dest_endpoint_id = E133_NULL_ENDPOINT;
  //  header.seqnum = resp->seq_num;
  //
  //  if (resp->original_command_included)
  //  {
  //    res = rdmctl_pack_command(&resp->original_command, &resp_buf[0]);
  //  }
  //  if (res == kEtcPalErrOk)
  //  {
  //    for (size_t i = 0; i < resp->num_responses; ++i)
  //    {
  //      size_t out_buf_offset = resp->original_command_included ? i + 1 : i;
  //      RdmResponse resp_data = resp->responses[i];
  //      if (resp->source_endpoint == E133_NULL_ENDPOINT)
  //        resp_data.source_uid = scope_entry->uid;
  //      res = rdmresp_pack_response(&resp_data, &resp_buf[out_buf_offset]);
  //      if (res != kEtcPalErrOk)
  //        break;
  //    }
  //  }
  //  if (res == kEtcPalErrOk)
  //  {
  //    res = rpt_send_notification(scope_handle, &cli->cid, &header, resp_buf, resp_buf_size);
  //  }
  //
  //#if RDMNET_DYNAMIC_MEM
  //  free(resp_buf);
  //#endif
  //  release_client_and_scope(cli, scope_entry);
  //  return res;
}

etcpal_error_t rc_client_send_rdm_nack(RCClient* client, rdmnet_client_scope_t scope_handle,
                                       const RdmnetSavedRdmCommand* received_cmd, rdm_nack_reason_t nack_reason)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(received_cmd);
  ETCPAL_UNUSED_ARG(nack_reason);
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_rdm_update(RCClient* client, rdmnet_client_scope_t scope_handle,
                                         const RdmnetSourceAddr* source_addr, uint16_t param_id, const uint8_t* data,
                                         size_t data_len)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(source_addr);
  ETCPAL_UNUSED_ARG(param_id);
  ETCPAL_UNUSED_ARG(data);
  ETCPAL_UNUSED_ARG(data_len);
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_rpt_status(RCClient* client, rdmnet_client_scope_t scope_handle,
                                         const RdmnetSavedRdmCommand* received_cmd, rpt_status_code_t status_code,
                                         const char* status_string)
{
  //  if (handle < 0 || scope_handle < 0 || !status)
  //    return kEtcPalErrInvalid;
  //
  //  RdmnetClient* cli;
  //  RCClientScope* scope_entry;
  //  etcpal_error_t res = get_client_and_scope(handle, scope_handle, &cli, &scope_entry);
  //  if (res != kEtcPalErrOk)
  //    return res;
  //
  //  RptHeader header;
  //  header.source_uid = scope_entry->uid;
  //  header.source_endpoint_id = status->source_endpoint;
  //  header.dest_uid = status->rdmnet_dest_uid;
  //  header.dest_endpoint_id = E133_NULL_ENDPOINT;
  //  header.seqnum = status->seq_num;
  //
  //  res = rpt_send_status(scope_handle, &cli->cid, &header, &status->msg);
  //
  //  release_client_and_scope(cli, scope_entry);
  //  return res;
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_llrp_ack(RCClient* client, const LlrpSavedRdmCommand* received_cmd,
                                       const uint8_t* response_data, uint8_t response_data_len)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(received_cmd);
  ETCPAL_UNUSED_ARG(response_data);
  ETCPAL_UNUSED_ARG(response_data_len);
  return kEtcPalErrNotImpl;
  //  if (handle < 0 || !resp)
  //    return kEtcPalErrInvalid;
  //
  //  RdmnetClient* cli;
  //  etcpal_error_t res = get_client(handle, &cli);
  //  if (res != kEtcPalErrOk)
  //    return res;
  //
  //  res = llrp_target_send_rdm_response(cli->llrp_handle, resp);
  //
  //  release_client(cli);
  //  return res;
}

etcpal_error_t rc_client_send_llrp_nack(RCClient* client, const LlrpSavedRdmCommand* received_cmd,
                                        rdm_nack_reason_t nack_reason)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(received_cmd);
  ETCPAL_UNUSED_ARG(nack_reason);
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_ept_data(RCClient* client, rdmnet_client_scope_t scope_handle, const EtcPalUuid* dest_cid,
                                       uint16_t manufacturer_id, uint16_t protocol_id, const uint8_t* data,
                                       size_t data_len)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(dest_cid);
  ETCPAL_UNUSED_ARG(data);
  return kEtcPalErrNotImpl;
}

etcpal_error_t rc_client_send_ept_status(RCClient* client, rdmnet_client_scope_t scope_handle,
                                         const EtcPalUuid* dest_cid, ept_status_code_t status_code,
                                         const char* status_string)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope_handle);
  ETCPAL_UNUSED_ARG(dest_cid);
  ETCPAL_UNUSED_ARG(status_code);
  ETCPAL_UNUSED_ARG(status_string);
  return kEtcPalErrNotImpl;
}

// Callback functions from the discovery interface

/*
void monitorcb_broker_found(rdmnet_scope_monitor_t handle, const RdmnetBrokerDiscInfo* broker_info, void* context)
{
  ETCPAL_UNUSED_ARG(context);

  RDMNET_LOG_INFO("Broker '%s' for scope '%s' discovered.", broker_info->service_name, broker_info->scope);

  RCClientScope* scope_entry = get_scope_by_disc_handle(handle);
  if (scope_entry && !scope_entry->broker_found)
  {
    scope_entry->broker_found = true;
    scope_entry->listen_addrs = broker_info->listen_addrs;
    scope_entry->num_listen_addrs = broker_info->num_listen_addrs;
    scope_entry->current_listen_addr = 0;
    scope_entry->port = broker_info->port;

    attempt_connection_on_listen_addrs(scope_entry);

    release_scope(scope_entry);
  }
}

void monitorcb_broker_lost(rdmnet_scope_monitor_t handle, const char* scope, const char* service_name, void* context)
{
  ETCPAL_UNUSED_ARG(context);

  RCClientScope* scope_entry = get_scope_by_disc_handle(handle);
  if (scope_entry)
  {
    scope_entry->broker_found = false;
    scope_entry->listen_addrs = NULL;
    scope_entry->num_listen_addrs = 0;
    scope_entry->current_listen_addr = 0;
    scope_entry->port = 0;

    release_scope(scope_entry);
  }
  RDMNET_LOG_INFO("Broker '%s' no longer discovered on scope '%s'", service_name, scope);
}

void monitorcb_scope_monitor_error(rdmnet_scope_monitor_t handle, const char* scope, int platform_error, void*
context)
{
  ETCPAL_UNUSED_ARG(handle);
  ETCPAL_UNUSED_ARG(scope);
  ETCPAL_UNUSED_ARG(platform_error);
  ETCPAL_UNUSED_ARG(context);

  // TODO
}
*/

rc_lock_state_t conncb_connected(RCConnection* conn, const RCConnectedInfo* connected_info)
{
  RCClientScope* scope_entry = GET_CLIENT_SCOPE_FROM_CONN(conn);
  RCClient* client = scope_entry->client;

  RDMNET_ASSERT(client->type == kClientProtocolRPT || client->type == kClientProtocolEPT);

  scope_entry->state = kRCScopeStateConnected;

  RdmnetClientConnectedInfo cli_conn_info;
  cli_conn_info.broker_addr = connected_info->connected_addr;
  // TODO cli_conn_info.broker_name =
  // TODO cli_conn_info.broker_cid =
  cli_conn_info.broker_uid = connected_info->broker_uid;

  if (client->type == kClientProtocolRPT)
  {
    if (!RDMNET_UID_IS_STATIC(&RC_RPT_CLIENT_DATA(client)->uid))
      scope_entry->uid = connected_info->client_uid;
  }

  return client->callbacks.connected(client, GET_SCOPE_HANDLE(client, scope_entry), &cli_conn_info);
}

rc_lock_state_t conncb_connect_failed(RCConnection* conn, const RCConnectFailedInfo* failed_info)
{
  RCClientScope* scope_entry = GET_CLIENT_SCOPE_FROM_CONN(conn);
  RCClient* client = scope_entry->client;

  RDMNET_ASSERT(client->type == kClientProtocolRPT || client->type == kClientProtocolEPT);

  scope_entry->state = kRCScopeStateDiscovery;

  RdmnetClientConnectFailedInfo cli_conn_failed_info;
  cli_conn_failed_info.event = failed_info->event;
  cli_conn_failed_info.socket_err = failed_info->socket_err;
  cli_conn_failed_info.rdmnet_reason = failed_info->rdmnet_reason;
  cli_conn_failed_info.will_retry =
      connect_failed_will_retry(cli_conn_failed_info.event, cli_conn_failed_info.rdmnet_reason);

  if (cli_conn_failed_info.will_retry)
  {
    if (scope_entry->monitor_handle != RDMNET_SCOPE_MONITOR_INVALID)
    {
      if (scope_entry->broker_found)
      {
        // Attempt to connect on the next listen address.
        if (++scope_entry->current_listen_addr == scope_entry->num_listen_addrs)
          scope_entry->current_listen_addr = 0;
        attempt_connection_on_listen_addrs(scope_entry);
      }
    }
    else
    {
      if (kEtcPalErrOk != start_connection_for_scope(scope_entry, &scope_entry->static_broker_addr))
      {
        // Some fatal error while attempting to connect to the statically-configured address.
        cli_conn_failed_info.will_retry = false;
      }
    }
  }

  return client->callbacks.connect_failed(client, GET_SCOPE_HANDLE(client, scope_entry), &cli_conn_failed_info);
}

rc_lock_state_t conncb_disconnected(RCConnection* conn, const RCDisconnectedInfo* disconn_info)
{
  RCClientScope* scope_entry = GET_CLIENT_SCOPE_FROM_CONN(conn);
  RCClient* client = scope_entry->client;

  RDMNET_ASSERT(client->type == kClientProtocolRPT || client->type == kClientProtocolEPT);

  RdmnetClientDisconnectedInfo cli_disconn_info;
  cli_disconn_info.event = disconn_info->event;
  cli_disconn_info.socket_err = disconn_info->socket_err;
  cli_disconn_info.rdmnet_reason = disconn_info->rdmnet_reason;
  cli_disconn_info.will_retry = disconnected_will_retry(cli_disconn_info.event, cli_disconn_info.rdmnet_reason);

  if (cli_disconn_info.will_retry)
  {
    // Retry connection on the scope.
    scope_entry->state = kRCScopeStateConnecting;
    if (scope_entry->monitor_handle != RDMNET_SCOPE_MONITOR_INVALID)
    {
      if (scope_entry->broker_found)
      {
        // Attempt to connect to the Broker on its reported listen addresses.
        attempt_connection_on_listen_addrs(scope_entry);
      }
    }
    else
    {
      if (kEtcPalErrOk != start_connection_for_scope(scope_entry, &scope_entry->static_broker_addr))
      {
        // Some fatal error while attempting to connect to the statically-configured address.
        cli_disconn_info.will_retry = false;
      }
    }
  }

  return client->callbacks.disconnected(client, GET_SCOPE_HANDLE(client, scope_entry), &cli_disconn_info);
}

rc_lock_state_t conncb_msg_received(RCConnection* conn, const RdmnetMessage* message)
{
  RCClientScope* scope_entry = GET_CLIENT_SCOPE_FROM_CONN(conn);
  RCClient* client = scope_entry->client;

  RDMNET_ASSERT(client->type == kClientProtocolRPT || client->type == kClientProtocolEPT);

  switch (message->vector)
  {
    case ACN_VECTOR_ROOT_BROKER:
      return client->callbacks.broker_msg_received(client, GET_SCOPE_HANDLE(client, scope_entry),
                                                   RDMNET_GET_BROKER_MSG(message));
    case ACN_VECTOR_ROOT_RPT:
      // if (client->type == kClientProtocolRPT)
      // {
      //   if (handle_rpt_message(client, scope_entry, RDMNET_GET_RPT_MSG(message),
      //   &cb.prot_info.rpt.args.msg_received))
      //   {
      //     cb.which = kClientCallbackMsgReceived;
      //   }
      // }
      // else
      // {
      //   RDMNET_LOG_WARNING("Incorrectly got RPT message for non-RPT client %d on scope %d", cli->handle, handle);
      // }
      // break;
      // TODO
      return kRCLockStateLocked;
    case ACN_VECTOR_ROOT_EPT:
      // TODO, for now fall through
    default:
      // RDMNET_LOG_WARNING("Got message with unhandled vector type %" PRIu32 " on scope %d", message->vector, handle);
      return kRCLockStateLocked;
  }
}

/*
bool handle_rpt_message(const RdmnetClient* cli, const RCClientScope* scope_entry, const RptMessage* rmsg,
                        RptMsgReceivedArgs* cb_args)
{
  ETCPAL_UNUSED_ARG(cli);
  bool res = false;

  switch (rmsg->vector)
  {
    case VECTOR_RPT_REQUEST:
      res = handle_rpt_request(rmsg, &cb_args->msg);
      break;
    case VECTOR_RPT_NOTIFICATION:
      res = handle_rpt_notification(rmsg, &cb_args->msg);
      break;
    case VECTOR_RPT_STATUS:
      res = handle_rpt_status(rmsg, &cb_args->msg);
      break;
  }

  if (res)
    cb_args->scope_handle = scope_entry->handle;
  return res;
}

bool handle_rpt_request(const RptMessage* rmsg, RptClientMessage* msg_out)
{
  ETCPAL_UNUSED_ARG(rmsg);
  ETCPAL_UNUSED_ARG(msg_out);
  //  RdmnetRemoteRdmCommand* cmd = RDMNET_GET_REMOTE_RDM_COMMAND(msg_out);
  //  const RptRdmBufList* list = RPT_GET_RDM_BUF_LIST(rmsg);
  //
  //  if (list->num_rdm_buffers == 1)  // Only one RDM command allowed in an RPT request
  //  {
  //    etcpal_error_t unpack_res = rdmresp_unpack_command(list->rdm_buffers, &cmd->rdm_command);
  //    if (unpack_res == kEtcPalErrOk)
  //    {
  //      msg_out->type = kRptClientMsgRdmCmd;
  //      cmd->source_uid = rmsg->header.source_uid;
  //      cmd->dest_endpoint = rmsg->header.dest_endpoint_id;
  //      cmd->seq_num = rmsg->header.seqnum;
  //      return true;
  //    }
  //  }
  return false;
}

bool handle_rpt_notification(const RptMessage* rmsg, RptClientMessage* msg_out)
{
  ETCPAL_UNUSED_ARG(rmsg);
  ETCPAL_UNUSED_ARG(msg_out);
  return false;
  //  RdmnetRemoteRdmResponse* resp = RDMNET_GET_REMOTE_RDM_RESPONSE(msg_out);
  //
  //  // Do some initialization
  //  msg_out->type = kRptClientMsgRdmResp;
  //  resp->command_included = false;
  //  resp->more_coming = RPT_GET_RDM_BUF_LIST(rmsg)->more_coming;
  //
  //  const RptRdmBufList* list = RPT_GET_RDM_BUF_LIST(rmsg);
  //  resp->responses = ALLOC_CLIENT_RDM_RESPONSE_ARRAY(list->num_rdm_buffers);
  //  if (!resp->responses)
  //    return false;
  //
  //  bool good_parse = true;
  //  bool first_msg = true;
  //  for (size_t i = 0; i < list->num_rdm_buffers; ++i)
  //  {
  //    const RdmBuffer* buffer = &list->rdm_buffers[i];
  //    if (first_msg)
  //    {
  //      if (rdmresp_is_non_disc_command(buffer))
  //      {
  //        // The command is included.
  //        etcpal_error_t unpack_res = rdmresp_unpack_command(buffer, &resp->cmd);
  //        if (unpack_res == kEtcPalErrOk)
  //        {
  //          resp->command_included = true;
  //        }
  //        else
  //        {
  //          good_parse = false;
  //        }
  //        continue;
  //      }
  //      first_msg = false;
  //    }
  //
  //    etcpal_error_t unpack_res = rdmctl_unpack_response(buffer, &resp->responses[i]);
  //    if (unpack_res != kEtcPalErrOk)
  //      good_parse = false;
  //  }
  //
  //  if (good_parse)
  //  {
  //    // Fill in the rest of the info
  //    resp->rdmnet_source_uid = rmsg->header.source_uid;
  //    resp->source_endpoint = rmsg->header.source_endpoint_id;
  //    resp->seq_num = rmsg->header.seqnum;
  //    return true;
  //  }
  //  else
  //  {
  //    // Clean up
  //    free_rpt_client_message(msg_out);
  //    return false;
  //  }
}

bool handle_rpt_status(const RptMessage* rmsg, RptClientMessage* msg_out)
{
  ETCPAL_UNUSED_ARG(rmsg);
  ETCPAL_UNUSED_ARG(msg_out);
  return false;
  //  RdmnetRemoteRptStatus* status_out = RDMNET_GET_REMOTE_RPT_STATUS(msg_out);
  //  const RptStatusMsg* status = RPT_GET_STATUS_MSG(rmsg);
  //
  //  // This one is quick and simple with no failure condition
  //  msg_out->type = kRptClientMsgStatus;
  //  status_out->source_uid = rmsg->header.source_uid;
  //  status_out->source_endpoint = rmsg->header.source_endpoint_id;
  //  status_out->seq_num = rmsg->header.seqnum;
  //  status_out->msg = *status;
  //  return true;
}

void free_rpt_client_message(RptClientMessage* msg)
{
  ETCPAL_UNUSED_ARG(msg);
  //  if (msg->type == kRptClientMsgRdmResp)
  //  {
  //    FREE_CLIENT_RDM_RESPONSE_ARRAY(RDMNET_GET_REMOTE_RDM_RESPONSE(msg)->responses);
  //  }
}

void free_ept_client_message(EptClientMessage* msg)
{
  ETCPAL_UNUSED_ARG(msg);
  // TODO
}
*/

rc_lock_state_t llrpcb_rdm_cmd_received(RCLlrpTarget* target, const LlrpRdmCommand* cmd, RdmnetSyncRdmResponse* resp)
{
  RCClient* cli = GET_CLIENT_FROM_LLRP_TARGET(target);
  return RC_RPT_CLIENT_DATA(cli)->callbacks.llrp_msg_received(cli, cmd, resp);
}

bool connect_failed_will_retry(rdmnet_connect_fail_event_t event, rdmnet_connect_status_t status)
{
  switch (event)
  {
    case kRdmnetConnectFailSocketFailure:
      return false;
    case kRdmnetConnectFailRejected:
      return (status == kRdmnetConnectCapacityExceeded);
    case kRdmnetConnectFailTcpLevel:
    case kRdmnetConnectFailNoReply:
    default:
      return true;
  }
}

bool disconnected_will_retry(rdmnet_disconnect_event_t event, rdmnet_disconnect_reason_t reason)
{
  ETCPAL_UNUSED_ARG(event);
  ETCPAL_UNUSED_ARG(reason);
  // Currently all disconnects are retried.
  return true;
}

/*
// Validate the data in an RdmnetRptClientConfig structure.
etcpal_error_t validate_rpt_client_config(const RdmnetRptClientConfig* config)
{
  if ((config->type != kRPTClientTypeDevice && config->type != kRPTClientTypeController) ||
      (ETCPAL_UUID_IS_NULL(&config->cid)) ||
      (!RDMNET_UID_IS_DYNAMIC_UID_REQUEST(&config->uid) && (config->uid.manu & 0x8000)) || !config->search_domain)
  {
    return kEtcPalErrInvalid;
  }
  return kEtcPalErrOk;
}

// Create and initialize a new RdmnetClient structure from a given RPT config.
etcpal_error_t new_rpt_client(const RdmnetRptClientConfig* config, rdmnet_client_t* handle)
{
  etcpal_error_t res = kEtcPalErrNoMem;

  rdmnet_client_t new_handle = get_next_int_handle(&state.handle_mgr);
  if (new_handle == RDMNET_CLIENT_INVALID)
    return res;

  RdmnetClient* new_cli = (RdmnetClient*)ALLOC_RDMNET_CLIENT();
  if (new_cli)
  {
    res = create_llrp_handle_for_client(config, new_cli);
    if (res == kEtcPalErrOk)
    {
      new_cli->handle = new_handle;
      if (kEtcPalErrOk == etcpal_rbtree_insert(&state.clients, new_cli))
      {
        // Init the client data
        new_cli->type = kClientProtocolRPT;
        new_cli->cid = config->cid;
        new_cli->data.rpt.callbacks = config->callbacks;
        new_cli->callback_context = config->callback_context;
        rdmnet_safe_strncpy(new_cli->search_domain, config->search_domain, E133_DOMAIN_STRING_PADDED_LENGTH);
        new_cli->data.rpt.type = config->type;
        if (RDMNET_UID_IS_DYNAMIC_UID_REQUEST(&config->uid))
        {
          new_cli->data.rpt.has_static_uid = false;
          new_cli->data.rpt.uid.manu = config->uid.manu;
        }
        else
        {
          new_cli->data.rpt.has_static_uid = true;
          new_cli->data.rpt.uid = config->uid;
        }
        new_cli->scope_list = NULL;
        *handle = new_handle;
      }
      else
      {
        etcpal_rbtree_remove(&state.clients_by_llrp_handle, new_cli);
        FREE_RDMNET_CLIENT(new_cli);
        res = kEtcPalErrNoMem;
      }
    }
    else
    {
      FREE_RDMNET_CLIENT(new_cli);
    }
  }

  return res;
}

void destroy_client(RdmnetClient* cli, rdmnet_disconnect_reason_t reason)
{
  RCClientScope* scope = cli->scope_list;

  while (scope)
  {
    if (scope->monitor_handle)
    {
      rdmnet_disc_stop_monitoring(scope->monitor_handle);
      etcpal_rbtree_remove(&state.scopes_by_disc_handle, scope);
    }
    rdmnet_connection_destroy(scope->handle, &reason);
    etcpal_rbtree_remove(&state.scopes_by_handle, scope);

    RCClientScope* next_scope = scope->next;
    FREE_CLIENT_SCOPE(scope);
    scope = next_scope;
  }

  llrp_target_destroy(cli->llrp_handle);

  FREE_RDMNET_CLIENT(cli);
}

// Callback for IntHandleManager to determine whether a handle is in use.
bool client_handle_in_use(int handle_val)
{
  return etcpal_rbtree_find(&state.clients, &handle_val);
}
*/

// Allocate a new scope list entry and append it to a client's scope list. If a scope string is
// already in the list, fails with kEtcPalErrExists. Attempts to create a new connection handle to
// accompany the scope. Returns kEtcPalErrOk on success, other error code otherwise. Fills in
// new_entry with the newly-created entry on success.
etcpal_error_t create_and_add_scope_entry(RCClient* client, const RdmnetScopeConfig* config, int* new_entry_index)
{
  RCClientScope* new_scope = get_unused_scope_entry(client);
  if (!new_scope)
    return kEtcPalErrNoMem;

  new_scope->conn.local_cid = client->cid;
  new_scope->conn.lock = client->lock;
  new_scope->conn.callbacks = conn_callbacks;

  etcpal_error_t res = rc_connection_register(&new_scope->conn);
  if (res != kEtcPalErrOk)
    return res;

  // Do the rest of the initialization
  rdmnet_safe_strncpy(new_scope->id, config->scope, E133_SCOPE_STRING_PADDED_LENGTH);
  new_scope->static_broker_addr = config->static_broker_addr;
  if (!ETCPAL_IP_IS_INVALID(&new_scope->static_broker_addr.ip))
    new_scope->state = kRCScopeStateConnecting;
  else
    new_scope->state = kRCScopeStateDiscovery;
  // uid init is done at connection time
  new_scope->send_seq_num = 1;
  new_scope->monitor_handle = RDMNET_SCOPE_MONITOR_INVALID;
  new_scope->broker_found = false;
  new_scope->listen_addrs = NULL;
  new_scope->num_listen_addrs = 0;
  new_scope->current_listen_addr = 0;
  new_scope->port = 0;
  new_scope->client = client;

  *new_entry_index = (int)(new_scope - client->scopes);
  return kEtcPalErrOk;
}

bool scope_exists_in_list(RCClient* client, const char* scope)
{
  for (RCClientScope* scope_entry = client->scopes; scope_entry < client->scopes + client->num_scopes; ++scope_entry)
  {
    if (strcmp(scope_entry->id, scope) == 0)
    {
      // Found
      return true;
    }
  }
  return false;
}

RCClientScope* get_unused_scope_entry(RCClient* client)
{
#if RDMNET_DYNAMIC_MEM
  for (RCClientScope* scope = client->scopes; scope < client->scopes + client->scopes_capacity; ++scope)
  {
    if (!scope->valid)
      return scope;
  }

  RDMNET_ASSERT(client->num_scopes == client->scopes_capacity);

  // Add a new scope to the end of the scope list, reallocating if necessary. The reallocation
  // grows the capacity by a factor of 2 each time.
  size_t new_capacity = client->scopes_capacity * 2;
  RCClientScope* new_buffer = (RCClientScope*)realloc(client->scopes, new_capacity * sizeof(RCClientScope));
  if (new_buffer)
  {
    for (RCClientScope* allocated_scope = &new_buffer[client->scopes_capacity];
         allocated_scope < new_buffer + new_capacity; ++allocated_scope)
    {
      allocated_scope->valid = false;
    }
    client->scopes = new_buffer;
    client->scopes_capacity = new_capacity;
    return &client->scopes[client->num_scopes];
  }
  else
  {
    return NULL;
  }
#else
  for (RCClientScope* scope = client->scopes; scope < client->scopes + RDMNET_MAX_SCOPES_PER_CLIENT; ++scope)
  {
    if (!scope->valid)
      return scope;
  }
  return NULL;
#endif
}

void cleanup_scope_entry(RCClient* client, RCClientScope* scope)
{
  // TODO fix
  rc_connection_unregister(&scope->conn, NULL);
  scope->valid = false;
}

etcpal_error_t start_scope_discovery(RCClientScope* scope_entry, const char* search_domain)
{
  // TODO
  return kEtcPalErrNotImpl;
  //  RdmnetScopeMonitorConfig config;
  //
  //  rdmnet_safe_strncpy(config.scope, scope_entry->id, E133_SCOPE_STRING_PADDED_LENGTH);
  //  rdmnet_safe_strncpy(config.domain, search_domain, E133_DOMAIN_STRING_PADDED_LENGTH);
  //  config.callbacks = disc_callbacks;
  //  config.callback_context = NULL;
  //
  //  // TODO capture errors
  //  int platform_error;
  //  etcpal_error_t res = rdmnet_disc_start_monitoring(&config, &scope_entry->monitor_handle, &platform_error);
  //  if (res == kEtcPalErrOk)
  //  {
  //    etcpal_rbtree_insert(&state.scopes_by_disc_handle, scope_entry);
  //  }
  //  else
  //  {
  //    RDMNET_LOG_WARNING("Starting discovery failed on scope '%s' with error '%s' (platform-specific error code %d)",
  //                       scope_entry->id, etcpal_strerror(res), platform_error);
  //  }
  //  return res;
}

void attempt_connection_on_listen_addrs(RCClientScope* scope_entry)
{
  /*
  size_t listen_addr_index = scope_entry->current_listen_addr;

  while (true)
  {
    char addr_str[ETCPAL_INET6_ADDRSTRLEN] = {'\0'};

    if (RDMNET_CAN_LOG(ETCPAL_LOG_WARNING))
    {
      etcpal_ip_to_string(&scope_entry->listen_addrs[listen_addr_index], addr_str);
    }

    RDMNET_LOG_INFO("Attempting broker connection on scope '%s' at address %s:%d...", scope_entry->id, addr_str,
                    scope_entry->port);

    EtcPalSockAddr connect_addr;
    connect_addr.ip = scope_entry->listen_addrs[listen_addr_index];
    connect_addr.port = scope_entry->port;

    etcpal_error_t connect_res = start_connection_for_scope(scope_entry, &connect_addr);
    if (connect_res == kEtcPalErrOk)
    {
      scope_entry->current_listen_addr = listen_addr_index;
      break;
    }
    else
    {
      if (++listen_addr_index == scope_entry->num_listen_addrs)
        listen_addr_index = 0;
      if (listen_addr_index == scope_entry->current_listen_addr)
      {
        // We've looped through all the addresses. This broker is no longer valid.
        scope_entry->broker_found = false;
        scope_entry->listen_addrs = NULL;
        scope_entry->num_listen_addrs = 0;
        scope_entry->current_listen_addr = 0;
        scope_entry->port = 0;
      }

      RDMNET_LOG_WARNING("Connection to broker for scope '%s' at address %s:%d failed with error: '%s'. %s",
                         scope_entry->id, addr_str, connect_addr.port, etcpal_strerror(connect_res),
                         scope_entry->broker_found ? "Trying next address..." : "All addresses exhausted. Giving up.");

      if (!scope_entry->broker_found)
        break;
    }
  }
  */
}

etcpal_error_t start_connection_for_scope(RCClientScope* scope_entry, const EtcPalSockAddr* broker_addr)
{
  // TODO
  return kEtcPalErrNotImpl;
  /*
  BrokerClientConnectMsg connect_msg;
  RdmnetClient* cli = scope_entry->client;

  if (cli->type == kClientProtocolRPT)
  {
    RptClientData* rpt_data = &cli->data.rpt;
    RdmUid my_uid;
    if (rpt_data->has_static_uid)
    {
      my_uid = rpt_data->uid;
    }
    else
    {
      RDMNET_INIT_DYNAMIC_UID_REQUEST(&my_uid, rpt_data->uid.manu);
    }

    rdmnet_safe_strncpy(connect_msg.scope, scope_entry->id, E133_SCOPE_STRING_PADDED_LENGTH);
    connect_msg.e133_version = E133_VERSION;
    rdmnet_safe_strncpy(connect_msg.search_domain, cli->search_domain, E133_DOMAIN_STRING_PADDED_LENGTH);
    if (rpt_data->type == kRPTClientTypeController)
      connect_msg.connect_flags = BROKER_CONNECT_FLAG_INCREMENTAL_UPDATES;
    else
      connect_msg.connect_flags = 0;
    connect_msg.client_entry.client_protocol = kClientProtocolRPT;
    create_rpt_client_entry(&cli->cid, &my_uid, rpt_data->type, NULL, GET_RPT_CLIENT_ENTRY(&connect_msg.client_entry));
  }
  else
  {
    // TODO EPT
    return kEtcPalErrNotImpl;
  }

  return rdmnet_connect(scope_entry->handle, broker_addr, &connect_msg);
  */
}

/*
etcpal_error_t get_client(rdmnet_client_t handle, RdmnetClient** client)
{
  if (!rdmnet_core_initialized())
    return kEtcPalErrNotInit;
  if (!RDMNET_CLIENT_LOCK())
    return kEtcPalErrSys;

  RdmnetClient* found_cli = (RdmnetClient*)etcpal_rbtree_find(&state.clients, &handle);
  if (!found_cli)
  {
    RDMNET_CLIENT_UNLOCK();
    return kEtcPalErrNotFound;
  }
  *client = found_cli;
  // Return keeping the lock
  return kEtcPalErrOk;
}

RdmnetClient* get_client_by_llrp_handle(llrp_target_t handle)
{
  if (!RDMNET_CLIENT_LOCK())
    return NULL;

  RdmnetClient llrp_cmp;
  llrp_cmp.llrp_handle = handle;
  RdmnetClient* found_cli = (RdmnetClient*)etcpal_rbtree_find(&state.clients_by_llrp_handle, &llrp_cmp);
  if (!found_cli)
  {
    RDMNET_CLIENT_UNLOCK();
    return NULL;
  }
  // Return keeping the lock
  return found_cli;
}

void release_client(const RdmnetClient* client)
{
  ETCPAL_UNUSED_ARG(client);
  RDMNET_CLIENT_UNLOCK();
}

RCClientScope* get_scope(rdmnet_client_scope_t handle)
{
  if (!RDMNET_CLIENT_LOCK())
    return NULL;

  RCClientScope* found_scope = (RCClientScope*)etcpal_rbtree_find(&state.scopes_by_handle, &handle);
  if (!found_scope)
  {
    RDMNET_CLIENT_UNLOCK();
    return NULL;
  }
  // Return keeping the lock
  return found_scope;
}

RCClientScope* get_scope_by_disc_handle(rdmnet_scope_monitor_t handle)
{
  if (!RDMNET_CLIENT_LOCK())
    return NULL;

  RCClientScope scope_cmp;
  scope_cmp.monitor_handle = handle;
  RCClientScope* found_scope = (RCClientScope*)etcpal_rbtree_find(&state.scopes_by_disc_handle, &scope_cmp);
  if (!found_scope)
  {
    RDMNET_CLIENT_UNLOCK();
    return NULL;
  }
  // Return keeping the lock
  return found_scope;
}

void release_scope(const RCClientScope* scope_entry)
{
  ETCPAL_UNUSED_ARG(scope_entry);
  RDMNET_CLIENT_UNLOCK();
}

etcpal_error_t get_client_and_scope(rdmnet_client_t handle, rdmnet_client_scope_t scope_handle, RdmnetClient** client,
                                    RCClientScope** scope_entry)
{
  RdmnetClient* found_cli;
  etcpal_error_t res = get_client(handle, &found_cli);
  if (res != kEtcPalErrOk)
    return res;

  RCClientScope* found_scope = (RCClientScope*)etcpal_rbtree_find(&state.scopes_by_handle, &scope_handle);
  if (!found_scope)
  {
    release_client(found_cli);
    return kEtcPalErrNotFound;
  }
  if (found_scope->client != found_cli)
  {
    release_client(found_cli);
    return kEtcPalErrInvalid;
  }
  *client = found_cli;
  *scope_entry = found_scope;
  // Return keeping the lock
  return kEtcPalErrOk;
}

void release_client_and_scope(const RdmnetClient* client, const RCClientScope* scope)
{
  ETCPAL_UNUSED_ARG(client);
  ETCPAL_UNUSED_ARG(scope);
  RDMNET_CLIENT_UNLOCK();
}

*/
