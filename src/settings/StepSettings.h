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

#ifndef STEP_SETTINGS_H
#define STEP_SETTINGS_H

#include <map>
#include <string>

#include "base/JUtil.h"
#include "base/Logging.h"
#include "base/Singleton.hpp"
#include "base/Utils.h"
#include "installer/InstallHistory.h"
#include "Settings.h"

class StepSettings: public Singleton<StepSettings> {
public:
    std::map<TaskStep, TaskStep> m_mapInstallSteps;
    std::map<TaskStep, TaskStep> m_mapRemoveSteps;

    /*! parse appisntalld configuration from appinstalld-conf file */
    bool loadStepConfigure();

protected:
friend class Singleton<StepSettings> ;
    StepSettings();
    virtual ~StepSettings();

};

#endif
