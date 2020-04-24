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

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "rdmnet/core/client.h"

#include "rdmnet_mock/core/common.h"
#include "rdmnet_mock/core/llrp_target.h"
#include "rdmnet_client_fake_callbacks.h"

class TestRCRptClientApi : public testing::Test
{
protected:
  RdmnetScopeConfig default_dynamic_scope_{};
  RCClient client_{};
  etcpal_mutex_t fake_client_lock_;

  TestRCRptClientApi()
  {
    RDMNET_CLIENT_SET_DEFAULT_SCOPE(&default_dynamic_scope_);

    client_.lock = &fake_client_lock_;
    client_.type = kClientProtocolRPT;
    client_.cid = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};
    RC_RPT_CLIENT_DATA(&client_)->type = kRPTClientTypeController;
    RC_RPT_CLIENT_DATA(&client_)->callbacks.connected = rc_client_connected;
    RC_RPT_CLIENT_DATA(&client_)->callbacks.disconnected = rc_client_disconnected;
    RC_RPT_CLIENT_DATA(&client_)->callbacks.broker_msg_received = rc_client_broker_msg_received;
    RC_RPT_CLIENT_DATA(&client_)->callbacks.llrp_msg_received = rc_client_llrp_msg_received;
    RC_RPT_CLIENT_DATA(&client_)->callbacks.rpt_msg_received = rc_client_rpt_msg_received;
    client_.search_domain[0] = '\0';
  }

  void SetUp() override
  {
    // Reset the fakes
    rc_client_callbacks_reset_all_fakes();
    rc_llrp_target_reset_all_fakes();
    rdmnet_mock_core_reset_and_init();

    // Init
    ASSERT_EQ(kEtcPalErrOk, rc_client_init());
  }

  void TearDown() override { rc_client_deinit(); }
};

// Test the rdmnet_rpt_client_create() function in valid and invalid scenarios
// TEST_F(TestRCRptClientApi, ClientCreateInvalidCallsFail)
//{
//  rdmnet_client_t handle;
//
//  // Invalid arguments
//  EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_create(NULL, NULL));
//  EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_create(&default_rpt_config_, NULL));
//  EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_create(NULL, &handle));
//
//  // Valid config, but core is not initialized
//  rdmnet_core_initialized_fake.return_val = false;
//  EXPECT_EQ(kEtcPalErrNotInit, rdmnet_rpt_client_create(&default_rpt_config_, &handle));
//}

// Test that basic registration of an RDMnet client works
TEST_F(TestRCRptClientApi, ClientRegisterWorks)
{
  EXPECT_EQ(kEtcPalErrOk, rc_rpt_client_register(&client_, false, nullptr, 0));
}

// Test that the client registers an associated LLRP target when requested to do so.
TEST_F(TestRCRptClientApi, ClientCreateLlrpTargetWorks)
{
  RdmnetMcastNetintId llrp_netint = {kEtcPalIpTypeV4, 1};
  ASSERT_EQ(kEtcPalErrOk, rc_rpt_client_register(&client_, true, &llrp_netint, 1));

  EXPECT_EQ(rc_llrp_target_register_fake.call_count, 1u);
  EXPECT_EQ(rc_llrp_target_register_fake.arg0_val, &client_.llrp_target);
  EXPECT_EQ(rc_llrp_target_register_fake.arg1_val, &llrp_netint);
  EXPECT_EQ(rc_llrp_target_register_fake.arg2_val, 1u);

  EXPECT_EQ(client_.cid, client_.llrp_target.cid);
  EXPECT_EQ(RC_RPT_CLIENT_DATA(&client_)->uid, client_.llrp_target.uid);
  EXPECT_EQ(client_.llrp_target.component_type, kLlrpCompRptController);
}

TEST_F(TestRCRptClientApi, ClientAddScopeWorks)
{
  EXPECT_EQ(kEtcPalErrOk, rc_rpt_client_register(&client_, false, nullptr, 0));
}

