var group__rdmnet__conn =
[
    [ "RdmnetConnectedInfo", "struct_rdmnet_connected_info.html", [
      [ "broker_uid", "struct_rdmnet_connected_info.html#adfda61471109f6b785db3e9070b6711b", null ],
      [ "client_uid", "struct_rdmnet_connected_info.html#ad896ef7a5267263d1ef6cf844d544fb0", null ],
      [ "connected_addr", "struct_rdmnet_connected_info.html#a8e996e3e60b6a0b1b1037658efc9d8e2", null ]
    ] ],
    [ "RdmnetConnectFailedInfo", "struct_rdmnet_connect_failed_info.html", [
      [ "event", "struct_rdmnet_connect_failed_info.html#af39c9f66fed4ca0cfa60d85fe113912c", null ],
      [ "rdmnet_reason", "struct_rdmnet_connect_failed_info.html#a5b18697adc2f956cb17081c8ec41a6b2", null ],
      [ "socket_err", "struct_rdmnet_connect_failed_info.html#a5e6e5b1257eb22d4e3a02a3ed475a817", null ]
    ] ],
    [ "RdmnetDisconnectedInfo", "struct_rdmnet_disconnected_info.html", [
      [ "event", "struct_rdmnet_disconnected_info.html#a12cc294bc1cd6be39934b72d94c96848", null ],
      [ "rdmnet_reason", "struct_rdmnet_disconnected_info.html#a88c5bb705c185425b4b891b99dcfe858", null ],
      [ "socket_err", "struct_rdmnet_disconnected_info.html#a5e6e5b1257eb22d4e3a02a3ed475a817", null ]
    ] ],
    [ "RdmnetConnCallbacks", "struct_rdmnet_conn_callbacks.html", [
      [ "connect_failed", "struct_rdmnet_conn_callbacks.html#acf1ea294240db71094dcd31dbc7cd79b", null ],
      [ "connected", "struct_rdmnet_conn_callbacks.html#aaaaf2b14702631ccda21653f5da2a916", null ],
      [ "disconnected", "struct_rdmnet_conn_callbacks.html#a882ef6a1fc7ea71dd9478d2b7ba790aa", null ],
      [ "msg_received", "struct_rdmnet_conn_callbacks.html#a737a8ea4e3b36b7f71a654ad7bd86344", null ]
    ] ],
    [ "RdmnetConnectionConfig", "struct_rdmnet_connection_config.html", [
      [ "callback_context", "struct_rdmnet_connection_config.html#a67b5953b36108e04f24abe803bc1fe44", null ],
      [ "callbacks", "struct_rdmnet_connection_config.html#a1528e42a4d23637efb81b72019c4cdb4", null ],
      [ "local_cid", "struct_rdmnet_connection_config.html#a170b549c8d603e08b0600f3d24f1647a", null ]
    ] ],
    [ "RDMNET_CONN_INVALID", "group__rdmnet__conn.html#gac00915ff81e5e00b61eabeeba9a7a93e", null ],
    [ "RDMNET_RECV_DATA_MAX_SIZE", "group__rdmnet__conn.html#gad1325d85131a36a798f8f77f87836df2", null ],
    [ "rdmnet_conn_t", "group__rdmnet__conn.html#gab1b61b54140e15d602870403a8e7b08a", null ],
    [ "RdmnetConnCallbacks", "group__rdmnet__conn.html#gad385a675d860a0545465e8be6c00e41f", null ],
    [ "RdmnetConnectedInfo", "group__rdmnet__conn.html#ga10c56affc413d90b3d1271daef39f33e", null ],
    [ "RdmnetConnectFailedInfo", "group__rdmnet__conn.html#gac3b48147fa2e7af1c3c4e9eb4ebbcf97", null ],
    [ "RdmnetConnectionConfig", "group__rdmnet__conn.html#ga57a3727da93a5498ec3207674b2ca769", null ],
    [ "RdmnetDisconnectedInfo", "group__rdmnet__conn.html#ga3445de6d3e8df9b9ee6ce0f0783cfe7d", null ],
    [ "rdmnet_connect_fail_event_t", "group__rdmnet__conn.html#ga496e65ee0af4670795917a7ba2436fd5", [
      [ "kRdmnetConnectFailSocketFailure", "group__rdmnet__conn.html#gga496e65ee0af4670795917a7ba2436fd5ad10a6c8fa911e712c9c3fde14fc29908", null ],
      [ "kRdmnetConnectFailTcpLevel", "group__rdmnet__conn.html#gga496e65ee0af4670795917a7ba2436fd5aff55f14de58c3a3da897f02c9793196b", null ],
      [ "kRdmnetConnectFailNoReply", "group__rdmnet__conn.html#gga496e65ee0af4670795917a7ba2436fd5aa70222d36dc4d526039a25e56ecc1faf", null ],
      [ "kRdmnetConnectFailRejected", "group__rdmnet__conn.html#gga496e65ee0af4670795917a7ba2436fd5a9f6b2bca5751f6e58796d863750bcd8c", null ]
    ] ],
    [ "rdmnet_disconnect_event_t", "group__rdmnet__conn.html#ga5c1cfd964635eb67c610f2007b4a1f3f", [
      [ "kRdmnetDisconnectAbruptClose", "group__rdmnet__conn.html#gga5c1cfd964635eb67c610f2007b4a1f3fac17bea4f8f0f27c0cebb5036b94a0b96", null ],
      [ "kRdmnetDisconnectNoHeartbeat", "group__rdmnet__conn.html#gga5c1cfd964635eb67c610f2007b4a1f3fa001989dd3f1073a64b6ae6e0ba87ba2e", null ],
      [ "kRdmnetDisconnectRedirected", "group__rdmnet__conn.html#gga5c1cfd964635eb67c610f2007b4a1f3fa4fd652506c5bec9d0253abf565fbbf6c", null ],
      [ "kRdmnetDisconnectGracefulRemoteInitiated", "group__rdmnet__conn.html#gga5c1cfd964635eb67c610f2007b4a1f3fad2a8a5dfee982eede7ede276e511f386", null ],
      [ "kRdmnetDisconnectGracefulLocalInitiated", "group__rdmnet__conn.html#gga5c1cfd964635eb67c610f2007b4a1f3fad5bde9e748fdc058de5905e90a1b5413", null ]
    ] ],
    [ "rdmnet_attach_existing_socket", "group__rdmnet__conn.html#ga0e3b184b92dd318baf32f36c0959e0be", null ],
    [ "rdmnet_connect", "group__rdmnet__conn.html#ga42e94e8e6211619c81556bd6d2b95fe0", null ],
    [ "rdmnet_connect_fail_event_to_string", "group__rdmnet__conn.html#ga6113acd9f138cef8466e9cab20e4b086", null ],
    [ "rdmnet_connection_create", "group__rdmnet__conn.html#ga6b8a4f5ceb43524efcb45afecdc0a61f", null ],
    [ "rdmnet_connection_destroy", "group__rdmnet__conn.html#ga267e5e382abe4d683975aa87f817e9a6", null ],
    [ "rdmnet_disconnect_event_to_string", "group__rdmnet__conn.html#ga6b05d48dd09416378ee1711c2cf126ae", null ],
    [ "rdmnet_send", "group__rdmnet__conn.html#ga2818e060195ed67f7654b0bc45847de1", null ],
    [ "rdmnet_set_blocking", "group__rdmnet__conn.html#ga7028a9bd341fd2dc647498f5353786a6", null ]
];