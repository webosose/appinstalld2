// Copyright (c) 2015-2018 LG Electronics, Inc.
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

#include <mntent.h>
#include <stdio.h>
#include <string.h>

#include "settings/Settings.h"
#include "System.h"
#include "Utils.h"
#include "webospaths.h"

int System::kill(const unsigned int pid, bool recursive)
{
    std::string cmd;

    if (recursive)
        cmd = R"(kill -TERM $(pstree -p )" + Utils::toString(pid)
            + R"( | sed 's/(/\n(/g' | grep '(' )"
            + R"( | sed 's/(\(.*\)).*/\1/') )";
    else
        cmd = R"(kill -TERM )" + Utils::toString(pid);

    return ::system(cmd.c_str());
}
