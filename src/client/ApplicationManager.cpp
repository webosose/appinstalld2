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

#include "ApplicationManager.h"

#include "base/LSUtils.h"
#include "service/AppInstallService.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

ApplicationManager::ApplicationManager()
    : AbsLunaClient("com.webos.service.config")
{
    setClassName("ApplicationManager");
}

ApplicationManager::~ApplicationManager()
{
}

bool ApplicationManager::onLockApp(LSHandle* sh, LSMessage* reply, void* ctx)
{
    return true;
}

bool ApplicationManager::lockApp(const char* sessionId, const string& id, bool lock)
{
    static const string API = "luna://com.webos.service.applicationmanager/lockApp";

    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("lock", lock);

    std::string errorText;
    LSCaller caller = LSUtils::acquireCaller("com.webos.appInstallService");
    if (!caller.CallOneReply(API.c_str(), requestPayload.stringify().c_str(), sessionId, onLockApp, this, nullptr, errorText)) {
        Logger::error(getClassName(), __FUNCTION__, "lockApp error: " + errorText);
        return false;
    }

    return true;
}

void ApplicationManager::onInitialzed()
{
}

void ApplicationManager::onFinalized()
{
}

void ApplicationManager::onServerStatusChanged(bool isConnected)
{
}
