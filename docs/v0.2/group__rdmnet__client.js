var group__rdmnet__client =
[
    [ "Controller API", "group__rdmnet__controller.html", "group__rdmnet__controller" ],
    [ "Device API", "group__rdmnet__device.html", "group__rdmnet__device" ],
    [ "RdmnetClientConnectedInfo", "struct_rdmnet_client_connected_info.html", [
      [ "broker_addr", "struct_rdmnet_client_connected_info.html#a3c19df47bab872ffc7c6522715129f4c", null ]
    ] ],
    [ "RdmnetClientConnectFailedInfo", "struct_rdmnet_client_connect_failed_info.html", [
      [ "event", "struct_rdmnet_client_connect_failed_info.html#af39c9f66fed4ca0cfa60d85fe113912c", null ],
      [ "rdmnet_reason", "struct_rdmnet_client_connect_failed_info.html#a5b18697adc2f956cb17081c8ec41a6b2", null ],
      [ "socket_err", "struct_rdmnet_client_connect_failed_info.html#adb8957de115741fc9cf1bb26d61d68c4", null ],
      [ "will_retry", "struct_rdmnet_client_connect_failed_info.html#ac5e79a77b6b7fa79f438ae3b3b9dc824", null ]
    ] ],
    [ "RdmnetClientDisconnectedInfo", "struct_rdmnet_client_disconnected_info.html", [
      [ "event", "struct_rdmnet_client_disconnected_info.html#a12cc294bc1cd6be39934b72d94c96848", null ],
      [ "rdmnet_reason", "struct_rdmnet_client_disconnected_info.html#a88c5bb705c185425b4b891b99dcfe858", null ],
      [ "socket_err", "struct_rdmnet_client_disconnected_info.html#adb8957de115741fc9cf1bb26d61d68c4", null ],
      [ "will_retry", "struct_rdmnet_client_disconnected_info.html#ac5e79a77b6b7fa79f438ae3b3b9dc824", null ]
    ] ],
    [ "RptClientCallbacks", "struct_rpt_client_callbacks.html", [
      [ "broker_msg_received", "struct_rpt_client_callbacks.html#a059edcc99f5f028292255d04692167b6", null ],
      [ "connect_failed", "struct_rpt_client_callbacks.html#a38ca676310c9d0a421e83b92ea1200dc", null ],
      [ "connected", "struct_rpt_client_callbacks.html#a898dae56f1d15fb7a0180231de576a1a", null ],
      [ "disconnected", "struct_rpt_client_callbacks.html#ac36e579bb6a8f9341eff40df45d51439", null ],
      [ "llrp_msg_received", "struct_rpt_client_callbacks.html#a5267178c7fc9be0eb0de322b8f1c1e6f", null ],
      [ "msg_received", "struct_rpt_client_callbacks.html#a23f1eaf1e1498feb110c4fa64a0e925f", null ]
    ] ],
    [ "EptClientCallbacks", "struct_ept_client_callbacks.html", [
      [ "broker_msg_received", "struct_ept_client_callbacks.html#a059edcc99f5f028292255d04692167b6", null ],
      [ "connect_failed", "struct_ept_client_callbacks.html#a38ca676310c9d0a421e83b92ea1200dc", null ],
      [ "connected", "struct_ept_client_callbacks.html#a898dae56f1d15fb7a0180231de576a1a", null ],
      [ "disconnected", "struct_ept_client_callbacks.html#ac36e579bb6a8f9341eff40df45d51439", null ],
      [ "msg_received", "struct_ept_client_callbacks.html#a1ab270a223576abf577b79bb2ad4af68", null ]
    ] ],
    [ "RdmnetScopeConfig", "struct_rdmnet_scope_config.html", [
      [ "has_static_broker_addr", "struct_rdmnet_scope_config.html#af3456bfb67a114a3b734a8b4b667e3e0", null ],
      [ "scope", "struct_rdmnet_scope_config.html#a2fd03e13c2aaced4f72d8b5a3e2599f8", null ],
      [ "static_broker_addr", "struct_rdmnet_scope_config.html#ac72dd659d9cc3b5a14528ac5860ef245", null ]
    ] ],
    [ "RptClientOptionalConfig", "struct_rpt_client_optional_config.html", [
      [ "search_domain", "struct_rpt_client_optional_config.html#aa7f5a8d120934e4d1fb7ba53b0489c62", null ],
      [ "uid", "struct_rpt_client_optional_config.html#ad908bad751766196cc2334b0f8579a5c", null ]
    ] ],
    [ "RdmnetRptClientConfig", "struct_rdmnet_rpt_client_config.html", [
      [ "callback_context", "struct_rdmnet_rpt_client_config.html#a67b5953b36108e04f24abe803bc1fe44", null ],
      [ "callbacks", "struct_rdmnet_rpt_client_config.html#a70cfc205d865a5d4647a660d7a9f8fd8", null ],
      [ "cid", "struct_rdmnet_rpt_client_config.html#a032cc7001d59441d52e16a3604dc3f2d", null ],
      [ "llrp_optional", "struct_rdmnet_rpt_client_config.html#ab68a231a46affce33a48b92c9070ab4a", null ],
      [ "optional", "struct_rdmnet_rpt_client_config.html#abc7c5b60db0c268dda0820cc8b8dc781", null ],
      [ "type", "struct_rdmnet_rpt_client_config.html#a99ea5a63f1a82d7352ad21fc593a21fc", null ]
    ] ],
    [ "RdmnetEptClientConfig", "struct_rdmnet_ept_client_config.html", [
      [ "callback_context", "struct_rdmnet_ept_client_config.html#a67b5953b36108e04f24abe803bc1fe44", null ],
      [ "callbacks", "struct_rdmnet_ept_client_config.html#a507a5e613fc45fa3140f638d71b6f485", null ],
      [ "num_protocols", "struct_rdmnet_ept_client_config.html#ac4c5442623e5438a686daf3f26345968", null ],
      [ "protocol_list", "struct_rdmnet_ept_client_config.html#ae14c626c908736b1f16984ebe3bf2fad", null ]
    ] ],
    [ "RDMNET_CLIENT_INVALID", "group__rdmnet__client.html#ga70b2a4d8e736be8de1935de552c10d9f", null ],
    [ "RDMNET_CLIENT_SCOPE_INVALID", "group__rdmnet__client.html#gad592a80648a3e35675ff9820831ec85f", null ],
    [ "RDMNET_CLIENT_SET_DEFAULT_SCOPE", "group__rdmnet__client.html#ga8fb92dc67b794d5e54a37ca84331e207", null ],
    [ "RDMNET_CLIENT_SET_SCOPE", "group__rdmnet__client.html#ga0b906d1d6ed5fcc678e9c530e440b62f", null ],
    [ "RDMNET_CLIENT_SET_STATIC_DEFAULT_SCOPE", "group__rdmnet__client.html#gab2855ff14aba079140f1070f80140247", null ],
    [ "RDMNET_CLIENT_SET_STATIC_SCOPE", "group__rdmnet__client.html#ga500c0af02a2db3a6b835f26cf304da85", null ],
    [ "RPT_CLIENT_CONFIG_INIT", "group__rdmnet__client.html#ga69f10c1a6dc9134ba78f31037c9c4e9e", null ],
    [ "EptClientCallbacks", "group__rdmnet__client.html#gad3bc6586af79a9f38bf8e1fc125d6169", null ],
    [ "EptClientMsgReceivedCb", "group__rdmnet__client.html#gace1d101ac9ae5fce64e64606893faf60", null ],
    [ "rdmnet_client_scope_t", "group__rdmnet__client.html#ga1da9c25a97d47dbccefdb05f715cab08", null ],
    [ "rdmnet_client_t", "group__rdmnet__client.html#ga790fd1f0905e05170777cb23216d94c6", null ],
    [ "RdmnetClientBrokerMsgReceivedCb", "group__rdmnet__client.html#ga07e27d1ccd8cfc0e374a0b1522a82531", null ],
    [ "RdmnetClientConnectedCb", "group__rdmnet__client.html#gabf316768e235310a187ed918f5e5cc99", null ],
    [ "RdmnetClientConnectedInfo", "group__rdmnet__client.html#gaaf1124635d6b5b294eb793b30fa98c31", null ],
    [ "RdmnetClientConnectFailedCb", "group__rdmnet__client.html#ga069e073fe6ea323646e3c7a43ee4eab6", null ],
    [ "RdmnetClientConnectFailedInfo", "group__rdmnet__client.html#ga57b88373159e706810140dd07afa45fd", null ],
    [ "RdmnetClientDisconnectedCb", "group__rdmnet__client.html#gac41f1df7a75bc79494b226003cdd0a05", null ],
    [ "RdmnetClientDisconnectedInfo", "group__rdmnet__client.html#ga4b4913720598f709c36d429277ee33a8", null ],
    [ "RdmnetClientLlrpMsgReceivedCb", "group__rdmnet__client.html#ga60cf8ac60ec5474e32a129028dfb41d1", null ],
    [ "RdmnetRptClientConfig", "group__rdmnet__client.html#gaadc36f5b0a0d2c69bddc396808d9cda0", null ],
    [ "RdmnetScopeConfig", "group__rdmnet__client.html#gab890eff22b533de22305dd0df209889b", null ],
    [ "RptClientCallbacks", "group__rdmnet__client.html#gac64d1e566c0b5c74a10034ea1ccc72dc", null ],
    [ "RptClientMsgReceivedCb", "group__rdmnet__client.html#gacb45912393f9a958de1478b7904d2d3a", null ],
    [ "RptClientOptionalConfig", "group__rdmnet__client.html#ga65ade56a5c762b00d012390cb60f519f", null ]
];