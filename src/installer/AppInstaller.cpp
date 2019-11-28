// Copyright (c) 2013-2019 LG Electronics, Inc.
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


#include <algorithm>
#include <functional>

#include "AppInstaller.h"
#include "base/LSUtils.h"
#include "base/JUtil.h"
#include "base/Utils.h"
#include "base/Logging.h"
#include "installer/AppInstallerErrors.h"
#include "installer/Task.h"
#include "settings/Settings.h"
#include "settings/StepSettings.h"

using namespace std::placeholders;

AppInstaller::AppInstaller()
    : m_installerDataPath(Settings::instance().getInstallerDataPath())
{
    StepSettings::instance().loadStepConfigure();
}

AppInstaller::~AppInstaller()
{
    finalize();
}

bool AppInstaller::initialize()
{
    return true;
}

void AppInstaller::finalize()
{
    m_mapTask.clear();
}

std::shared_ptr<Task> AppInstaller::install(const std::string& appId,
                                            const std::string& ipkUrl,
                                            pbnjson::JValue appInfo,
                                            int &errorCode,
                                            std::string &errorText,
                                            bool verify,
                                            bool allowDowngrade,
                                            std::string taskName)
{
    LOG_INFO(MSGID_APP_INSTALL_REQ, 2,
             PMLOGKS(APP_ID, appId.c_str()),
             PMLOGKS(CALLER, appInfo["details"]["client"].asString().c_str()),
             "");

    if (!isAppKnown(appId)) {
        pbnjson::JValue param = pbnjson::Object();
        param.put("id", appId);
        param.put("ipkurl", ipkUrl);
        param.put("appinfo", appInfo);
        param.put("verify", verify);
        param.put("downgrade", allowDowngrade);

        auto task = createTask(appId, taskName, param);
        if (!task) {
            errorCode = APP_INSTALL_ERR_INSTALL;
            errorText = "unable to initialize command";
            return nullptr;
        }

        task->run();
        return task;
    } else {
        errorCode = APP_INSTALL_ERR_INSTALL;
        errorText = "duplicate command while current command has not completed, ignoring";
    }

    return nullptr;
}

std::shared_ptr<Task> AppInstaller::remove(const std::string& appId,
                                           pbnjson::JValue appInfo,
                                           int& errorCode,
                                           std::string &errorText,
                                           bool verify)
{
    LOG_INFO(MSGID_APP_REMOVE_REQ, 2,
             PMLOGKS(APP_ID, appId.c_str()),
             PMLOGKS(CALLER, appInfo["details"]["client"].asString().c_str()),
             "");

    if (!isAppKnown(appId)) {
        pbnjson::JValue param = pbnjson::Object();
        param.put("id", appId);
        param.put("appinfo", appInfo);
        param.put("verify", verify);

        auto task = createTask(appId, "RemoveTask", param);
        if (!task) {
            errorCode = APP_REMOVE_ERR_REMOVE;
            errorText = "unable to initialize command";
            return nullptr;
        }

        task->setPackageId(appId);
        task->run();
        return task;
    } else {
        errorCode = APP_REMOVE_ERR_REMOVE;
        errorText = "duplicate command while current command has not completed, ignoring";
    }

    return nullptr;
}

void AppInstaller::onStartTask(const Task &task)
{
    //TODO : writePerformanceLog
}

void AppInstaller::onUpdateTask(const Task &task)
{
    pbnjson::JValue json = task.toJValue();

    if (json.isNull()) {
        return;
    }

    LSCaller caller = LSUtils::acquireCaller("com.webos.appInstallService");
    std::string key = std::string("status_") + task.getAppId();
    std::string payload = JUtil::toSimpleString(json);
    if (!caller.replySubscription(key.c_str(), payload.c_str())) {
        LOG_WARNING(MSGID_REPLY_SUBSCR_FAIL, 2,
                    PMLOGKS(KEY,key.c_str()),
                    PMLOGKS(PAYLOAD,payload.c_str()),
                    "reply subscription failed");

    }
    if (!caller.replySubscription("status", payload.c_str())) {
        LOG_WARNING(MSGID_REPLY_SUBSCR_FAIL,1,PMLOGKS(PAYLOAD,payload.c_str()),"reply subscription failed");
    }
}

void AppInstaller::onFinishTask(const Task &task)
{
    signalFinished(task);

    std::string id = task.getAppId();

    Utils::async([=] {
        releaseTask(id);
    });

    if (task.getName() == "InstallTask" && task.isError()) {
        if (task.isUnpacked()) {
            LOG_DEBUG("[AppInstaller]::onFinishTask: clean up packed files by install task error\n");
            std::string id = task.getAppId();
            Utils::async([=] {
                pbnjson::JValue appInfo = pbnjson::Object();
                pbnjson::JValue details = pbnjson::Object();

                details.put("client", "com.webos.appInstallService");
                appInfo.put("id", id);
                appInfo.put("details", details);

                int errorCode = 0;
                std::string errorText;

                remove(id, appInfo, errorCode, errorText);
            });
        }
    }
    //resumeAllTask(Task::SYSTEM); //TODO : need to check
}

bool AppInstaller::isAppKnown(const std::string& appId) {
    if (m_mapTask.find(appId) == m_mapTask.end())
        return false;
    return true;
}

bool AppInstaller::releaseTask(const std::string appId) {
    std::map< std::string, std::shared_ptr<Task> >::iterator it = m_mapTask.find(appId);
    if (it == m_mapTask.end())
        return false;

    m_mapTask.erase(it);
    return true;
}

std::shared_ptr<Task> AppInstaller::createTask(const std::string &id, const std::string &name, pbnjson::JValue param) {

    std::shared_ptr<Task> task = std::make_shared<Task>();
    param.put("name", name);

    task->initialize(param);

    if (name == "InstallTask") {
        task->prepareStep(StepSettings::instance().m_mapInstallSteps);
    } else if (name == "RemoveTask") {
        task->prepareStep(StepSettings::instance().m_mapRemoveSteps);
    } else {
        return nullptr;
    }

    task->signalStarted.connect(std::bind(&AppInstaller::onStartTask, this, _1));
    task->signalStatusChanged.connect(std::bind(&AppInstaller::onUpdateTask, this, _1));
    task->signalFinished.connect(std::bind(&AppInstaller::onFinishTask, this, _1));

    m_mapTask[id] = task;

    return task;
}

bool AppInstaller::contains(Task *task)
{
    if (m_mapTask.empty())
        return false;

    auto it = std::find_if(m_mapTask.begin(),
                           m_mapTask.end(),
                           [=] (const std::pair<std::string, std::shared_ptr<Task> > v) -> bool { return v.second.get() == task; });

    if (it == m_mapTask.end())
        return false;

    return true;
}

std::shared_ptr<Task> AppInstaller::get(const std::string &appId) {
    auto it = m_mapTask.find(appId);
    if (it == m_mapTask.end())
        return nullptr;
    return it->second;
}

pbnjson::JValue AppInstaller::toJValue() const {
    pbnjson::JValue array = pbnjson::Array();

    std::for_each(m_mapTask.begin(),
                  m_mapTask.end(),
                  [&array] (const std::pair<std::string, std::shared_ptr<Task> > v) -> void {
                      pbnjson::JValue json = v.second->toJValue();
                      array.append(json);
                  });

    return array;
}
