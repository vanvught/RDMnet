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

#ifndef RDMNET_CORE_LLRP_TARGET_H_
#define RDMNET_CORE_LLRP_TARGET_H_

#include "rdmnet/llrp_target.h"
#include "rdmnet/core/common.h"
#include "etcpal/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RCLlrpTarget RCLlrpTarget;

/*
 * An RDM command has been received addressed to an LLRP target.
 * [in] target LLRP target which has received the RDM command.
 * [in] cmd The RDM command data.
 * [out] response Fill in with response data if responding synchronously (see \ref handling_rdm_commands).
 * [in] context Context pointer that was given at the creation of the LLRP target instance.
 */
typedef rc_lock_state_t (*RCLlrpTargetRdmCommandReceivedCallback)(RCLlrpTarget* target, const LlrpRdmCommand* cmd,
                                                                  RdmnetSyncRdmResponse* response);

typedef struct RCLlrpTargetNetintInfo
{
  RdmnetMcastNetintId id;
  etcpal_socket_t send_sock;
  // uint8_t send_buf[LLRP_TARGET_MAX_MESSAGE_SIZE];

  bool reply_pending;
  EtcPalUuid pending_reply_cid;
  uint32_t pending_reply_trans_num;
  EtcPalTimer reply_backoff;
} RCLlrpTargetNetintInfo;

// A struct containing the map keys we use for LLRP targets.
// typedef struct RCLlrpTargetKeys
//{
//  llrp_target_t handle;
//  EtcPalUuid cid;
//} RCLlrpTargetKeys;

typedef struct RCLlrpTarget
{
  // Identifying info
  EtcPalUuid cid;
  RdmUid uid;
  llrp_component_t component_type;
  RCLlrpTargetRdmCommandReceivedCallback rdm_cmd_received_cb;
  etcpal_mutex_t* lock;

  RDMNET_DECLARE_BUF(RCLlrpTargetNetintInfo, netints, RDMNET_MAX_MCAST_NETINTS);

  // Global target state info
  bool connected_to_broker;

  // Synchronized destruction tracking
  // bool marked_for_destruction;
  // RCLlrpTarget* next_to_destroy;
} RCLlrpTarget;

/*
typedef enum
{
  kTargetCallbackNone,
  kTargetCallbackRdmCmdReceived
} target_callback_t;

typedef struct RdmCmdReceivedArgs
{
  LlrpRdmCommand cmd;
} RdmCmdReceivedArgs;

typedef struct TargetCallbackDispatchInfo
{
  llrp_target_t handle;
  RCLlrpTargetCallbacks cbs;
  void* context;

  target_callback_t which;
  union
  {
    RdmCmdReceivedArgs cmd_received;
  } args;
} TargetCallbackDispatchInfo;
*/

etcpal_error_t rc_llrp_target_init(void);
void rc_llrp_target_deinit(void);
void rc_llrp_target_tick(void);

etcpal_error_t rc_llrp_target_register(RCLlrpTarget* target, const RdmnetMcastNetintId* netints, size_t num_netints);
void rc_llrp_target_unregister(RCLlrpTarget* target);
void rc_llrp_target_update_connection_state(RCLlrpTarget* target, bool connected_to_broker);

etcpal_error_t rc_llrp_target_send_ack(RCLlrpTarget* target, const LlrpSavedRdmCommand* received_cmd,
                                       const uint8_t* response_data, uint8_t response_data_len);
etcpal_error_t rc_llrp_target_send_nack(RCLlrpTarget* target, const LlrpSavedRdmCommand* received_cmd,
                                        rdm_nack_reason_t nack_reason);

void target_data_received(const uint8_t* data, size_t data_size, const RdmnetMcastNetintId* netint);

#ifdef __cplusplus
}
#endif

#endif /* RDMNET_PRIVATE_LLRP_TARGET_H_ */
