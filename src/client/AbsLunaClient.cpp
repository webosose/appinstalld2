// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "AbsLunaClient.h"

#include "service/AppInstallService.h"

bool AbsLunaClient::_onServerStatus(LSHandle* sh, LSMessage* message, void* context)
{
    AbsLunaClient* client = static_cast<AbsLunaClient*>(context);

    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    if (subscriptionPayload.isNull())
        return true;

    bool connected = false;
    if (!JValueUtil::getValue(subscriptionPayload, "connected", connected)) {
        return true;
    }

    if (connected)
        Logger::info(client->getClassName(), __FUNCTION__, "Service is up");
    else
        Logger::info(client->getClassName(), __FUNCTION__, "Service is down");

    client->m_isConnected = connected;
    client->onServerStatusChanged(connected);
    return true;
}

AbsLunaClient::AbsLunaClient(const string& name)
    : m_name(name),
      m_isConnected(false)
{
    setClassName("AbsLunaClient");
}

AbsLunaClient::~AbsLunaClient()
{
}

void AbsLunaClient::initialize()
{
    Logger::info(getClassName(), __FUNCTION__, "");

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("serviceName", getName());
    m_statusCall = AppInstallService::getInstance().callMultiReply(
        "luna://com.webos.service.bus/signal/registerServerStatus",
        requestPayload.stringify().c_str(),
        _onServerStatus,
        this
    );

    onInitialzed();
}

void AbsLunaClient::finalize()
{
    Logger::info(getClassName(), __FUNCTION__, "");

    m_statusCall.cancel();
    onFinalized();
}
