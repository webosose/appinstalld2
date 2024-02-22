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

#include "IpkInstallStep.h"
#include <functional>
#include "installer/Task.h"

using namespace std::placeholders;

const std::vector<std::string> opkgInstallValidationErrorFunctions({
    "satisfy_dependencies_for:",
    "check_conflicts_for:",
    "verify_pkg_installable:",
    "opkg_download_pkg:",
    "opkg_verify_file:",
    "preinst_configure:",
    "check_data_file_clashes:"
});


IpkInstallStep::IpkInstallStep()
{
}

IpkInstallStep::~IpkInstallStep()
{
    LOG_DEBUG("IpkInstallStep::~IpkInstallStep called\n");
}

bool IpkInstallStep::proceed(Task *task)
{
    LOG_DEBUG("IpkInstallStep::proceed() called\n");
    m_parentTask = task;

    if(!checkStorageSize())
    {
        task->setError(ErrorInstall, APP_INSTALL_ERR_DISKFULL, "There is no available space to install App");
        task->proceed();
        return false;
    }

    pbnjson::JValue param = m_parentTask->getParam();
    bool verify = param["verify"].asBool();
    std::string ipkFile= param["ipkurl"].asString();
    bool allowDowngrade = param["allowDowngrade"].asBool();

    AppInstallerUtility::Result result =
            m_installerUtility.install(std::move(ipkFile), 0, verify, allowDowngrade, task->isAllowReInstall(), task->getInstallBasePath(),
                std::bind(&IpkInstallStep::cbInstallIpkProgress, this, _1),
                std::bind(&IpkInstallStep::cbInstallIpkComplete, this, _1));

    switch(result)
    {
    case AppInstallerUtility::FAIL:
        task->setError(ErrorInstall, APP_INSTALL_ERR_GENERAL, "unable to call ApplicationInstallerUtility");
        return false;
    case AppInstallerUtility::LOCKED:
        //pause(Task::SYSTEM);
        return true;
    default:
        break;
    }

    sync();

    task->setUnpacked(true);
    task->setStep(IpkInstallRequested);
    return true;
}

void IpkInstallStep::cbInstallIpkProgress(const char *str)
{
    std::vector<std::string> strs;
    boost::split(strs, str, boost::is_any_of(" \n"));

    if (strs.size() > 1)
    {
        std::string status = strs[1];
        std::string function = "";
        if (strs.size() > 2)
            function = strs[2];

        // strs[0] -> "status:"
        // strs[1] -> stage: starting, unpacking, verifying, installing, done
        //            error: "*"
        // strs[2] -> opkg function
        if ("starting" == status)
            m_parentTask->setStep(IpkInstallStarting);
        else if ("unpacking" == status)
            m_parentTask->setStep(IpkInstallUnpacking);
        else if ("verifying" == status)
            m_parentTask->setStep(IpkInstallVerifying);
        else if ("installing" == status)
            m_parentTask->setStep(IpkInstallCurrent);
        else if ("done" == status)
            m_parentTask->setStep(IpkInstallComplete);

        if (!function.empty())
        {
            if (std::find(opkgInstallValidationErrorFunctions.begin()
                , opkgInstallValidationErrorFunctions.end()
                , function) != opkgInstallValidationErrorFunctions.end())
                m_parentTask->setUnpacked(false);
        }
    }
}

void IpkInstallStep::cbInstallIpkComplete(int status)
{
    if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
    {
        std::string errorText;
        switch (WEXITSTATUS(status))
        {
        case AI_ERR_INTERNAL:
        case AI_ERR_INVALID_ARGS:
            errorText = "FAILED_INTERNAL_ERROR"; break;
        case AI_ERR_INSTALL_FAILEDUNPACK:
            errorText = "FAILED_CREATE_TMP"; break;
        case AI_ERR_INSTALL_NOTENOUGHTEMPSPACE:
            errorText = "FAILED_NOT_ENOUGH_TEMP_SPACE";  break;
        case AI_ERR_INSTALL_NOTENOUGHINSTALLSPACE:
            errorText = "FAILED_NOT_ENOUGH_INSTALL_SPACE";  break;
        case AI_ERR_INSTALL_TARGETNOTFOUND:
            errorText = "FAILED_PACKAGEFILE_NOT_FOUND"; break;
        case AI_ERR_INSTALL_BADPACKAGE:
            errorText = "FAILED_PACKAGEFILE_CORRUPT"; break;
        case AI_ERR_INSTALL_FAILEDVERIFY:
            errorText = "FAILED_VERIFY"; break;
        case AI_ERR_INSTALL_FAILEDIPKGINST:
            errorText = "FAILED_IPKG_INSTALL"; break;
        default:
            errorText = "FAILED_INSTALL"; break;
        }

        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, std::move(errorText));
        LOG_WARNING(MSGID_IPK_INSTALL_FAIL, 2,
            PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
            PMLOGKFV(STATUS, "%" PRId32, status),
                        " ");
    }

    m_parentTask->proceed();
}

bool IpkInstallStep::checkStorageSize()
{
    uint64_t availableSize = 0;
    uint64_t totalSize = 0;

    //Need to change when get size info from server.
    uint64_t requiredUnpackFileSize = m_parentTask->getUnpackFilesize();

    getStorageSize(m_parentTask->getInstallBasePath(), &availableSize, &totalSize);

    if (requiredUnpackFileSize > availableSize)
    {
        LOG_ERROR(MSGID_NOT_ENOUGH_STORAGE, 4,
            PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
            PMLOGKS(CALLER, m_parentTask->getAppInfo()["details"]["client"].asString().c_str()),
            PMLOGKFV(STORAGE_SIZE,"%" PRIu64, availableSize),
            PMLOGKFV(REQUIRED_SIZE, "%" PRIu64, requiredUnpackFileSize),
                        "");

        return false;
    }

    return true;
}

void IpkInstallStep::getStorageSize(const std::string &storagePath, uint64_t *availableSize, uint64_t *totalSize)
{
    struct statfs fsinfo;

    *availableSize = 0;
    *totalSize = 0;

    if (statfs( storagePath.c_str(), &fsinfo ) != 0)
    {
        LOG_WARNING(MSGID_STATFS_FAIL, 0, "statfs failed");
        return;
    }

    *totalSize = ( (uint64_t)fsinfo.f_blocks * fsinfo.f_bsize ) ;
    *availableSize = ( (uint64_t)fsinfo.f_bavail * fsinfo.f_bsize )  ;
}

