# RDMnet C Client API unit tests

rdmnet_add_unit_test(test_rdmnet_core_client
  test_rc_rpt_client_api.cpp
  # test_rpt_client_behavior.cpp

  rdmnet_client_fake_callbacks.h
  rdmnet_client_fake_callbacks.c

  main.cpp

  # Source under test
  ${RDMNET_SRC}/rdmnet/core/client.c

  # Mock dependencies
  ${RDMNET_SRC}/rdmnet_mock/core/common.c
  ${RDMNET_SRC}/rdmnet_mock/core/llrp_target.c
  # ${RDMNET_MOCK_DISCOVERY_SOURCES}

  # Real dependencies
  # ${RDMNET_SRC}/rdmnet/core/broker_prot.c
  # ${RDMNET_SRC}/rdmnet/core/message.c
  # ${RDMNET_SRC}/rdmnet/core/msg_buf.c
  # ${RDMNET_SRC}/rdmnet/core/util.c
)
target_link_libraries(test_rdmnet_core_client PRIVATE EtcPalMock RDM)

