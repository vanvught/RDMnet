var group__rdmnet__message =
[
    [ "ClientConnectMsg", "struct_client_connect_msg.html", [
      [ "client_entry", "struct_client_connect_msg.html#a9ebc75b50789fc8c43389bc5f7780401", null ],
      [ "connect_flags", "struct_client_connect_msg.html#a16548771900c3e7c13b759795547ea00", null ],
      [ "e133_version", "struct_client_connect_msg.html#a6c9362c7773ff18ee23c0b1d0425df7a", null ],
      [ "scope", "struct_client_connect_msg.html#a2fd03e13c2aaced4f72d8b5a3e2599f8", null ],
      [ "search_domain", "struct_client_connect_msg.html#a3ce1f6ba8035a40d9afbb98da41d1e12", null ]
    ] ],
    [ "ConnectReplyMsg", "struct_connect_reply_msg.html", [
      [ "broker_uid", "struct_connect_reply_msg.html#a8288b529331b851671c76479c1b0d958", null ],
      [ "connect_status", "struct_connect_reply_msg.html#a45878cd8ae9c841014e11e941861ee15", null ],
      [ "e133_version", "struct_connect_reply_msg.html#a6c9362c7773ff18ee23c0b1d0425df7a", null ]
    ] ],
    [ "ClientEntryUpdateMsg", "struct_client_entry_update_msg.html", [
      [ "client_entry", "struct_client_entry_update_msg.html#a9ebc75b50789fc8c43389bc5f7780401", null ],
      [ "connect_flags", "struct_client_entry_update_msg.html#a16548771900c3e7c13b759795547ea00", null ]
    ] ],
    [ "ClientRedirectMsg", "struct_client_redirect_msg.html", [
      [ "new_addr", "struct_client_redirect_msg.html#a2c09aef6c20961dca743d8d3d3086379", null ]
    ] ],
    [ "ClientList", "struct_client_list.html", [
      [ "client_entry_list", "struct_client_list.html#a8193431a5eec23f3cbcfd56f3b82c864", null ],
      [ "partial", "struct_client_list.html#a450e8f772f5ba536489f6cdd3a0954af", null ]
    ] ],
    [ "DisconnectMsg", "struct_disconnect_msg.html", [
      [ "disconnect_reason", "struct_disconnect_msg.html#a7f4221993f5ad0485f02968bf097e246", null ]
    ] ],
    [ "BrokerMessage", "struct_broker_message.html", [
      [ "client_connect", "struct_broker_message.html#add68de3751c6888973bdca9ac942ab81", null ],
      [ "client_entry_update", "struct_broker_message.html#a0711010efd79ffd8da27b3285c615269", null ],
      [ "client_list", "struct_broker_message.html#adf541bfdb888b1638dfdabe4eff185fc", null ],
      [ "client_redirect", "struct_broker_message.html#a01ceff0c068ba783c519035f52575cdd", null ],
      [ "connect_reply", "struct_broker_message.html#ab53cb13ad7354be0e61d05101dad43b6", null ],
      [ "data", "struct_broker_message.html#abf6e6d57496876476faf6ce340f5a243", null ],
      [ "disconnect", "struct_broker_message.html#a1f5c6a09b1b7e6bfae9496b9d36d6d5f", null ],
      [ "vector", "struct_broker_message.html#a631ad08149d041e8b8330da035d54ade", null ]
    ] ],
    [ "RdmnetMessage", "struct_rdmnet_message.html", [
      [ "broker", "struct_rdmnet_message.html#a54302c30c5004baad9cef89ca9f946e3", null ],
      [ "data", "struct_rdmnet_message.html#a31c56d4326b56cf9d63d072c093992cb", null ],
      [ "ept", "struct_rdmnet_message.html#ad71556bc99ba5fd8b8b384e48a34d911", null ],
      [ "rpt", "struct_rdmnet_message.html#acfe970667ad0b3d46b792ac29d319d14", null ],
      [ "sender_cid", "struct_rdmnet_message.html#aa637edf35bbbdf0fceb946c870aca115", null ],
      [ "vector", "struct_rdmnet_message.html#a513d751c7097b43b3968ac81f3a5715b", null ]
    ] ],
    [ "CONNECTFLAG_INCREMENTAL_UPDATES", "group__rdmnet__message.html#ga21824dc15b7fe1e55a19cfc73081b4de", null ],
    [ "get_broker_msg", "group__rdmnet__message.html#gaa5d26affd96e54bd2bb77eb5fe069bba", null ],
    [ "get_ept_msg", "group__rdmnet__message.html#ga6d7db520b1c023c2a7ff29d244df110d", null ],
    [ "get_rpt_msg", "group__rdmnet__message.html#ga659ec09c5a216dc99cf09e91a5373cc7", null ],
    [ "is_broker_msg", "group__rdmnet__message.html#gadb35a3f4a44cd089e43c3ae17984edce", null ],
    [ "is_ept_msg", "group__rdmnet__message.html#ga6fead3eaa9e856ffac42ceacfb6c53eb", null ],
    [ "is_rpt_msg", "group__rdmnet__message.html#ga4914f526753ca471b2304932f08b5904", null ],
    [ "RdmnetMessage", "group__rdmnet__message.html#gaefbdd2f0f04bb6c4c247b9e1d0128ec1", null ],
    [ "rdmnet_connect_status_t", "group__rdmnet__message.html#gad6fe81536dc232de7ebab51774ed7b5d", [
      [ "kRDMnetConnectOk", "group__rdmnet__message.html#ggad6fe81536dc232de7ebab51774ed7b5da013326d6bbcc6764e544aefe498af08e", null ],
      [ "kRDMnetConnectScopeMismatch", "group__rdmnet__message.html#ggad6fe81536dc232de7ebab51774ed7b5da016a1d7339d7b52156582c4015b3fc49", null ],
      [ "kRDMnetConnectCapacityExceeded", "group__rdmnet__message.html#ggad6fe81536dc232de7ebab51774ed7b5da5fec44a33068337840edf9af208591c7", null ],
      [ "kRDMnetConnectDuplicateUID", "group__rdmnet__message.html#ggad6fe81536dc232de7ebab51774ed7b5dae8912c331cd8b788c9f6c6cafed0a1f9", null ],
      [ "kRDMnetConnectInvalidClientEntry", "group__rdmnet__message.html#ggad6fe81536dc232de7ebab51774ed7b5da6654837ca0b905341a9ec58e4ee450fd", null ]
    ] ],
    [ "rdmnet_disconnect_reason_t", "group__rdmnet__message.html#ga3eb1adaf6e830a807d1dcfbc64197222", [
      [ "kRDMnetDisconnectShutdown", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a042e28ff2cd5843d94de545a7290b2ca", null ],
      [ "kRDMnetDisconnectCapacityExhausted", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a1572a7ccc37a0cf3645834c72a45af40", null ],
      [ "kRDMnetDisconnectIncorrectClientType", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a46282491ec15b2f2c2a5b3d7c9c14260", null ],
      [ "kRDMnetDisconnectHardwareFault", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a5b840f9e1e1b68539be3d549b429e327", null ],
      [ "kRDMnetDisconnectSoftwareFault", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222aa99bf23b991ea121ce20aeccf4fffcee", null ],
      [ "kRDMnetDisconnectSoftwareReset", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222abe023d51b576b8b8ff3dc78c79d316e2", null ],
      [ "kRDMnetDisconnectIncorrectScope", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a6b6764d33dd326ea5161060c97b027e1", null ],
      [ "kRDMnetDisconnectLLRPReconfigure", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a51c38b0d227a7463b24ce601236bc948", null ],
      [ "kRDMnetDisconnectUserReconfigure", "group__rdmnet__message.html#gga3eb1adaf6e830a807d1dcfbc64197222a91f4adef33ad94a44e74c4bf7d06691f", null ]
    ] ],
    [ "free_rdmnet_message", "group__rdmnet__message.html#ga20a1bebbaa680ed3eef71fec2398225b", null ]
];