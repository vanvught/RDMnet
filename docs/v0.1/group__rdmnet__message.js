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
    [ "EptMessage", "struct_ept_message.html", [
      [ "dummy", "struct_ept_message.html#a7c1d654b7b6114d7a0abc8d351dd1bcd", null ]
    ] ],
    [ "RdmnetMessage", "struct_rdmnet_message.html", [
      [ "broker", "struct_rdmnet_message.html#a54302c30c5004baad9cef89ca9f946e3", null ],
      [ "data", "struct_rdmnet_message.html#a31c56d4326b56cf9d63d072c093992cb", null ],
      [ "ept", "struct_rdmnet_message.html#ad71556bc99ba5fd8b8b384e48a34d911", null ],
      [ "rpt", "struct_rdmnet_message.html#acfe970667ad0b3d46b792ac29d319d14", null ],
      [ "sender_cid", "struct_rdmnet_message.html#aa637edf35bbbdf0fceb946c870aca115", null ],
      [ "vector", "struct_rdmnet_message.html#a513d751c7097b43b3968ac81f3a5715b", null ]
    ] ],
    [ "RptHeader", "struct_rpt_header.html", [
      [ "dest_endpoint_id", "struct_rpt_header.html#a796b124e76b7c4fc052e33c92542a137", null ],
      [ "dest_uid", "struct_rpt_header.html#a5bd5d92fd21a26e7c55a4528d714e2eb", null ],
      [ "seqnum", "struct_rpt_header.html#ad088047c391303bc8e77e28fdc8c1f77", null ],
      [ "source_endpoint_id", "struct_rpt_header.html#a8dada8b2b88d8c5b3c99ee92fd9dc4da", null ],
      [ "source_uid", "struct_rpt_header.html#a7e2cc867da711e1facb2132e0bc10e76", null ]
    ] ],
    [ "RptStatusMsg", "struct_rpt_status_msg.html", [
      [ "status_code", "struct_rpt_status_msg.html#ad1e92880377c93c4e7a412a5bb57639c", null ],
      [ "status_string", "struct_rpt_status_msg.html#a531d2fcefb28ff0c8c9eae9b6078d16b", null ]
    ] ],
    [ "RdmCmdListEntry", "struct_rdm_cmd_list_entry.html", [
      [ "msg", "struct_rdm_cmd_list_entry.html#a8b5ccb6be7a1562c3c083ce2f4fabbed", null ],
      [ "next", "struct_rdm_cmd_list_entry.html#a73ecbe1746d26fbea8f98573634a995e", null ]
    ] ],
    [ "RdmCmdList", "struct_rdm_cmd_list.html", [
      [ "list", "struct_rdm_cmd_list.html#af2538e6ae1d367ce536c9f7490655946", null ],
      [ "partial", "struct_rdm_cmd_list.html#a450e8f772f5ba536489f6cdd3a0954af", null ]
    ] ],
    [ "RptMessage", "struct_rpt_message.html", [
      [ "data", "struct_rpt_message.html#a88648288454818417dd39c5ac3a0b224", null ],
      [ "header", "struct_rpt_message.html#ab841c527cb68301d94706726f8a20ae0", null ],
      [ "rdm", "struct_rpt_message.html#a0ba53544fc51c4f608161bb2e6c5b434", null ],
      [ "status", "struct_rpt_message.html#aceb098cf5bda340f2a3e6135e7d020fe", null ],
      [ "vector", "struct_rpt_message.html#a513d751c7097b43b3968ac81f3a5715b", null ]
    ] ],
    [ "CONNECTFLAG_INCREMENTAL_UPDATES", "group__rdmnet__message.html#ga21824dc15b7fe1e55a19cfc73081b4de", null ],
    [ "get_broker_msg", "group__rdmnet__message.html#gaa5d26affd96e54bd2bb77eb5fe069bba", null ],
    [ "get_client_connect_msg", "group__rdmnet__message.html#ga2dcb0b4b64585d3cecbff9a3aa7407b2", null ],
    [ "get_client_entry_update_msg", "group__rdmnet__message.html#gaa1a9b7a7605bd406b790a36960119d3d", null ],
    [ "get_client_list", "group__rdmnet__message.html#ga6d92fad50037974b48edfc7433ac1adf", null ],
    [ "get_client_redirect_msg", "group__rdmnet__message.html#gadc30f0bcc63621dd0dcb7ac8b2ad5e17", null ],
    [ "get_connect_reply_msg", "group__rdmnet__message.html#gaf5e88e310173d995f0761c89d832c20e", null ],
    [ "get_disconnect_msg", "group__rdmnet__message.html#ga8a20a282c28d6904ded37688a9e11803", null ],
    [ "get_ept_msg", "group__rdmnet__message.html#ga6d7db520b1c023c2a7ff29d244df110d", null ],
    [ "get_rdm_cmd_list", "group__rdmnet__message.html#ga5b7eb4eccfbd6ccedd79d8fbda9a7fae", null ],
    [ "get_rpt_msg", "group__rdmnet__message.html#ga659ec09c5a216dc99cf09e91a5373cc7", null ],
    [ "get_rpt_status_msg", "group__rdmnet__message.html#gae830698553e4ce011489cf12e7855a72", null ],
    [ "is_broker_msg", "group__rdmnet__message.html#gadb35a3f4a44cd089e43c3ae17984edce", null ],
    [ "is_client_connect_msg", "group__rdmnet__message.html#ga93de501041b989bda8ded87537fd56c6", null ],
    [ "is_client_entry_update_msg", "group__rdmnet__message.html#ga0043b37bd42a8ee5a54b397a6505016a", null ],
    [ "is_client_list", "group__rdmnet__message.html#ga4c7e1fe2262c509c50cea4fcfc3d9c02", null ],
    [ "is_client_redirect_msg", "group__rdmnet__message.html#ga905bfac8412263dba38a6cf4bb1a619a", null ],
    [ "is_connect_reply_msg", "group__rdmnet__message.html#ga9b3a4dd6b8f5ec3d6f2d64ccf2d04da4", null ],
    [ "is_disconnect", "group__rdmnet__message.html#gaff668c6f43f0af86abc282655bbfd6e8", null ],
    [ "is_ept_msg", "group__rdmnet__message.html#ga6fead3eaa9e856ffac42ceacfb6c53eb", null ],
    [ "is_rdm_cmd_list", "group__rdmnet__message.html#ga9d93c21ef50e3dfe3ec3f38514af8088", null ],
    [ "is_rpt_msg", "group__rdmnet__message.html#ga4914f526753ca471b2304932f08b5904", null ],
    [ "is_rpt_status_msg", "group__rdmnet__message.html#gab453d82c138f3d9b8c9b70ea9f1a3971", null ],
    [ "RPT_PDU_FULL_HEADER_SIZE", "group__rdmnet__message.html#ga66eb4ed7550104a35d195dc2ab4eea61", null ],
    [ "RPT_PDU_HEADER_SIZE", "group__rdmnet__message.html#ga1440b2617f08e835a953f2e0102a377f", null ],
    [ "RPT_STATUS_FULL_MSG_MAX_SIZE", "group__rdmnet__message.html#ga2b73e9ddbbcf91f7442930573cd80108", null ],
    [ "RPT_STATUS_HEADER_SIZE", "group__rdmnet__message.html#ga1829149ac5586661278df5e4f429e288", null ],
    [ "RPT_STATUS_STRING_MAXLEN", "group__rdmnet__message.html#ga24088f7128baf48f979d18c65e93d9a6", null ],
    [ "BrokerMessage", "group__rdmnet__message.html#gac7c8058c147b8a783b2becb15af62220", null ],
    [ "ClientConnectMsg", "group__rdmnet__message.html#ga8be6a28c2f85ef9c1714322b84596646", null ],
    [ "ClientEntryUpdateMsg", "group__rdmnet__message.html#gaf76e0e6113046ef35b2e3a8ca25b985e", null ],
    [ "ClientList", "group__rdmnet__message.html#ga1bf51df9a85f7e624633d2fc6746fe9f", null ],
    [ "ClientRedirectMsg", "group__rdmnet__message.html#ga824e9490f3ce01a825e6a6a1218f9a11", null ],
    [ "ConnectReplyMsg", "group__rdmnet__message.html#ga2bb52b926162275184bebdcd615295e9", null ],
    [ "DisconnectMsg", "group__rdmnet__message.html#gac9c8d5b1565aea3c991b76b907340e6f", null ],
    [ "EptMessage", "group__rdmnet__message.html#ga0db66aedcced66ad8c118d928452025c", null ],
    [ "RdmCmdList", "group__rdmnet__message.html#ga762b21fdf305c9e5682e4c921b74177a", null ],
    [ "RdmnetMessage", "group__rdmnet__message.html#gaefbdd2f0f04bb6c4c247b9e1d0128ec1", null ],
    [ "RptHeader", "group__rdmnet__message.html#ga5b7f4ce15c9e39f9f70c2a6666fe0fd5", null ],
    [ "RptMessage", "group__rdmnet__message.html#gae4dbcafacb0ec3d04949592f123b3917", null ],
    [ "RptStatusMsg", "group__rdmnet__message.html#ga6cedfa2c297c958319ea18f60deba61c", null ],
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
    [ "rpt_status_code_t", "group__rdmnet__message.html#gaa9aca0658bfae057363a520c0d9907c2", [
      [ "kRptStatusUnknownRptUid", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2af30548e86cb4ffea1370501b566ccb4b", null ],
      [ "kRptStatusRdmTimeout", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2a08699760f9bff1a12623953eb3ca513b", null ],
      [ "kRptStatusInvalidRdmResponse", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2aacb7604870f3fafd5847d235ec8cbc1f", null ],
      [ "kRptStatusUnknownRdmUid", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2adc85d110baa8e6f493a4e193f83a82fc", null ],
      [ "kRptStatusUnknownEndpoint", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2a196f60d23a101c1dcb4c543285129043", null ],
      [ "kRptStatusBroadcastComplete", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2ac057a735905bfc371c7a577a2583a922", null ],
      [ "kRptStatusUnknownVector", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2a57eb7961b3b8e07041126584172495fb", null ],
      [ "kRptStatusInvalidMessage", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2a64656d8b931ee3468a3f17c2619d1517", null ],
      [ "kRptStatusInvalidCommandClass", "group__rdmnet__message.html#ggaa9aca0658bfae057363a520c0d9907c2ab5343b3586658f6a836febf088617f90", null ]
    ] ],
    [ "bufsize_client_list", "group__rdmnet__message.html#ga8331a696f154ec573e4d6ba887231171", null ],
    [ "bufsize_rpt_notification", "group__rdmnet__message.html#ga3123f987b5e41e45d4d4a83b1a329b82", null ],
    [ "bufsize_rpt_request", "group__rdmnet__message.html#ga76b937d48a55bb1016d9cd33ff9fc97f", null ],
    [ "bufsize_rpt_status", "group__rdmnet__message.html#gac03037a77e0e0c816988ee24742ca5f5", null ],
    [ "free_rdmnet_message", "group__rdmnet__message.html#ga20a1bebbaa680ed3eef71fec2398225b", null ],
    [ "pack_client_list", "group__rdmnet__message.html#ga610a5ddebaaa6fa493ef296e2650480b", null ],
    [ "pack_connect_reply", "group__rdmnet__message.html#ga9fcb3b5fcdd4090dddb3814084a044d5", null ],
    [ "pack_rpt_notification", "group__rdmnet__message.html#gab22781070bfd7c1d0ecfc7ea50252fb9", null ],
    [ "pack_rpt_request", "group__rdmnet__message.html#ga9c1978a0d9706efb73c1d6a9abc40a24", null ],
    [ "pack_rpt_status", "group__rdmnet__message.html#ga960387a5c5d994835268180cc9d39778", null ],
    [ "send_connect_reply", "group__rdmnet__message.html#gae0b1e317f393a4fb883fce9248e7e4d6", null ],
    [ "send_fetch_client_list", "group__rdmnet__message.html#gae6f3e87139019e85636542e31d79d8e4", null ],
    [ "send_rpt_notification", "group__rdmnet__message.html#ga792feea60685b669c1f8d1837c43e71f", null ],
    [ "send_rpt_request", "group__rdmnet__message.html#gae41a90bbbaa6e3f1ff53b414007f96c5", null ],
    [ "send_rpt_status", "group__rdmnet__message.html#ga6a3fc76e7e006cebcdf11f786caf752d", null ]
];