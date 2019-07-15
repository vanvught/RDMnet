/******************************************************************************
************************* IMPORTANT NOTE -- READ ME!!! ************************
*******************************************************************************
* THIS SOFTWARE IMPLEMENTS A **DRAFT** STANDARD, BSR E1.33 REV. 77. UNDER NO
* CIRCUMSTANCES SHOULD THIS SOFTWARE BE USED FOR ANY PRODUCT AVAILABLE FOR
* GENERAL SALE TO THE PUBLIC. DUE TO THE INEVITABLE CHANGE OF DRAFT PROTOCOL
* VALUES AND BEHAVIORAL REQUIREMENTS, PRODUCTS USING THIS SOFTWARE WILL **NOT**
* BE INTEROPERABLE WITH PRODUCTS IMPLEMENTING THE FINAL RATIFIED STANDARD.
*******************************************************************************
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
*******************************************************************************
* This file is a part of RDMnet. For more information, go to:
* https://github.com/ETCLabs/RDMnet
******************************************************************************/

#include "rdmnet_mock/core.h"
#include "rdmnet_mock/private/core.h"

#include "rdmnet_mock/core/broker_prot.h"
#include "rdmnet_mock/core/connection.h"
#include "rdmnet_mock/core/discovery.h"
#include "rdmnet_mock/core/rpt_prot.h"
#include "rdmnet_mock/core/llrp_target.h"

// public mocks
DEFINE_FAKE_VALUE_FUNC(lwpa_error_t, rdmnet_core_init, const LwpaLogParams*);
DEFINE_FAKE_VOID_FUNC(rdmnet_core_deinit);
DEFINE_FAKE_VOID_FUNC(rdmnet_core_tick);
DEFINE_FAKE_VALUE_FUNC(bool, rdmnet_core_initialized);

DEFINE_FAKE_VALUE_FUNC(bool, rdmnet_readlock);
DEFINE_FAKE_VOID_FUNC(rdmnet_readunlock);
DEFINE_FAKE_VALUE_FUNC(bool, rdmnet_writelock);
DEFINE_FAKE_VOID_FUNC(rdmnet_writeunlock);

DEFINE_FAKE_VALUE_FUNC(lwpa_error_t, rdmnet_core_add_polled_socket, lwpa_socket_t, lwpa_poll_events_t,
                       PolledSocketInfo*);
DEFINE_FAKE_VALUE_FUNC(lwpa_error_t, rdmnet_core_modify_polled_socket, lwpa_socket_t, lwpa_poll_events_t,
                       PolledSocketInfo*);
DEFINE_FAKE_VOID_FUNC(rdmnet_core_remove_polled_socket, lwpa_socket_t);

const LwpaLogParams* rdmnet_log_params = NULL;

void rdmnet_mock_core_reset_and_init()
{
  RESET_FAKE(rdmnet_core_init);
  RESET_FAKE(rdmnet_core_deinit);
  RESET_FAKE(rdmnet_core_tick);
  RESET_FAKE(rdmnet_core_initialized);

  RESET_FAKE(rdmnet_readlock);
  RESET_FAKE(rdmnet_readunlock);
  RESET_FAKE(rdmnet_writelock);
  RESET_FAKE(rdmnet_writeunlock);

  RESET_FAKE(rdmnet_core_add_polled_socket);
  RESET_FAKE(rdmnet_core_modify_polled_socket);
  RESET_FAKE(rdmnet_core_remove_polled_socket);

#if RDMNET_BUILDING_FULL_MOCK_CORE_LIB
  RDMNET_CORE_BROKER_PROT_DO_FOR_ALL_FAKES(RESET_FAKE);
  RDMNET_CORE_CONNECTION_DO_FOR_ALL_FAKES(RESET_FAKE);
  RDMNET_CORE_DISCOVERY_DO_FOR_ALL_FAKES(RESET_FAKE);
  RDMNET_CORE_LLRP_TARGET_DO_FOR_ALL_FAKES(RESET_FAKE);
  RDMNET_CORE_RPT_PROT_DO_FOR_ALL_FAKES(RESET_FAKE);
#endif

  rdmnet_readlock_fake.return_val = true;
  rdmnet_writelock_fake.return_val = true;
  rdmnet_core_initialized_fake.return_val = true;
}
