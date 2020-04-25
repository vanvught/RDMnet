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

#include "rdmnet_mock/core/connection.h"

DEFINE_FAKE_VALUE_FUNC(etcpal_error_t, rc_connection_register, RCConnection*);
DEFINE_FAKE_VALUE_FUNC(etcpal_error_t, rc_connection_unregister, RCConnection*, const rdmnet_disconnect_reason_t*);
DEFINE_FAKE_VALUE_FUNC(etcpal_error_t, rc_connection_connect, RCConnection*, const EtcPalSockAddr*,
                       const BrokerClientConnectMsg*);
DEFINE_FAKE_VALUE_FUNC(etcpal_error_t, rc_connection_set_blocking, RCConnection*, bool);
DEFINE_FAKE_VALUE_FUNC(int, rc_connection_send, RCConnection*, const uint8_t*, size_t);

void rdmnet_connection_reset_all_fakes(void)
{
  RESET_FAKE(rc_connection_register);
  RESET_FAKE(rc_connection_unregister);
  RESET_FAKE(rc_connection_connect);
  RESET_FAKE(rc_connection_set_blocking);
  RESET_FAKE(rc_connection_send);
}
