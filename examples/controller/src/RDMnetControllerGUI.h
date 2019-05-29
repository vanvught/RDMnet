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

#pragma once

#include <memory>
#include "ControllerUtils.h"

BEGIN_INCLUDE_QT_HEADERS()
#include <QMainWindow>
#include <QItemSelection>
#include "ui_RDMnetControllerGUI.h"
END_INCLUDE_QT_HEADERS()

#include "ControllerLog.h"
#include "RDMnetLibWrapper.h"
#include "BrokerStaticAddGUI.h"
#include "RDMnetNetworkModel.h"
#include "SimpleNetworkProxyModel.h"
#include "NetworkDetailsProxyModel.h"
#include "RDMnetNetworkItem.h"

class RDMnetControllerGUI : public QMainWindow, public IHandlesBrokerStaticAdd
{
  Q_OBJECT

public:
  static RDMnetControllerGUI *makeRDMnetControllerGUI();

  ~RDMnetControllerGUI();

  virtual void handleAddBrokerByIP(QString scope, const LwpaSockaddr &addr);

public slots:

  void networkTreeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  void addScopeTriggered();
  void removeSelectedBrokerTriggered();
  void removeAllBrokersTriggered();
  void resetDeviceTriggered();
  void identifyDeviceTriggered();
  void openBrokerStaticAddDialog();
  void openLogWindowDialog();
  void openAboutDialog();
  void processBrokerItemTextUpdate(const class BrokerItem *item);
  void processFeatureSupportChange(const class RDMnetNetworkItem *item, SupportedDeviceFeature feature);
  void expandNewItem(const QModelIndex &index, int type);
  void identifyChanged(const RDMnetNetworkItem *item, bool identify);
  void exitApplication();

signals:

  void addScopeActivated(QString scope);
  void removeSelectedBrokerActivated(class BrokerItem *brokerItem);
  void removeAllBrokersActivated();
  void featureActivated(class RDMnetNetworkItem *device, SupportedDeviceFeature feature);
  void addBrokerByIPActivated(QString scope, const LwpaSockaddr &addr);

private:
  explicit RDMnetControllerGUI(QWidget *parent = Q_NULLPTR);

  Ui::RDMnetControllerGUIClass ui;

  RDMnetNetworkModel *main_network_model_{nullptr};
  SimpleNetworkProxyModel *simple_net_proxy_{nullptr};
  NetworkDetailsProxyModel *net_details_proxy_{nullptr};
  ControllerLog *log_{nullptr};
  RDMnetLibWrapper *rdmnet_library_{nullptr};
  BrokerItem *currently_selected_broker_item_{nullptr};
  RDMnetNetworkItem *currently_selected_network_item_{nullptr};
};