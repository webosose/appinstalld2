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
#define PMTRACE_DEFINE

#include "Logging.h"

PmLogContext getPmLogContext()
{
    static PmLogContext logContext = 0;
    if (0 == logContext) {
        PmLogGetContext("AppInstallD", &logContext);
    }
    return logContext;
}

PmLogContext getPmLogHistoryContext()
{
    static PmLogContext loghistoryContext = 0;
    if (0 == loghistoryContext) {
        PmLogGetContext("AppInstallD.history", &loghistoryContext);
    }
    return loghistoryContext;
}
