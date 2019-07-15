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

#include "rdmnet/core/llrp.h"
#include "rdmnet/private/llrp.h"
#include "rdmnet/private/llrp_manager.h"
#include "rdmnet/private/llrp_target.h"
#include "rdmnet/private/opts.h"

/**************************** Global variables *******************************/

LwpaSockaddr kLlrpIpv4RespAddrInternal;
LwpaSockaddr kLlrpIpv6RespAddrInternal;
LwpaSockaddr kLlrpIpv4RequestAddrInternal;
LwpaSockaddr kLlrpIpv6RequestAddrInternal;

const LwpaSockaddr* kLlrpIpv4RespAddr = &kLlrpIpv4RespAddrInternal;
const LwpaSockaddr* kLlrpIpv6RespAddr = &kLlrpIpv6RespAddrInternal;
const LwpaSockaddr* kLlrpIpv4RequestAddr = &kLlrpIpv4RequestAddrInternal;
const LwpaSockaddr* kLlrpIpv6RequestAddr = &kLlrpIpv6RequestAddrInternal;

/*********************** Private function prototypes *************************/

lwpa_error_t create_sys_socket(const LwpaIpAddr* netint, bool manager, lwpa_socket_t* socket);
lwpa_error_t subscribe_multicast(lwpa_socket_t socket, const LwpaIpAddr* netint, bool manager);

/*************************** Function definitions ****************************/

lwpa_error_t rdmnet_llrp_init()
{
  lwpa_error_t res = kLwpaErrOk;
#if RDMNET_DYNAMIC_MEM
  bool manager_initted = false;
#endif
  bool target_initted = false;

  lwpa_inet_pton(kLwpaIpTypeV4, LLRP_MULTICAST_IPV4_ADDRESS_RESPONSE, &kLlrpIpv4RespAddrInternal.ip);
  kLlrpIpv4RespAddrInternal.port = LLRP_PORT;
  lwpa_inet_pton(kLwpaIpTypeV4, LLRP_MULTICAST_IPV6_ADDRESS_RESPONSE, &kLlrpIpv6RespAddrInternal.ip);
  kLlrpIpv6RespAddrInternal.port = LLRP_PORT;
  lwpa_inet_pton(kLwpaIpTypeV4, LLRP_MULTICAST_IPV4_ADDRESS_REQUEST, &kLlrpIpv4RequestAddrInternal.ip);
  kLlrpIpv4RequestAddrInternal.port = LLRP_PORT;
  lwpa_inet_pton(kLwpaIpTypeV4, LLRP_MULTICAST_IPV6_ADDRESS_REQUEST, &kLlrpIpv6RequestAddrInternal.ip);
  kLlrpIpv6RequestAddrInternal.port = LLRP_PORT;
  llrp_prot_init();

#if RDMNET_DYNAMIC_MEM
  manager_initted = ((res = rdmnet_llrp_manager_init()) == kLwpaErrOk);
#endif

  if (res == kLwpaErrOk)
  {
    target_initted = ((res = rdmnet_llrp_target_init()) == kLwpaErrOk);
  }

  if (res != kLwpaErrOk)
  {
    if (target_initted)
      rdmnet_llrp_target_deinit();
#if RDMNET_DYNAMIC_MEM
    if (manager_initted)
      rdmnet_llrp_manager_deinit();
#endif
  }

  return res;
}

void rdmnet_llrp_deinit()
{
  rdmnet_llrp_target_deinit();
#if RDMNET_DYNAMIC_MEM
  rdmnet_llrp_manager_deinit();
#endif
}

lwpa_error_t create_llrp_socket(const LwpaIpAddr* netint, bool manager, lwpa_socket_t* socket)
{
  lwpa_socket_t socket_out;

  lwpa_error_t res = create_sys_socket(netint, manager, &socket_out);
  if (res == kLwpaErrOk)
    res = subscribe_multicast(socket_out, netint, manager);
  if (res == kLwpaErrOk)
    *socket = socket_out;
  return res;
}

lwpa_error_t create_sys_socket(const LwpaIpAddr* netint, bool manager, lwpa_socket_t* socket)
{
  lwpa_socket_t sock = LWPA_SOCKET_INVALID;
  lwpa_error_t res = lwpa_socket(LWPA_IP_IS_V6(netint) ? LWPA_AF_INET6 : LWPA_AF_INET, LWPA_DGRAM, &sock);

  if (res == kLwpaErrOk)
  {
    // SO_REUSEADDR allows multiple sockets to bind to LLRP_PORT, which is very important for our
    // multicast needs.
    int option = 1;
    res = lwpa_setsockopt(sock, LWPA_SOL_SOCKET, LWPA_SO_REUSEADDR, (const void*)(&option), sizeof(option));
  }

  if (res == kLwpaErrOk)
  {
    // MULTICAST_TTL controls the TTL field in outgoing multicast datagrams.
    if (LWPA_IP_IS_V4(netint))
    {
      int value = LLRP_MULTICAST_TTL_VAL;
      res = lwpa_setsockopt(sock, LWPA_IPPROTO_IP, LWPA_IP_MULTICAST_TTL, (const void*)(&value), sizeof(value));
    }
    else
    {
      // TODO: add Ipv6 support
    }
  }

  if (res == kLwpaErrOk)
  {
    // MULTICAST_IF is critical for multicast sends to go over the correct interface.
    if (LWPA_IP_IS_V4(netint))
    {
      res = lwpa_setsockopt(sock, LWPA_IPPROTO_IP, LWPA_IP_MULTICAST_IF, (const void*)(netint), sizeof(LwpaIpAddr));
    }
    else
    {
      // TODO: add Ipv6 support
    }
  }

  if (res == kLwpaErrOk)
  {
    if (LWPA_IP_IS_V4(netint))
    {
      LwpaSockaddr bind_addr;
#if RDMNET_LLRP_BIND_TO_MCAST_ADDRESS
      // Bind socket to multicast address for IPv4
      bind_addr.ip = (manager ? kLlrpIpv4RespAddrInternal.ip : kLlrpIpv4RequestAddrInternal.ip);
#else
      (void)manager;
      // Bind socket to INADDR_ANY
      lwpa_ip_set_wildcard(kLwpaIpTypeV4, &bind_addr.ip);
#endif
      bind_addr.port = LLRP_PORT;
      res = lwpa_bind(sock, &bind_addr);
    }
    else
    {
      // TODO: add Ipv6 support
    }
  }

  if (res == kLwpaErrOk)
  {
    *socket = sock;
  }
  else
  {
    lwpa_close(sock);
  }

  return res;
}

lwpa_error_t subscribe_multicast(lwpa_socket_t socket, const LwpaIpAddr* netint, bool manager)
{
  lwpa_error_t res = kLwpaErrNotImpl;

  if (LWPA_IP_IS_V4(netint))
  {
    LwpaMreq multireq;

    multireq.group = (manager ? kLlrpIpv4RespAddrInternal.ip : kLlrpIpv4RequestAddrInternal.ip);
    multireq.netint = *netint;
    res = lwpa_setsockopt(socket, LWPA_IPPROTO_IP, LWPA_MCAST_JOIN_GROUP, (const void*)&multireq, sizeof(multireq));
  }
  else
  {
    // TODO: add Ipv6 support
  }

  return res;
}

void rdmnet_llrp_tick()
{
#if RDMNET_DYNAMIC_MEM
  rdmnet_llrp_manager_tick();
#endif
  rdmnet_llrp_target_tick();
}
