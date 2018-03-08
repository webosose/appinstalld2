// Copyright (c) 2018 LG Electronics, Inc.
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

#ifndef DEVICEID_H
#define DEVICEID_H

#include <string>

namespace DeviceId
{
    const std::string INTERNAL_STORAGE            = "INTERNAL_STORAGE";
    const std::string INTERNAL_STORAGE_SYSTEM     = "INTERNAL_STROAGE_SYSTEM";
    const std::string INTERNAL_STORAGE_ALIAS      = "INTERNAL_STORAGE_ALIAS";
    const std::string INTERNAL_STORAGE_ALIASTMP   = "INTERNAL_STORAGE_ALIASTMP";

    bool isInternal(const std::string &deviceId);
}

#endif
