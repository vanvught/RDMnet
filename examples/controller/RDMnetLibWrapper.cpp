/******************************************************************************
************************* IMPORTANT NOTE -- READ ME!!! ************************
*******************************************************************************
* THIS SOFTWARE IMPLEMENTS A **DRAFT** STANDARD, BSR E1.33 REV. 63. UNDER NO
* CIRCUMSTANCES SHOULD THIS SOFTWARE BE USED FOR ANY PRODUCT AVAILABLE FOR
* GENERAL SALE TO THE PUBLIC. DUE TO THE INEVITABLE CHANGE OF DRAFT PROTOCOL
* VALUES AND BEHAVIORAL REQUIREMENTS, PRODUCTS USING THIS SOFTWARE WILL **NOT**
* BE INTEROPERABLE WITH PRODUCTS IMPLEMENTING THE FINAL RATIFIED STANDARD.
*******************************************************************************
* Copyright 2018 ETC Inc.
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

#include "RDMnetLibWrapper.h"

static void controllercb_connected(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                   const RdmnetClientConnectedInfo *info, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->Connected(handle, scope, info);
}

static void controllercb_not_connected(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                       const RdmnetClientNotConnectedInfo *info, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->NotConnected(handle, scope, info);
}

static void controllercb_client_list_update(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                            client_list_action_t list_action, const ClientList *list, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->ClientListUpdate(handle, scope, list_action, list);
}

static void controllercb_rdm_response_received(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                               const RemoteRdmResponse *resp, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->RdmResponseReceived(handle, scope, resp);
}

static void controllercb_rdm_command_received(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                              const RemoteRdmCommand *cmd, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->RdmCommandReceived(handle, scope, cmd);
}

static void controllercb_status_received(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                         const RemoteRptStatus *status, void *context)
{
  RDMnetLibNotifyInternal *notify = static_cast<RDMnetLibNotifyInternal *>(context);
  if (notify)
    notify->StatusReceived(handle, scope, status);
}

RDMnetLibWrapper::RDMnetLibWrapper(ControllerLog *log) : log_(log)
{
  lwpa_generate_v4_uuid(&my_cid_);
}

bool RDMnetLibWrapper::Startup(RDMnetLibNotify *notify)
{
  if (!running_)
  {
    notify_ = notify;

    // Initialize the RDMnet core library
    lwpa_error_t res = rdmnet_core_init(log_ ? log_->GetLogParams() : nullptr);
    if (res != kLwpaErrOk)
    {
      if (log_)
        log_->Log(LWPA_LOG_ERR, "Error initializing RDMnet core library: '%s'", lwpa_strerror(res));
      return false;
    }

    // Create our controller handle in the RDMnet library
    RdmnetControllerConfig config;
    config.uid = RPT_CLIENT_DYNAMIC_UID(0x6574);
    config.cid = my_cid_;
    config.callbacks = {controllercb_connected,
                        controllercb_not_connected,
                        controllercb_client_list_update,
                        controllercb_rdm_response_received,
                        controllercb_rdm_command_received,
                        controllercb_status_received};
    config.callback_context = this;

    res = rdmnet_controller_create(&config, &controller_handle_);
    if (res != kLwpaErrOk)
    {
      if (log_)
        log_->Log(LWPA_LOG_ERR, "Error creating an RDMnet Controller handle: '%s'", lwpa_strerror(res));
      rdmnet_core_deinit();
      return false;
    }

    running_ = true;
  }
  return true;
}

void RDMnetLibWrapper::Shutdown()
{
  if (running_)
  {
    rdmnet_controller_destroy(controller_handle_);
    rdmnet_core_deinit();
    running_ = false;
  }
}

rdmnet_client_scope_t RDMnetLibWrapper::AddScope(const std::string &scope, StaticBrokerConfig static_broker)
{
  // Check if the scope is too long
  if (scope.length() >= E133_SCOPE_STRING_PADDED_LENGTH)
    return RDMNET_CLIENT_SCOPE_INVALID;

  RdmnetScopeConfig config;
  rdmnet_client_set_scope(&config, scope.c_str());
  if (static_broker.valid)
  {
    config.has_static_broker_addr = true;
    config.static_broker_addr = static_broker.addr;
  }
  else
  {
    config.has_static_broker_addr = false;
  }

  rdmnet_client_scope_t new_scope_handle;
  lwpa_error_t res = rdmnet_controller_add_scope(controller_handle_, &config, &new_scope_handle);
  if (res != kLwpaErrOk)
  {
    if (log_)
      log_->Log(LWPA_LOG_ERR, "Error adding new RDMnet scope '%s': '%s'", scope.c_str(), lwpa_strerror(res));
    return RDMNET_CLIENT_SCOPE_INVALID;
  }
  return new_scope_handle;
}

void RDMnetLibWrapper::Connected(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                 const RdmnetClientConnectedInfo *info)
{
  if (notify_ && handle == controller_handle_ && info)
  {
    notify_->Connected(scope, *info);
  }
}

void RDMnetLibWrapper::NotConnected(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                    const RdmnetClientNotConnectedInfo *info)
{
  if (notify_ && handle == controller_handle_ && info)
  {
    notify_->NotConnected(scope, *info);
  }
}

void RDMnetLibWrapper::ClientListUpdate(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                        client_list_action_t list_action, const ClientList *list)
{
  if (notify_ && handle == controller_handle_ && list)
  {
    notify_->ClientListUpdate(scope, list_action, *list);
  }
}

void RDMnetLibWrapper::RdmResponseReceived(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                           const RemoteRdmResponse *resp)
{
  if (notify_ && handle == controller_handle_ && resp)
  {
    notify_->RdmResponseReceived(scope, *resp);
  }
}

void RDMnetLibWrapper::RdmCommandReceived(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                          const RemoteRdmCommand *cmd)
{
  if (notify_ && handle == controller_handle_ && cmd)
  {
    notify_->RdmCommandReceived(scope, *cmd);
  }
}

void RDMnetLibWrapper::StatusReceived(rdmnet_controller_t handle, rdmnet_client_scope_t scope,
                                      const RemoteRptStatus *status)
{
  if (notify_ && handle == controller_handle_ && status)
  {
    notify_->StatusReceived(scope, *status);
  }
}
