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

#ifndef TASK_H
#define TASK_H

#include <boost/signals2.hpp>
#include <map>
#include <pbnjson.hpp>

#include "InstallHistory.h"
#include "step/Step.h"

class Step;

class Task {
public:
    typedef enum {
        SUCCESS = 0,
        BUSY,
        FAIL
    } Result;

    typedef enum {
        NONE = 0,
        SYSTEM, // paused by opkg
        AUTH, // paused by authentication
        NETWORK, //paused by network
        DEVICE, //paused by device (such as usb)
        USER // paused by user
    } Reason;

    //! Constructor
    Task();

    //! Destructor
    ~Task();

    //! initalize task
    bool initialize(pbnjson::JValue param);

    //! check it's sharable with param
    bool accept(pbnjson::JValue param);

    //! Run task
    bool run();

    //! get appId
    std::string getAppId() const;

    //! get appInfo
    pbnjson::JValue getAppInfo() const;

    //! get install status
    TaskStep getStep() const;

    //! get last error code
    int getErrorCode() const;

    //! get last error text
    std::string getErrorText() const;

    //! to pbnjson::JValue
    pbnjson::JValue toJValue() const;

    // get task name
    std::string getName() const;

    //! get sender
    std::string getSender() const;

    //! signal for notify started
    boost::signals2::signal<void(const Task&)> signalStarted;

    //! signal for status changed
    boost::signals2::signal<void(const Task&)> signalStatusChanged;

    //! signal for notify finished
    boost::signals2::signal<void(const Task&)> signalFinished;

    //! get whether error has occured
    bool isError() const;

    //! prepare status map
    bool prepareStep(std::map<TaskStep, TaskStep> mapStep);

    //! proceed status
    bool proceed();

    //! change current status
    void setStep(TaskStep step, bool forceSet = false);

    //! set Ipk File path
    void setIpkFile(const std::string ipkFile);

    uint64_t getUnpackFilesize() const;

    void setUnpackFilesize(uint64_t unpackFileSize);

    //! get luna request param;
    pbnjson::JValue getParam() const;

    //! set error
    void setError(TaskStep step, int errorCode, std::string errorText);

    //! get app services
    std::vector<std::string>& getServices();

    //! set PackageId
    void setPackageId(std::string packageId);

    //! get PackageId
    std::string getPackageId() const;

    //! set packFileSize & unpackFileSize
    void setFilesize(uint64_t packFileSize, uint64_t unpackFileSize);

    //! set HasInstalledSizeWithControlFile
    void setHasInstalledSizeWithControlFile(bool hasSize);

    //! set origin AppInfo
    void setOriginAppInfo(pbnjson::JValue appInfo);

    //! set InstallBasePath
    void setInstallBasePath(std::string installBasePath);

    //! get InstallBasePath
    std::string getInstallBasePath() const;

    //! set unpacked
    void setUnpacked(bool unpacked);

    bool isUnpacked() const;

    //!set allowReInstall
    void setAllowReInstall(bool allowReInstall);

    //!get allowReInstall
    bool isAllowReInstall() const;

    bool isUpdate() const;

protected:
    typedef std::pair<TaskStep, TaskStep> make_status_pair;

    //! accept task
    bool onAccept(pbnjson::JValue param);

    //TODO :
    bool onProceed(TaskStep step);

    //! finish this task
    void finish();

    std::shared_ptr<Step> createStep(const TaskStep step);

private:
    struct Status {
        Status()
                : reason(NONE), status(false), requested(false)
        {
        }

        Reason reason;
        bool status;
        bool requested;
    };

    std::string m_appId;
    std::string m_name;
    pbnjson::JValue m_appInfo;

    int m_errorCode;
    std::string m_errorText;

    TaskStep m_step;
    std::map<TaskStep, TaskStep> m_mapStep;
    std::shared_ptr<Step> m_currentStep;

    bool m_finished;
    bool m_run;

    //luna-request param;
    pbnjson::JValue m_param;
    pbnjson::JValue m_originAppInfo;

    // set in IpkRemoveStep
    // get in KeyRemoveStep, DataRemoveStep
    std::vector<std::string> m_services;

    //TODO : Need to move task specific values
    //packageInfo
    std::string m_packageId;
    bool m_hasInstalledSizeWithControlFile;
    uint64_t m_unpackFileSize;
    uint64_t m_packFileSize;

    //TODO : Need to move
    std::string m_installBasePath;
    bool m_unpacked;
    bool m_allowReInstall;
    bool m_update;
    bool m_verify;

};

#endif
