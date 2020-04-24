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

#ifndef RDMNET_CORE_CLIENT_FAKE_CALLBACKS_H
#define RDMNET_CORE_CLIENT_FAKE_CALLBACKS_H

#include "fff.h"
#include "rdmnet/core/client.h"

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_connected, RCClient*, rdmnet_client_scope_t,
                        const RdmnetClientConnectedInfo*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_connect_failed, RCClient*, rdmnet_client_scope_t,
                        const RdmnetClientConnectFailedInfo*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_disconnected, RCClient*, rdmnet_client_scope_t,
                        const RdmnetClientDisconnectedInfo*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_broker_msg_received, RCClient*, rdmnet_client_scope_t,
                        const BrokerMessage*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_llrp_msg_received, RCClient*, const LlrpRdmCommand*,
                        RdmnetSyncRdmResponse*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_rpt_msg_received, RCClient*, rdmnet_client_scope_t,
                        const RptClientMessage*, RdmnetSyncRdmResponse*);
DECLARE_FAKE_VALUE_FUNC(rc_lock_state_t, rc_client_ept_msg_received, RCClient*, rdmnet_client_scope_t,
                        const EptClientMessage*, RdmnetSyncEptResponse*);

void rc_client_callbacks_reset_all_fakes(void);

#ifdef __cplusplus
}
#endif

#endif  // RDMNET_CORE_CLIENT_FAKE_CALLBACKS_H
