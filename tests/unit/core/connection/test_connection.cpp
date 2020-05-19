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

#include "gtest/gtest.h"
#include "etcpal_mock/common.h"
#include "rdmnet/core/connection.h"
#include "rdmnet_mock/core/common.h"

class TestConnection : public testing::Test
{
protected:
  RCConnection conn_;

  void SetUp() override
  {
    etcpal_reset_all_fakes();
    rdmnet_mock_core_reset_and_init();
    ASSERT_EQ(kEtcPalErrOk, rc_conn_module_init());
  }

  void TearDown() override { rc_conn_module_deinit(); }
};

TEST_F(TestConnection, InitWorks)
{
  rc_conn_init(&conn_);
  EXPECT_EQ(conn_.hb_timer.interval, static_cast<uint32_t>(E133_HEARTBEAT_TIMEOUT_SEC));
  EXPECT_EQ(conn_.send_timer.interval, static_cast<uint32_t>(E133_TCP_HEARTBEAT_INTERVAL_SEC));
}
