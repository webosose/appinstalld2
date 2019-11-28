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

#ifndef INSTALLHISTORY_H
#define INSTALLHISTORY_H

#include <map>
#include <string>

#include "base/Utils.h"

namespace pbnjson {
    class JValue;
}

//! value for representing install status
enum TaskStep {
    Undefied = -1, Unknown, // 0
    IconDownloadNeeded, // 1
    IconDownloadRequested, // 2
    IconDownloadCurrent, // 3
    IconDownloadPaused, // 4
    IconDownloadComplete, // 5
    IpkDownloadNeeded, // 6
    IpkDownloadRequested,  // 7
    IpkDownloadCurrent, // 8
    IpkDownloadPaused, // 9
    IpkDownloadComplete, // 10
    IpkInstallNeeded, // 11
    IpkInstallRequested, // 12
    IpkInstallStarting,  // 13
    IpkInstallUnpacking, // 14
    IpkInstallVerifying, // 15
    IpkInstallCurrent, // 16
    IpkInstallComplete, // 17
    IpkRemoveNeeded, // 18
    IpkRemoveRequested,  // 19
    IpkRemoveStarted,  // 20
    IpkRemoveComplete,  // 21
    InstallCanceled,  // 22
    ErrorDownload, // 23
    ErrorInstall, // 24
    ErrorRemove, // 25
    Finish, // 26
    ServiceInstallNeeded, // 27
    ServiceInstallRequested, // 28
    ServiceInstallComplete, // 29
    InstallComplete, // 30
    RemoveComplete, // 31
    AppCloseNeeded, // 32
    AppCloseRequested, // 33
    AppCloseComplete, // 34
    IpkParseNeeded, // 35
    IpkParseRequested, // 36
    IpkParseComplete, // 37
    ServiceUninstallNeeded, //38
    ServiceUninstallRequested, //39
    ServiceUninstallComplete, //40
    RemoveNeeded, //41
    RemoveStarted, //42
    DataRemoveNeeded, //43
    DataRemoveRequested, //44
    DataRemoveComplete, //45
    RemoveJailNeeded, //46
    RemoveJailRequested, //47
    RemoveJailComplete, //48
    User = 0xFF,
    // add enums at the end to maintain database correctly
    //TODO : move below values to plugin if appinstalld get plugin model
    //DRM Related start. RO means 'Right Object'
    InstallRONeeded, //256
    InstallRORequested, //257
    InstallROComplete, //258
    UninstallRONeeded, //259
    UninstallRORequested, //260
    UninstallROComplete, //261
    //DRM Related end
    //SDX Related start
    GetIpkInfoNeeded, //262
    GetIpkInfoRequested, //263
    GetIpkInfoComplete, //264
    PostDoneNeeded, //265
    PostDoneRequested, //266
    PostDoneComplete, //267
    //SDX Related end
    //Signing Related start
    IpkVerifyNeeded, //268
    IpkVerifyRequested, //269
    IpkVerifyComplete, //270
//Signing Related end
};

