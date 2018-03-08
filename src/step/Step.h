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

#ifndef STEP_H
#define STEP_H

#include "installer/Task.h"
#include "installer/AppInstaller.h"
#include "installer/InstallHistory.h"
#include "base/Logging.h"
#include "installer/AppInstallerErrors.h"
#include <string>
#include <functional>

class Task;
class Step
{
public:
    //! Constructor
    Step();

    //! Destructor
    virtual ~Step();

    //! Proceed each step
    virtual bool proceed(Task *task) = 0;

protected:

    std::string m_appId;

    Task* m_parentTask;

private:


};

#endif
