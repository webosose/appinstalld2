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

#ifndef DATA_REMOVE_STEP_H
#define DATA_REMOVE_STEP_H

#include "Step.h"
#include "base/CallChain.h"
#include "base/Logging.h"
#include "installer/CallChainEventHandler.h"
#include "installer/DeviceId.h"
#include "installer/ServiceInstallerUtility.h"
#include "settings/Settings.h"

class Task;
class DataRemoveStep : public Step
{
public:
    //! Constructor
    DataRemoveStep();

    //! Destructor
    ~DataRemoveStep();

    virtual bool proceed(Task *task);

protected:
    bool onDataRemoved(pbnjson::JValue result, void *user_data);
};

#endif
