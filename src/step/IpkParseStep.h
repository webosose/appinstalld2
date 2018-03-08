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

#ifndef IPK_PARSE_STEP_H
#define IPK_PARSE_STEP_H

#include "Step.h"
#include "installer/InstallHistory.h"
#include "base/Logging.h"
#include "installer/AppPackage.h"
#include "installer/DeviceId.h"
#include "settings/Settings.h"

class Task;
class IpkParseStep : public Step
{
public:
    //! Constructor
    IpkParseStep();


    //! Destructor
    ~IpkParseStep();

    //! Destructor
    virtual bool proceed(Task *task);

protected:
    //! callback function
    void init(Task &task);

    void onPackageExtracted(bool result);

    void determineInstallPath();

private:
    AppPackage m_appPackage;
    std::string m_installDataPath;
    std::string m_ipkFile;

    bool m_verify;
    std::string m_appId;

};

#endif
