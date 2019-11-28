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

#include "IpkParseStep.h"
#include <functional>

using namespace std::placeholders;

IpkParseStep::IpkParseStep()
    :m_verify(false)
{
    LOG_DEBUG("IpkParse::IpkParseStep() called\n");
}

IpkParseStep::~IpkParseStep()
{
    LOG_DEBUG("IpkParse::~IpkParseStep() called\n");
}

bool IpkParseStep::proceed(Task *task)
{
    LOG_DEBUG("IpkParse::proceed() called\n");

    m_parentTask = task;
    pbnjson::JValue param = m_parentTask->getParam();
    m_verify = param["verify"].asBool();
    m_appId = param["id"].asString();
    m_ipkFile = param["ipkurl"].asString();

    determineInstallPath();

    if (!Utils::make_dir(m_installDataPath, true))
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_GENERAL, "unable to create install temp directory");
        m_parentTask->proceed();
        return false;
    }

    AppPackage::Control control;
    if (!m_appPackage.extract(m_ipkFile, AppPackage::CONTROL, m_installDataPath,
        std::bind(&IpkParseStep::onPackageExtracted, this, _1)))
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_GENERAL, "failed to extract ipk file");
        m_parentTask->proceed();
        return false;
    }

    return true;
}

//Can be a step
void IpkParseStep::determineInstallPath()
{
    //Apps installed to only INTERNAL_STORAGE
    pbnjson::JValue targetInfo = pbnjson::Object();
    targetInfo.put("deviceId", DeviceId::INTERNAL_STORAGE);

    std::string deviceId = targetInfo["deviceId"].asString();
    std::string installBasePath;

    if (deviceId == DeviceId::INTERNAL_STORAGE)
    {
        installBasePath = Settings::instance().getInstallPath(m_verify);
    }

    m_parentTask->setInstallBasePath(installBasePath);

    m_installDataPath = installBasePath + "/tmp/" + m_appId;

    LOG_DEBUG("[InstallTask][determineInstallPath] deviceId:%s, installBasePath:%s",
        deviceId.c_str(), installBasePath.c_str());
}

void IpkParseStep::onPackageExtracted(bool result)
{
    LOG_DEBUG("[IpkParseStep]::onPackageExtracted. result: %d\n", result);

    if (!result)
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, "Failed to extract package");
        m_parentTask->proceed();
        return;
    }

    AppPackage appPackage;
    AppPackage::Control control;
    if (!appPackage.parseControl(m_installDataPath + "/control", control))
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, "Failed to parse control");
        m_parentTask->proceed();
        return;
    }

    LOG_INFO(MSGID_PACKAGE_INFO, 3,
        PMLOGKS(APP_ID, m_appId.c_str()),
        PMLOGKS("package", control.getPackage().c_str()),
        PMLOGKS("version", control.getVersion().c_str()),
        "");

    if (m_verify && m_appId != control.getPackage())
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, "appId is wrong");
        m_parentTask->proceed();
        return;
    }

    m_parentTask->setPackageId(control.getPackage());

    std::string installedControlFilePath = m_parentTask->getInstallBasePath() + Settings::instance().getOpkgInfoPath() + "/" + control.getPackage() + ".control";

    AppPackage::Control installedControl;
    if (appPackage.parseControl(installedControlFilePath, installedControl))
    {
        if (control.getVersion() == installedControl.getVersion())
            m_parentTask->setAllowReInstall(true);
    }

    uint64_t unpackFileSize = control.getInstalledSize();

    // have "Installed-size" in control file
    if (unpackFileSize != 0)
         m_parentTask->setHasInstalledSizeWithControlFile(true);

    // have "Installed-Size"in control file and do not have "unpackFileSize" in server response
    if (m_parentTask->getUnpackFilesize() == 0 && unpackFileSize != 0)
        //m_parentTask->setFilesize(m_packFileSize, unpackFileSize);
        m_parentTask->setUnpackFilesize(unpackFileSize);


    Utils::remove_dir(m_installDataPath);
    m_parentTask->setStep(IpkParseComplete);
    m_parentTask->proceed();
}


