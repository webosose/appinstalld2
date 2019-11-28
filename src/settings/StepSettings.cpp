// Copyright (c) 2017-2019 LG Electronics, Inc.
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

#include "StepSettings.h"

StepSettings::StepSettings()
{
}

StepSettings::~StepSettings()
{
}

bool StepSettings::loadStepConfigure()
{
    std::string conf_path = Settings::instance().getConfPath();
    bool isJailMode = Settings::instance().isJailMode();

    LOG_DEBUG("[StepSettings]::loadStepConfigure : %s", conf_path.c_str());

    JUtil::Error error;
    pbnjson::JValue root = JUtil::parseFile(conf_path, "appinstalld-conf", &error);
    if (root.isNull()) {
        LOG_WARNING(MSGID_SETTINGS_PARSE_FAIL, 1, PMLOGKS("FILE", conf_path.c_str()), "");
        return false;
    }

    TaskStepParser parser;

    if (root["installSteps"].isArray()) {
        pbnjson::JValue installSteps = root["installSteps"];
        int arraySize = installSteps.arraySize();

        for (int i = 0; i < arraySize; i++) {
            std::string status, action;
            if (installSteps[i]["status"].asString(status) != CONV_OK)
                continue;
            if (installSteps[i]["action"].asString(action) != CONV_OK)
                continue;

            LOG_DEBUG("[StepStettings]::installSteps : status : %s, action : %s", status.c_str(), action.c_str());

            m_mapInstallSteps.insert(std::pair<TaskStep, TaskStep>(parser.stringToEnumStep(status), parser.stringToEnumStep(action)));
        }
    }

    std::string keepStatus;

    if (root["removeSteps"].isArray()) {
        pbnjson::JValue removeSteps = root["removeSteps"];
        int arraySize = removeSteps.arraySize();

        for (int i = 0; i < arraySize; i++) {
            std::string status, action;
            if (removeSteps[i]["status"].asString(status) != CONV_OK)
                continue;
            if (removeSteps[i]["action"].asString(action) != CONV_OK)
                continue;

            //Remove "RemoveJailer" Step when jailer is not supported
            if (!isJailMode) {
                if (action == "RemoveJailNeeded") {
                    keepStatus = status; //keep "RemoveStarted" status
                    continue;
                }

                if (status == "RemoveJailComplete") {
                    status = keepStatus;
                }
            }

            LOG_DEBUG("[StepStettings]::removeSteps : status : %s, action : %s", status.c_str(), action.c_str());
            m_mapRemoveSteps.insert(std::pair<TaskStep, TaskStep>(parser.stringToEnumStep(status), parser.stringToEnumStep(action)));
        }
    }
    return true;
}