// TEST_F(TestRCRptClientApi, ClientAddMultipleScopesWorks)
// {
//   ASSERT_EQ(kEtcPalErrOk, rc_rpt_client_register(handle_1, &default_dynamic_scope_, &scope_handle));
//
//   // Valid create with 100 different scopes
//   rdmnet_client_t handle_2;
//   ASSERT_EQ(kEtcPalErrOk, rdmnet_rpt_client_create(&default_rpt_config_, &handle_2));
//
//   for (size_t i = 0; i < 100; ++i)
//   {
//     std::string scope_str = E133_DEFAULT_SCOPE + std::to_string(i);
//     RdmnetScopeConfig tmp_scope;
//     RDMNET_CLIENT_SET_SCOPE(&tmp_scope, scope_str.c_str());
//     rdmnet_client_scope_t tmp_handle;
//     ASSERT_EQ(kEtcPalErrOk, rdmnet_client_add_scope(handle_2, &tmp_scope, &tmp_handle));
//   }
// }

// TEST_F(TestRCRptClientApi, SendRdmCommandInvalidCallsFail)
//{
//  rdmnet_client_t handle;
//  rdmnet_client_scope_t scope_handle;
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_rpt_client_create(&default_rpt_config_, &handle));
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_client_add_scope(handle, &default_dynamic_scope_, &scope_handle));
//
//  RdmnetLocalRdmCommand cmd;
//  uint32_t seq_num = 0;
//
//  // Core not initialized
//  rdmnet_core_initialized_fake.return_val = false;
//  EXPECT_EQ(kEtcPalErrNotInit, rdmnet_rpt_client_send_rdm_command(handle, scope_handle, &cmd, &seq_num));
//
//  // Invalid parameters
//  rdmnet_core_initialized_fake.return_val = true;
//  EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_command(RDMNET_CLIENT_INVALID, scope_handle, &cmd,
//  &seq_num)); EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_command(handle, RDMNET_CLIENT_SCOPE_INVALID,
//  &cmd, &seq_num)); EXPECT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_command(handle, scope_handle, NULL,
//  &seq_num));
//}
//
// TEST_F(TestRCRptClientApi, SendRdmResponseInvalidCallsFail)
//{
//  rdmnet_client_t handle;
//  rdmnet_client_scope_t scope_handle;
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_rpt_client_create(&default_rpt_config_, &handle));
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_client_add_scope(handle, &default_dynamic_scope_, &scope_handle));
//
//  RdmnetLocalRdmResponse resp;
//
//  // Core not initialized
//  rdmnet_core_initialized_fake.return_val = false;
//  ASSERT_EQ(kEtcPalErrNotInit, rdmnet_rpt_client_send_rdm_response(handle, scope_handle, &resp));
//
//  // Invalid parameters
//  rdmnet_core_initialized_fake.return_val = true;
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_response(RDMNET_CLIENT_INVALID, scope_handle, &resp));
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_response(handle, RDMNET_CLIENT_SCOPE_INVALID, &resp));
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_rdm_response(handle, scope_handle, NULL));
//}
//
// TEST_F(TestRCRptClientApi, SendStatusInvalidCallsFail)
//{
//  rdmnet_client_t handle;
//  rdmnet_client_scope_t scope_handle;
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_rpt_client_create(&default_rpt_config_, &handle));
//  ASSERT_EQ(kEtcPalErrOk, rdmnet_client_add_scope(handle, &default_dynamic_scope_, &scope_handle));
//
//  LocalRptStatus status;
//
//  // Core not initialized
//  rdmnet_core_initialized_fake.return_val = false;
//  ASSERT_EQ(kEtcPalErrNotInit, rdmnet_rpt_client_send_status(handle, scope_handle, &status));
//
//  // Invalid parameters
//  rdmnet_core_initialized_fake.return_val = true;
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_status(RDMNET_CLIENT_INVALID, scope_handle, &status));
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_status(handle, RDMNET_CLIENT_SCOPE_INVALID, &status));
//  ASSERT_EQ(kEtcPalErrInvalid, rdmnet_rpt_client_send_status(handle, scope_handle, NULL));
//}
