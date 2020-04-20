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

#include "SessionManager.h"

#include "base/SessionList.h"
#include "service/AppInstallService.h"
#include "util/Logger.h"

SessionManager::SessionManager()
    : AbsLunaClient("com.webos.service.sessionmanager")
{
    setClassName("SessionManager");
}

SessionManager::~SessionManager()
{
}

void SessionManager::onInitialzed()
{
}

void SessionManager::onFinalized()
{
    m_getSessionListCall.cancel();
}

void SessionManager::onServerStatusChanged(bool isConnected)
{
    static string method = string("luna://") + getName() + string("/getSessionList");

    if (isConnected) {
        if (m_getSessionListCall.isActive())
            return;

        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);

        m_getSessionListCall = AppInstallService::getInstance().callMultiReply(
            method.c_str(),
            requestPayload.stringify().c_str(),
            onGetSessionList,
            nullptr
        );
    } else {
        m_getSessionListCall.cancel();
    }
}

bool SessionManager::onGetSessionList(LSHandle* sh, LSMessage* message, void* context)
{
    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse(getInstance().getClassName(), __FUNCTION__, response, subscriptionPayload);

    if (subscriptionPayload.isNull())
        return false;

    JValue sessionList;
    if (JValueUtil::getValue(subscriptionPayload, "sessionList", sessionList) && sessionList.isArray()) {
        SessionList::getInstance().clear();
        int arraySize = sessionList.arraySize();
        for (int i = 0; i < arraySize; ++i) {
            string sessionId;
            if (!JValueUtil::getValue(sessionList[i], "sessionId", sessionId))
                continue;
            SessionList::getInstance().emplace_back(sessionId);
            Logger::info(SessionManager::getInstance().getClassName(), "session", sessionId);
        }
    }

    return true;
}