class TaskStepParser {
    std::map<std::string, TaskStep> enumMap;
public:
    TaskStepParser()
    {
        enumMap["Unknown"] = Unknown;

        enumMap["IconDownloadNeeded"] = IconDownloadNeeded;
        enumMap["IconDownloadRequested"] = IconDownloadRequested;
        enumMap["IconDownloadCurrent"] = IconDownloadCurrent;
        enumMap["IconDownloadPaused"] = IconDownloadPaused;
        enumMap["IconDownloadComplete"] = IconDownloadComplete;

        enumMap["IpkDownloadNeeded"] = IpkDownloadNeeded;
        enumMap["IpkDownloadRequested"] = IpkDownloadRequested;
        enumMap["IpkDownloadCurrent"] = IpkDownloadCurrent;
        enumMap["IpkDownloadPaused"] = IpkDownloadPaused;
        enumMap["IpkDownloadComplete"] = IpkDownloadComplete;

        enumMap["IpkInstallNeeded"] = IpkInstallNeeded;
        enumMap["IpkInstallRequested"] = IpkInstallRequested;
        enumMap["IpkInstallStarting"] = IpkInstallStarting;
        enumMap["IpkInstallUnpacking"] = IpkInstallUnpacking;
        enumMap["IpkInstallVerifying"] = IpkInstallVerifying;
        enumMap["IpkInstallCurrent"] = IpkInstallCurrent;
        enumMap["IpkInstallComplete"] = IpkInstallComplete;

        enumMap["IpkRemoveNeeded"] = IpkRemoveNeeded;
        enumMap["IpkRemoveRequested"] = IpkRemoveRequested;
        enumMap["IpkRemoveStarted"] = IpkRemoveStarted;
        enumMap["IpkRemoveComplete"] = IpkRemoveComplete;
        enumMap["InstallCanceled"] = InstallCanceled;

        enumMap["ErrorDownload"] = ErrorDownload;
        enumMap["ErrorInstall"] = ErrorInstall;
        enumMap["ErrorRemove"] = ErrorRemove;

        enumMap["Finish"] = Finish;

        enumMap["ServiceInstallNeeded"] = ServiceInstallNeeded;
        enumMap["ServiceInstallRequested"] = ServiceInstallRequested;
        enumMap["ServiceInstallComplete"] = ServiceInstallComplete;

        enumMap["InstallComplete"] = InstallComplete;
        enumMap["RemoveComplete"] = RemoveComplete;

        enumMap["AppCloseNeeded"] = AppCloseNeeded;
        enumMap["AppCloseRequested"] = AppCloseRequested;
        enumMap["AppCloseComplete"] = AppCloseComplete;

        enumMap["IpkParseNeeded"] = IpkParseNeeded;
        enumMap["IpkParseRequested"] = IpkParseRequested;
        enumMap["IpkParseComplete"] = IpkParseComplete;

        enumMap["ServiceUninstallNeeded"] = ServiceUninstallNeeded;
        enumMap["ServiceUninstallRequested"] = ServiceUninstallRequested;
        enumMap["ServiceUninstallComplete"] = ServiceUninstallComplete;

        enumMap["RemoveNeeded"] = RemoveNeeded;
        enumMap["RemoveStarted"] = RemoveStarted;

        enumMap["DataRemoveNeeded"] = DataRemoveNeeded;
        enumMap["DataRemoveRequested"] = DataRemoveRequested;
        enumMap["DataRemoveComplete"] = DataRemoveComplete;

        enumMap["RemoveJailNeeded"] = RemoveJailNeeded;
        enumMap["RemoveJailRequested"] = RemoveJailRequested;
        enumMap["RemoveJailComplete"] = RemoveJailComplete;

        enumMap["InstallRONeeded"] = InstallRONeeded;
        enumMap["InstallRORequested"] = InstallRORequested;
        enumMap["InstallROComplete"] = InstallROComplete;

        enumMap["UninstallRONeeded"] = UninstallRONeeded;
        enumMap["UninstallRORequested"] = UninstallRORequested;
        enumMap["UninstallROComplete"] = UninstallROComplete;

        enumMap["GetIpkInfoNeeded"] = GetIpkInfoNeeded;
        enumMap["GetIpkInfoRequested"] = GetIpkInfoRequested;
        enumMap["GetIpkInfoComplete"] = GetIpkInfoComplete;

        enumMap["PostDoneNeeded"] = PostDoneNeeded;
        enumMap["PostDoneRequested"] = PostDoneRequested;
        enumMap["PostDoneComplete"] = PostDoneComplete;

        enumMap["IpkVerifyNeeded"] = IpkVerifyNeeded;
        enumMap["IpkVerifyRequested"] = IpkVerifyRequested;
        enumMap["IpkVerifyComplete"] = IpkVerifyComplete;
    }

    TaskStep stringToEnumStep(const std::string &strStep)
    {
        std::map<std::string, TaskStep>::const_iterator enumIter = enumMap.find(strStep);
        if (enumIter == enumMap.end())
            return Undefied;
        return enumIter->second;
    }

    std::string enumToStringStep(const TaskStep step)
    {
        for (auto it = enumMap.begin(); it != enumMap.end(); it++)
            if (it->second == step)
                return it->first;

        return std::string("");
    }
};

#endif // INSTALLHISTORY_H
