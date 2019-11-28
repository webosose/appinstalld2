// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#include <cinttypes>

#include "base/Creator.h"
#include "base/Factory.h"
#include "base/Logging.h"
#include "base/Utils.h"
#include "step/AppCloseStep.h"
#include "step/DataRemoveStep.h"
#include "step/GetIpkInfoStep.h"
#include "step/IpkInstallStep.h"
#include "step/IpkParseStep.h"
#include "step/IpkRemoveStep.h"
#include "step/RemoveJailStep.h"
#include "step/RemoveStartStep.h"
#include "step/ServiceInstallStep.h"
#include "step/ServiceUninstallStep.h"
#include "Task.h"

typedef Factory<Step> StepFactory;

Task::Task()
        : m_errorCode(0),
          m_step(Unknown),
          m_finished(false),
          m_run(false),
          m_hasInstalledSizeWithControlFile(false),
          m_unpackFileSize(0),
          m_packFileSize(0),
          m_unpacked(false),
          m_allowReInstall(false),
          m_update(false),
          m_verify(false)
{
    LOG_DEBUG("Task::Task() called\n");
}

Task::~Task()
{
    LOG_DEBUG("Task::~Task() called\n");
}

bool Task::initialize(pbnjson::JValue param)
{
    m_appId = param["id"].asString();
    m_name = param["name"].asString();
    m_appInfo = param["appinfo"];
    m_verify = param["verify"].asBool();
    m_param = param.duplicate();

    StepFactory::instance().registerObject("IpkParseNeeded", CreatorUsingNew<IpkParseStep>());
    StepFactory::instance().registerObject("GetIpkInfoNeeded", CreatorUsingNew<GetIpkInfoStep>());
    StepFactory::instance().registerObject("AppCloseNeeded", CreatorUsingNew<AppCloseStep>());
    StepFactory::instance().registerObject("IpkInstallNeeded", CreatorUsingNew<IpkInstallStep>());
    StepFactory::instance().registerObject("ServiceInstallNeeded", CreatorUsingNew<ServiceInstallStep>());
    StepFactory::instance().registerObject("RemoveNeeded", CreatorUsingNew<RemoveStartStep>());
    StepFactory::instance().registerObject("RemoveJailNeeded", CreatorUsingNew<RemoveJailStep>());
    StepFactory::instance().registerObject("ServiceUninstallNeeded", CreatorUsingNew<ServiceUninstallStep>());
    StepFactory::instance().registerObject("IpkRemoveNeeded", CreatorUsingNew<IpkRemoveStep>());
    StepFactory::instance().registerObject("DataRemoveNeeded", CreatorUsingNew<DataRemoveStep>());

    return true;
}

bool Task::accept(pbnjson::JValue param)
{
    if (m_appId != param["id"].asString())
        return false;
    if (m_name != param["name"].asString())
        return false;

    return onAccept(param);
}

bool Task::onAccept(pbnjson::JValue param)
{
    return false;
}

bool Task::run()
{
    m_run = true;
    Utils::async([=] {
        //signalStarted(*this);
        LOG_DEBUG("Task::run\n");
        proceed();
    });

    return true;
}

void Task::setStep(TaskStep step, bool forceSet)
{
    if (m_finished)
        return;

    if (!forceSet && step == m_step)
        return;

    m_step = step;
    signalStatusChanged(*this);
}

void Task::setError(TaskStep status, int errorCode, std::string errorText)
{
    m_errorCode = errorCode;
    m_errorText = errorText;

    LOG_ERROR(MSGID_TASK_ERROR, 3,
              PMLOGKS(APP_ID, getAppId().c_str()),
              PMLOGKFV(LOGKEY_ERRCODE, "%d", errorCode),
              PMLOGKS(LOGKEY_ERRTEXT, errorText.c_str()),
              "");

    switch (status) {
        case ErrorInstall:
            //remove installDataPath for removing control files
            Utils::remove_dir(m_installBasePath + "/tmp/" + m_appId);
            break;
        default:
            break;
    }

    setStep(status, false);
}

std::vector<std::string>& Task::getServices()
{
    return m_services;
}

void Task::setPackageId(std::string packageId)
{
    m_packageId = packageId;
}

std::string Task::getPackageId() const
{
    return m_packageId;
}

void Task::setFilesize(uint64_t packFileSize, uint64_t unpackFileSize)
{
    m_packFileSize = packFileSize;
    m_unpackFileSize = unpackFileSize;
}

uint64_t Task::getUnpackFilesize() const
{
    return m_unpackFileSize;
}

void Task::setUnpackFilesize(uint64_t unpackFileSize)
{
    m_unpackFileSize = unpackFileSize;
}

void Task::setHasInstalledSizeWithControlFile(bool hasSizeInCtrFile)
{
    m_hasInstalledSizeWithControlFile = hasSizeInCtrFile;
}

void Task::setOriginAppInfo(pbnjson::JValue appInfo)
{
    m_originAppInfo = appInfo;
    if (!m_originAppInfo.isNull())
        m_update = true;
}

//! set InstallBasePath
void Task::setInstallBasePath(std::string installBasePath)
{
    m_installBasePath = installBasePath;
}

//! get InstallBasePath
std::string Task::getInstallBasePath() const
{
    return m_installBasePath;
}

void Task::setUnpacked(bool unpacked)
{
    m_unpacked = unpacked;
}

bool Task::isUnpacked() const
{
    return m_unpacked;
}

