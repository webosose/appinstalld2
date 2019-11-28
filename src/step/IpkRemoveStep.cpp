// Copyright (c) 2017-2018 LG Electronics, Inc.
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

#include "IpkRemoveStep.h"

using namespace std::placeholders;

IpkRemoveStep::IpkRemoveStep()
{
}

IpkRemoveStep::~IpkRemoveStep()
{
    LOG_DEBUG("IpkRemoveStep::~IpkRemoveStep() called\n");
}

bool IpkRemoveStep::proceed(Task *task)
{
    LOG_DEBUG("IpkRemoveStep::proceed() called\n");

    m_parentTask = task;

    pbnjson::JValue param = m_parentTask->getParam();
    bool verify = param["verify"].asBool();
    std::vector<std::string>& appServices = m_parentTask->getServices();

    CallChain& callchain = CallChain::acquire(std::bind(&IpkRemoveStep::onIpkRemoved,
        this, _1, _2));

    auto itemInternal = std::make_shared<CallChainEventHandler::RemoveIpk>(
        m_parentTask->getAppId(),
        verify,
        std::string("")
    );
    callchain.add(itemInternal);

    // read services
    {
        std::string packagePath = Settings::instance().getInstallPath(verify) +
            Settings::instance().getPackageinstallPath() +
            std::string("/") + m_parentTask->getAppId();

        PackageInfo packageInfo(packagePath);
        if (packageInfo.isLoaded())
            packageInfo.getServices(appServices);
    }
    // read dev services
    if (verify && appServices.empty())
    {
        std::string packagePath = Settings::instance().getInstallPath(false) +
            Settings::instance().getPackageinstallPath() +
            std::string("/") + m_parentTask->getAppId();

        PackageInfo packageInfo(packagePath);
        if (packageInfo.isLoaded())
            packageInfo.getServices(appServices);
    }

    m_parentTask->setStep(IpkRemoveRequested);
    callchain.run();
    return true;
}

bool IpkRemoveStep::onIpkRemoved(pbnjson::JValue result, void *user_data)
{
    LOG_DEBUG("IpkRemoveStep::onIpkRemoved() called\n");

    bool returnValue = result["returnValue"].asBool();

    int removed = (result.hasKey("removed")) ? result["removed"].asNumber<int>() : 0;
    int failed = (result.hasKey("failed"))? result["failed"].asNumber<int>() : 0;
    if ( (removed == 0) || (failed > 0) )
        returnValue = false;

    if (returnValue)
    {
        // internal remove when installing error occurs.. keep app data and finish remove step.
        if (m_parentTask->getSender() == "com.webos.appInstallService")
        {
            LOG_DEBUG("IpkRemoveStep::onIpkRemoved: internal remove when installing error. keep app data\n");
            m_parentTask->setStep(RemoveComplete);
        }
        else
        {
            m_parentTask->setStep(IpkRemoveComplete);
        }
    }
    else
    {
        if (result["locked"].asBool())
        {
//            m_parentTask->setStep(IpkRemoveNeeded);
//            pause(SYSTEM);
//            return true;
        }

        std::string errorText = result["errorText"].asString();
        if (errorText.empty())
            errorText = "FAILED_REMOVE";
        m_parentTask->setError(ErrorRemove, APP_REMOVE_ERR_REMOVE, errorText);
    }

    m_parentTask->proceed();
    return true;
}
