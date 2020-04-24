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

#ifndef RDMNET_CORE_CLIENT_H_
#define RDMNET_CORE_CLIENT_H_

#include "etcpal/uuid.h"
#include "etcpal/inet.h"
#include "etcpal/lock.h"
#include "rdm/uid.h"
#include "rdm/message.h"
#include "rdmnet/defs.h"
#include "rdmnet/discovery.h"
#include "rdmnet/client.h"
#include "rdmnet/message.h"
#include "rdmnet/private/opts.h"
#include "rdmnet/core/broker_prot.h"
#include "rdmnet/core/common.h"
#include "rdmnet/core/connection.h"
#include "rdmnet/core/llrp_target.h"

/*
 * rdmnet/core/client.h: Implementation of RDMnet client functionality
 *
 * RDMnet clients encompass controllers (which originate RDM commands on the network and receive
 * responses) and devices (which receive RDM commands and respond to them), as well as EPT clients,
 * which use the Extensible Packet Transport feature of RDMnet to transport opaque data through a
 * broker.
 *
 * Clients and the scopes they participate in are tracked by handles. Management of connections to
 * brokers, as well as LLRP functionality, is handled under the hood.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if RDMNET_MAX_CONTROLLERS != 0
#define RDMNET_MAX_SCOPES_PER_CLIENT RDMNET_MAX_SCOPES_PER_CONTROLLER
#else
#define RDMNET_MAX_SCOPES_PER_CLIENT 1
#endif

typedef struct RCClient RCClient;

typedef enum
{
  kRptClientMsgRdmCmd,
  kRptClientMsgRdmResp,
  kRptClientMsgStatus
} rpt_client_msg_t;

typedef struct RptClientMessage
{
  rpt_client_msg_t type;
  union
  {
    RdmnetRdmCommand cmd;
    RdmnetRdmResponse resp;
    RdmnetRptStatus status;
  } payload;
} RptClientMessage;

#define RDMNET_GET_RDM_COMMAND(rptclimsgptr) (&(rptclimsgptr)->payload.cmd)
#define RDMNET_GET_RDM_RESPONSE(rptclimsgptr) (&(rptclimsgptr)->payload.resp)
#define RDMNET_GET_RPT_STATUS(rptclimsgptr) (&(rptclimsgptr)->payload.status)

typedef enum
{
  kEptClientMsgData,
  kEptClientMsgStatus
} ept_client_msg_t;

typedef struct EptClientMessage
{
  ept_client_msg_t type;
  union
  {
    RdmnetEptStatus status;
    RdmnetEptData data;
  } payload;
} EptClientMessage;

/**************************************************************************************************
 * Client callback functions: Function types used as callbacks for RPT and EPT clients.
 *************************************************************************************************/

/* A client has connected successfully to a broker on a scope. */
typedef rc_lock_state_t (*RCClientConnectedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                               const RdmnetClientConnectedInfo* info);

/* A client experienced a failure while attempting to connect to a broker on a scope. */
typedef rc_lock_state_t (*RCClientConnectFailedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                                   const RdmnetClientConnectFailedInfo* info);

/* A client disconnected from a broker on a scope. */
typedef rc_lock_state_t (*RCClientDisconnectedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                                  const RdmnetClientDisconnectedInfo* info);

/*
 * A broker message has been received on a client connection.
 *
 * Broker messages are exchanged between an RDMnet client and broker to setup and faciliate RDMnet
 * communication. Use the macros from broker_prot.h to inspect the BrokerMessage.
 */
typedef rc_lock_state_t (*RCClientBrokerMsgReceivedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                                       const BrokerMessage* msg);

/* An LLRP RDM command has been received by a client. */
typedef rc_lock_state_t (*RCClientLlrpMsgReceivedCb)(RCClient* client, const LlrpRdmCommand* cmd,
                                                     RdmnetSyncRdmResponse* response);

/*
 * An RPT message was received on an RPT client connection.
 *
 * RPT messages include Request and Notification, which wrap RDM commands and responses, as well as
 * Status, which informs of exceptional conditions in response to a Request. Use the macros from
 * this header to inspect the RptClientMessage.
 */
typedef rc_lock_state_t (*RCClientRptMsgReceivedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                                    const RptClientMessage* msg, RdmnetSyncRdmResponse* response);

/*
 * An EPT message was received on an EPT client connection.
 *
 * EPT messages include Data, which wraps opaque data, and Status, which informs of exceptional
 * conditions in response to Data.
 */
typedef rc_lock_state_t (*RCClientEptMsgReceivedCb)(RCClient* client, rdmnet_client_scope_t scope_handle,
                                                    const EptClientMessage* msg, RdmnetSyncEptResponse* response);

/* The set of possible callbacks that are delivered to an RPT client. */
typedef struct RCRptClientCallbacks
{
  RCClientConnectedCb connected;
  RCClientConnectFailedCb connect_failed;
  RCClientDisconnectedCb disconnected;
  RCClientBrokerMsgReceivedCb broker_msg_received;
  RCClientLlrpMsgReceivedCb llrp_msg_received;
  RCClientRptMsgReceivedCb rpt_msg_received;
} RCRptClientCallbacks;