void Task::setAllowReInstall(bool allowReInstall)
{
    m_allowReInstall = allowReInstall;
}

bool Task::isAllowReInstall() const
{
    return m_allowReInstall;
}

bool Task::isUpdate() const
{
    return m_update;
}

pbnjson::JValue Task::getParam() const
{
    return m_param;
}

std::string Task::getAppId() const
{
    return m_appId;
}

pbnjson::JValue Task::getAppInfo() const
{
    return m_appInfo;
}

TaskStep Task::getStep() const
{
    return m_step;
}

int Task::getErrorCode() const
{
    return m_errorCode;
}

std::string Task::getErrorText() const
{
    return m_errorText;
}

std::string Task::getSender() const
{
    pbnjson::JValue json = m_appInfo["details"];
    return json["client"].asString();
}

std::string Task::getName() const
{
    return m_name;
}

pbnjson::JValue Task::toJValue() const
{
    pbnjson::JValue json = getAppInfo();
    json.put("statusValue", (int) getStep());

    pbnjson::JValue details = json["details"];
    if (details.isNull()) {
        details = pbnjson::Object();
        json.put("details", details);
    }

    if (!details["id"].isNull())
        details.remove("id");

    //common fields
    details.put("packageId", m_packageId);
    details.put("verified", m_verify);

    if (m_name == "InstallTask")
        details.put("installBasePath", m_installBasePath);

    TaskStep step = getStep();
    if (step == InstallComplete) {
        LOG_NORMAL(MSGID_APP_INSTALLED, 2,
                   PMLOGKS(APP_ID, getAppId().c_str()),
                   PMLOGKS(CALLER, json["details"]["client"].asString().c_str()),
                   "");
    } else if (step == RemoveComplete) {
        LOG_INFO(MSGID_APP_REMOVED, 2,
                 PMLOGKS(APP_ID, getAppId().c_str()),
                 PMLOGKS(CALLER, json["details"]["client"].asString().c_str()),
                 "");
    }

    //TODO : Move details definition to installHistory.h
    switch (step) {
        case IpkParseNeeded:
        case IpkParseRequested:
        case IpkParseComplete:
            details.put("state", "ipk parsing");
            break;

        case IpkInstallNeeded:
        case IpkInstallRequested:
            details.put("state", "installing");
            break;

        case IpkInstallStarting:
            details.put("state", "installing : start");
            break;

        case AppCloseNeeded:
        case AppCloseRequested:
        case AppCloseComplete:
            details.put("state", "app closing");
            break;

        case InstallComplete:
            details.put("state", "installed");
            details.put("progress", 100);
            break;

        case ErrorInstall:
            details.put("state", "install failed");
            details.put("errorCode", getErrorCode());
            details.put("reason", getErrorText());
            break;

        case IpkRemoveNeeded:
        case IpkRemoveRequested:
        case IpkRemoveStarted:
        case IpkRemoveComplete:
            details.put("state", "removing ipk");
            details.put("progress", 0);
            break;

        case ServiceUninstallNeeded:
        case ServiceUninstallRequested:
        case ServiceUninstallComplete:
            details.put("state", "removing service files");
            details.put("progress", 0);
            break;

        case RemoveComplete:
            details.put("state", "removed");
            details.put("progress", 100);
            break;

        case ErrorRemove:
            details.put("state", "remove failed");
            details.put("reason", getErrorText());
            break;

        case RemoveNeeded:
            details.put("state", "remove start");
            details.put("progress", 0);
            break;

        case RemoveStarted:
            details.put("state", "removing");
            details.put("progress", 0);
            break;

        case RemoveJailNeeded:
        case RemoveJailRequested:
        case RemoveJailComplete:
            details.put("state", "removing jail");
            details.put("progress", 0);
            break;

        case DataRemoveNeeded:
        case DataRemoveRequested:
        case DataRemoveComplete:
            details.put("state", "removing data");
            details.put("progress", 0);
            break;

        case Unknown:
        default:
            break;
    }

    return json;
}

bool Task::isError() const
{
    return (0 != m_errorCode);
}

bool Task::prepareStep(std::map<TaskStep, TaskStep> mapStep)
{
    m_mapStep = mapStep;
    return true;
}

void Task::finish()
{
    if (m_finished)
        return;

    signalFinished(*this);
    m_currentStep = nullptr;
    m_finished = true;
}

bool Task::proceed()
{
    TaskStepParser parser;
    if (m_finished)
        return true;

    std::map<TaskStep, TaskStep>::iterator it = m_mapStep.find(m_step);
    if (it == m_mapStep.end()) {
        LOG_WARNING(MSGID_STATUS_CHANGE_UNDEFINED, 2,
                    PMLOGKS(APP_ID, getAppId().c_str()),
                    PMLOGKFV(STATUS,"%d",m_step),
                    "");
        finish();
        return false;
    }

    setStep(it->second);
    bool success = onProceed(m_step);
    if (!success)
        finish();

    return success;
}

std::shared_ptr<Step> Task::createStep(TaskStep step)
{
    TaskStepParser parser;
    auto stepTask = StepFactory::instance().create(parser.enumToStringStep(step));
    return stepTask;
}

bool Task::onProceed(TaskStep step)
{
    TaskStepParser parser;
    bool success = false;
    m_currentStep = createStep(step);
    if (!m_currentStep) {
        return false;
    }

    success = m_currentStep->proceed(this);

    return success;
}

