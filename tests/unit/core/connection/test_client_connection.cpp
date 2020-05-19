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

#include <array>
#include "etcpal/common.h"
#include "etcpal/cpp/uuid.h"
#include "etcpal/cpp/inet.h"
#include "etcpal_mock/socket.h"
#include "etcpal_mock/timer.h"
#include "rdmnet/core/connection.h"
#include "rdmnet_mock/core/common.h"
#include "gtest/gtest.h"

extern "C" {
FAKE_VOID_FUNC(conncb_connected, RCClientConnection*, const RCConnectedInfo*);
FAKE_VOID_FUNC(conncb_connect_failed, RCClientConnection*, const RCConnectFailedInfo*);
FAKE_VOID_FUNC(conncb_disconnected, RCClientConnection*, const RCDisconnectedInfo*);
FAKE_VOID_FUNC(conncb_msg_received, RCClientConnection*, const RdmnetMessage*);
FAKE_VOID_FUNC(conncb_destroyed, RCClientConnection*);
}

class TestClientConnection : public testing::Test
{
protected:
  RCClientConnection conn_;

  void SetUp() override
  {
    RESET_FAKE(conncb_connected);
    RESET_FAKE(conncb_connect_failed);
    RESET_FAKE(conncb_disconnected);
    RESET_FAKE(conncb_msg_received);
    RESET_FAKE(conncb_destroyed);

    rdmnet_mock_core_reset_and_init();

    etcpal_socket_reset_all_fakes();
    etcpal_timer_reset_all_fakes();
    etcpal_socket_fake.return_val = kEtcPalErrOk;
    etcpal_setblocking_fake.return_val = kEtcPalErrOk;
    etcpal_connect_fake.return_val = kEtcPalErrInProgress;

    etcpal_poll_add_socket_fake.return_val = kEtcPalErrOk;
    etcpal_poll_wait_fake.return_val = kEtcPalErrTimedOut;

    ASSERT_EQ(kEtcPalErrOk, rc_conn_module_init());
    ASSERT_EQ(kEtcPalErrOk, rc_client_conn_register(&conn_));
  }

  void TearDown() override
  {
    rc_client_conn_unregister(&conn_, nullptr);
    rc_conn_module_deinit();
  }

  void PassTimeAndTick()
  {
    etcpal_getms_fake.return_val += 1000;
    rc_conn_module_tick();
  }
};

// class TestClientConnectionAlreadyConnected : public TestClientConnection
//{
// protected:
//  void SetUp() override
//  {
//    TestClientConnection::SetUp();
//
//    // This allows us to skip the connection process and go straight to a connected state.
//    etcpal_socket_t fake_socket = 0;
//    EtcPalSockAddr remote_addr{};
//    ASSERT_EQ(kEtcPalErrOk, rdmnet_attach_existing_socket(conn_, fake_socket, &remote_addr));
//  }
//};
//
//// Need to test the value of disconn_info inside a custom fake, because only the pointer is saved
//// (which is invalid after the function returns)
// extern "C" void conncb_socket_error(rdmnet_conn_t, const RdmnetDisconnectedInfo* disconn_info, void* context)
//{
//  ETCPAL_UNUSED_ARG(context);
//  EXPECT_EQ(disconn_info->socket_err, kEtcPalErrConnReset);
//  EXPECT_EQ(disconn_info->event, kRdmnetDisconnectAbruptClose);
//}
//
// TEST_F(TestClientConnectionAlreadyConnected, DisconnectsOnSocketError)
//{
//  conncb_disconnected_fake.custom_fake = conncb_socket_error;
//
//  // Simulate an error on a socket, make sure it is marked disconnected.
//  rdmnet_socket_error(conn_, kEtcPalErrConnReset);
//
//  ASSERT_EQ(conncb_disconnected_fake.call_count, 1u);
//  ASSERT_EQ(conncb_disconnected_fake.arg0_val, conn_);
//}

TEST_F(TestClientConnection, HandlesSocketErrorOnConnect)
{
  auto local_cid = etcpal::Uuid::FromString("51077344-7164-487e-88c1-b3146de32d4c");
  etcpal::SockAddr remote_addr(etcpal::IpAddr::FromString("10.101.1.1"), 8888);
  BrokerClientConnectMsg connect_msg{};

  ASSERT_EQ(kEtcPalErrOk, rc_client_conn_connect(&conn_, &local_cid.get(), &remote_addr.get(), &connect_msg));

  // TODO etcpal_poll stub to return socket error
  PassTimeAndTick();
  // rdmnet_socket_error(conn_, kEtcPalErrConnRefused);

  EXPECT_EQ(conncb_connect_failed_fake.call_count, 1u);
}

TEST_F(TestClientConnection, SetsCorrectSocketOptionsIpv4)
{
  auto local_cid = etcpal::Uuid::FromString("51077344-7164-487e-88c1-b3146de32d4c");
  etcpal::SockAddr remote_addr(etcpal::IpAddr::FromString("10.101.1.1"), 8888);
  BrokerClientConnectMsg connect_msg{};

  ASSERT_EQ(kEtcPalErrOk, rc_client_conn_connect(&conn_, &local_cid.get(), &remote_addr.get(), &connect_msg));
  PassTimeAndTick();

  EXPECT_EQ(etcpal_socket_fake.call_count, 1u);
  EXPECT_EQ(etcpal_socket_fake.arg0_val, ETCPAL_AF_INET);
  EXPECT_EQ(etcpal_socket_fake.arg1_val, ETCPAL_STREAM);

  EXPECT_EQ(etcpal_setblocking_fake.call_count, 1u);
  EXPECT_EQ(etcpal_setblocking_fake.arg1_val, false);

  EXPECT_EQ(etcpal_connect_fake.call_count, 1u);
}

TEST_F(TestClientConnection, SetsCorrectSocketOptionsIpv6)
{
  auto local_cid = etcpal::Uuid::FromString("51077344-7164-487e-88c1-b3146de32d4c");
  etcpal::SockAddr remote_addr(etcpal::IpAddr::FromString("2001:db8::1234:5678"), 8888);
  BrokerClientConnectMsg connect_msg{};

  ASSERT_EQ(kEtcPalErrOk, rc_client_conn_connect(&conn_, &local_cid.get(), &remote_addr.get(), &connect_msg));
  PassTimeAndTick();

  EXPECT_EQ(etcpal_socket_fake.call_count, 1u);
  EXPECT_EQ(etcpal_socket_fake.arg0_val, ETCPAL_AF_INET6);
  EXPECT_EQ(etcpal_socket_fake.arg1_val, ETCPAL_STREAM);

  EXPECT_EQ(etcpal_setblocking_fake.call_count, 1u);
  EXPECT_EQ(etcpal_setblocking_fake.arg1_val, false);

  EXPECT_EQ(etcpal_connect_fake.call_count, 1u);
}