/* The set of possible callbacks that are delivered to an EPT client. */
typedef struct RCEptClientCallbacks
{
  RCClientConnectedCb connected;
  RCClientConnectFailedCb connect_failed;
  RCClientDisconnectedCb disconnected;
  RCClientBrokerMsgReceivedCb broker_msg_received;
  RCClientEptMsgReceivedCb msg_received;
} RCEptClientCallbacks;

typedef enum
{
  kRCScopeStateDiscovery,
  kRCScopeStateConnecting,
  kRCScopeStateConnected
} rc_scope_state_t;

typedef struct RCClientScope
{
  bool valid;

  char id[E133_SCOPE_STRING_PADDED_LENGTH];
  EtcPalSockAddr static_broker_addr;
  rc_scope_state_t state;
  RdmUid uid;
  uint32_t send_seq_num;

  rdmnet_scope_monitor_t monitor_handle;
  bool broker_found;
  const EtcPalIpAddr* listen_addrs;
  size_t num_listen_addrs;
  size_t current_listen_addr;
  uint16_t port;

  RCConnection conn;

  RCClient* client;
} RCClientScope;

typedef struct RCRptClientData
{
  rpt_client_type_t type;
  RdmUid uid;
  RCRptClientCallbacks callbacks;
} RCRptClientData;

typedef struct RCEptClientData
{
  RDMNET_DECLARE_BUF(RdmnetEptSubProtocol, protocols, RDMNET_MAX_PROTOCOLS_PER_EPT_CLIENT);
  RCEptClientCallbacks callbacks;
} RCEptClientData;

struct RCClient
{
  // Fill in these items before initializing the RdmnetClient.
  etcpal_mutex_t* lock;
  client_protocol_t type;
  EtcPalUuid cid;
  union
  {
    RCRptClientData rpt;
    RCEptClientData ept;
  } data;
  char search_domain[E133_DOMAIN_STRING_PADDED_LENGTH];

  // These items will be initialized by the client init functions.
  RDMNET_DECLARE_BUF(RCClientScope, scopes, RDMNET_MAX_SCOPES_PER_CLIENT);
  size_t num_scopes;

  RCLlrpTarget llrp_target;
};

#define RC_RPT_CLIENT_DATA(clientptr) (&(clientptr)->data.rpt)
#define RC_EPT_CLIENT_DATA(clientptr) (&(clientptr)->data.ept)

/*
typedef enum
{
  kClientCallbackNone,
  kClientCallbackConnected,
  kClientCallbackConnectFailed,
  kClientCallbackDisconnected,
  kClientCallbackBrokerMsgReceived,
  kClientCallbackLlrpMsgReceived,
  kClientCallbackMsgReceived
} client_callback_t;

typedef struct ConnectedArgs
{
  rdmnet_client_scope_t scope_handle;
  RdmnetClientConnectedInfo info;
} ConnectedArgs;

typedef struct ConnectFailedArgs
{
  rdmnet_client_scope_t scope_handle;
  RdmnetClientConnectFailedInfo info;
} ConnectFailedArgs;

typedef struct DisconnectedArgs
{
  rdmnet_client_scope_t scope_handle;
  RdmnetClientDisconnectedInfo info;
} DisconnectedArgs;

typedef struct BrokerMsgReceivedArgs
{
  rdmnet_client_scope_t scope_handle;
  const BrokerMessage* msg;
} BrokerMsgReceivedArgs;

typedef struct LlrpMsgReceivedArgs
{
  const LlrpRemoteRdmCommand* cmd;
} LlrpMsgReceivedArgs;

typedef struct RptMsgReceivedArgs
{
  rdmnet_client_scope_t scope_handle;
  RptClientMessage msg;
} RptMsgReceivedArgs;

typedef struct EptMsgReceivedArgs
{
  rdmnet_client_scope_t scope_handle;
  EptClientMessage msg;
} EptMsgReceivedArgs;

typedef struct RptCallbackDispatchInfo
{
  RptClientCallbacks cbs;
  union
  {
    RptMsgReceivedArgs msg_received;
    LlrpMsgReceivedArgs llrp_msg_received;
  } args;
} RptCallbackDispatchInfo;

typedef struct EptCallbackDispatchInfo
{
  EptClientCallbacks cbs;
  EptMsgReceivedArgs msg_received;
} EptCallbackDispatchInfo;

typedef struct ClientCallbackDispatchInfo
{
  rdmnet_client_t handle;
  client_protocol_t type;
  client_callback_t which;
  void* context;
  union
  {
    RptCallbackDispatchInfo rpt;
    EptCallbackDispatchInfo ept;
  } prot_info;
  union
  {
    ConnectedArgs connected;
    ConnectFailedArgs connect_failed;
    DisconnectedArgs disconnected;
    BrokerMsgReceivedArgs broker_msg_received;
  } common_args;
} ClientCallbackDispatchInfo;
*/

