// Copyright (c) 2017-2020 LG Electronics, Inc.
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

#include "AppCloseStep.h"

#include <boost/algorithm/string/predicate.hpp>
#include "installer/Task.h"

#include "base/SessionList.h"
#include "settings/Settings.h"

using namespace std::placeholders;

AppCloseStep::AppCloseStep()
{
}

AppCloseStep::~AppCloseStep()
{
    LOG_DEBUG("AppCloseStep::~AppCloseStep called\n");
}

bool AppCloseStep::proceed(Task *task)
{
    LOG_DEBUG("AppCloseStep::proceed() called\n");
    m_parentTask = task;
    std::string packageId = task->getPackageId();

    if(!checkPriviligedApp())
    {
        if (task->getName() == "RemoveTask")
            task->setError(ErrorRemove, APP_REMOVE_ERR_PRIVILEGED, "Cannnot remove privileged app on developer mode");
        else
            task->setError(ErrorInstall, APP_INSTALL_ERR_PRIVILEGED, "Cannnot install privileged app on developer mode");

        task->proceed();
        return false;
    }

    CallChain& callchain = CallChain::acquire(std::bind(&AppCloseStep::onAppClosed,
        this, _1, _2));

    if (Settings::instance().isSupportMultiProfile()) {
        size_t size = SessionList::getInstance().size();
        for (size_t i = 0; i < size; ++i) {
            const std::string& sessionId = SessionList::getInstance().at(i);
            addCallItems(sessionId.c_str(), packageId, callchain);
        }
    } else {
        addCallItems(nullptr, packageId, callchain);
    }

    m_parentTask->setStep(AppCloseRequested);

    return callchain.run();
}

void AppCloseStep::onAppClosed(pbnjson::JValue result, void *user_data)
{
    LOG_DEBUG("AppCloseStep::onAppClosed() called\n");

    bool returnValue = result["returnValue"].asBool();
    if (!returnValue)
    {
        if (m_parentTask->getName() == "RemoveTask")
            m_parentTask->setError(ErrorRemove, APP_REMOVE_ERR_REMOVE, result["errorText"].asString());
        else
            m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, result["errorText"].asString());

    }
    else
    {
        m_parentTask->setStep(AppCloseComplete);
    }

    m_parentTask->proceed();
}

bool AppCloseStep::checkPriviligedApp()
{
    std::string packageId = m_parentTask->getPackageId();
    pbnjson::JValue param = m_parentTask->getParam();
    bool verify = param["verify"].asBool();

    LOG_DEBUG("[AppCloseStep]::checkPriviligedApp. PackageId:%s, verify:%d\n", packageId.c_str(), verify);

    if (!verify)
    {
        if (boost::starts_with(packageId, std::string("com.palm.")) ||
            boost::starts_with(packageId, std::string("com.webos.")) ||
            boost::starts_with(packageId, std::string("com.lge.")))
        {
            return false;
        }
    }

    return true;
}

void AppCloseStep::addCallItems(const char *sessionId, const string& packageId, CallChain& callchain)
{
    auto itemAppInfo = std::make_shared<CallChainEventHandler::AppInfo>(
        "com.webos.appInstallService",
        sessionId,
        packageId
    );
    itemAppInfo->setOption(CallItem::OPTION_NONSTOP);

    auto itemAppRemovable = std::make_shared<CallChainEventHandler::AppRemovable>(
    );

    auto itemAppLock = std::make_shared<CallChainEventHandler::AppLock>(
        "com.webos.appInstallService",
        sessionId,
        packageId
    );

    auto itemRunning = std::make_shared<CallChainEventHandler::AppRunning>(
        "com.webos.appInstallService",
        sessionId,
        packageId
    );
    itemRunning->setOption(CallItem::OPTION_NONSTOP);

    auto itemClose = std::make_shared<CallChainEventHandler::AppClose>(
        "com.webos.appInstallService",
        sessionId,
        packageId
    );

    auto itemSvcClose = std::make_shared<CallChainEventHandler::SvcClose>(
        nullptr // TODO Not sure whether service is running in session or host
    );

    callchain
        .add(itemAppInfo)
        .add_if(itemAppInfo, true, itemAppRemovable)
        .add_if(itemAppInfo, true, itemAppLock)
        .add(itemRunning)
        .add_if(itemRunning, true, itemClose)
        .add(itemSvcClose);
}