/*! An unsolicited RDM response generated by a local component, to be sent over RDMnet. */
typedef struct RdmnetSourceAddr
{
  /*! The endpoint from which this response is being sent. */
  uint16_t source_endpoint;
  /*! The UID of the RDM responder from which this response is being sent. */
  RdmUid rdm_source_uid;
  /*! The sub-device from which this response is being sent, or 0 for the root device. */
  uint16_t subdevice;
} RdmnetSourceAddr;

etcpal_error_t rc_client_init(void);
void rc_client_deinit(void);

etcpal_error_t rc_rpt_client_register(RCClient* client, bool create_llrp_target,
                                      const RdmnetMcastNetintId* llrp_netints, size_t num_llrp_netints);
etcpal_error_t rc_ept_client_register(RCClient* client);
etcpal_error_t rc_client_unregister(RCClient* client, rdmnet_disconnect_reason_t disconnect_reason);
etcpal_error_t rc_client_add_scope(RCClient* client, const RdmnetScopeConfig* scope_config,
                                   rdmnet_client_scope_t* scope_handle);
etcpal_error_t rc_client_remove_scope(RCClient* client, rdmnet_client_scope_t scope_handle,
                                      rdmnet_disconnect_reason_t reason);
etcpal_error_t rc_client_change_scope(RCClient* client, rdmnet_client_scope_t scope_handle,
                                      const RdmnetScopeConfig* new_scope_config,
                                      rdmnet_disconnect_reason_t disconnect_reason);
etcpal_error_t rc_client_get_scope(RCClient* client, rdmnet_client_scope_t scope_handle, char* scope_str_buf,
                                   EtcPalSockAddr* static_broker_addr);
etcpal_error_t rc_client_change_search_domain(RCClient* client, const char* new_search_domain,
                                              rdmnet_disconnect_reason_t reason);
etcpal_error_t rc_client_request_client_list(RCClient* client, rdmnet_client_scope_t scope_handle);
etcpal_error_t rc_client_request_dynamic_uids(RCClient* client, rdmnet_client_scope_t scope_handle,
                                              const EtcPalUuid* responder_ids, size_t num_responders);
etcpal_error_t rc_client_request_responder_ids(RCClient* client, rdmnet_client_scope_t scope_handle, const RdmUid* uids,
                                               size_t num_uids);
etcpal_error_t rc_client_send_rdm_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination,
                                          rdmnet_command_class_t command_class, uint16_t param_id, const uint8_t* data,
                                          uint8_t data_len, uint32_t* seq_num);
etcpal_error_t rc_client_send_get_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination, uint16_t param_id,
                                          const uint8_t* data, uint8_t data_len, uint32_t* seq_num);
etcpal_error_t rc_client_send_set_command(RCClient* client, rdmnet_client_scope_t scope_handle,
                                          const RdmnetDestinationAddr* destination, uint16_t param_id,
                                          const uint8_t* data, uint8_t data_len, uint32_t* seq_num);
etcpal_error_t rc_client_send_rdm_ack(RCClient* client, rdmnet_client_scope_t scope_handle,
                                      const RdmnetSavedRdmCommand* received_cmd, const uint8_t* response_data,
                                      size_t response_data_len);
etcpal_error_t rc_client_send_rdm_nack(RCClient* client, rdmnet_client_scope_t scope_handle,
                                       const RdmnetSavedRdmCommand* received_cmd, rdm_nack_reason_t nack_reason);
etcpal_error_t rc_client_send_rdm_update(RCClient* client, rdmnet_client_scope_t scope_handle,
                                         const RdmnetSourceAddr* source_addr, uint16_t param_id, const uint8_t* data,
                                         size_t data_len);
etcpal_error_t rc_client_send_rpt_status(RCClient* client, rdmnet_client_scope_t scope_handle,
                                         const RdmnetSavedRdmCommand* received_cmd, rpt_status_code_t status_code,
                                         const char* status_string);
etcpal_error_t rc_client_send_llrp_ack(RCClient* client, const LlrpSavedRdmCommand* received_cmd,
                                       const uint8_t* response_data, uint8_t response_data_len);
etcpal_error_t rc_client_send_llrp_nack(RCClient* client, const LlrpSavedRdmCommand* received_cmd,
                                        rdm_nack_reason_t nack_reason);
etcpal_error_t rc_client_send_ept_data(RCClient* client, rdmnet_client_scope_t scope_handle, const EtcPalUuid* dest_cid,
                                       uint16_t manufacturer_id, uint16_t protocol_id, const uint8_t* data,
                                       size_t data_len);
etcpal_error_t rc_client_send_ept_status(RCClient* handle, rdmnet_client_scope_t scope_handle,
                                         const EtcPalUuid* dest_cid, ept_status_code_t status_code,
                                         const char* status_string);

#ifdef __cplusplus
}
#endif

#endif /* RDMNET_CORE_CLIENT_H_ */
