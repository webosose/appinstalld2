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

#ifndef APP_INSTALLER_H
#define APP_INSTALLER_H

#include <map>
#include <memory>
#include <pbnjson.hpp>
#include <string>

#include "base/Singleton.hpp"
#include "InstallHistory.h"
#include "Task.h"

class Task;

/*! AppInstaller class is responsible for creating Task and managing them.
 * This class stores Task's updates in db and can restore them when it is needed.
 */
class AppInstaller : public Singleton<AppInstaller> {
public:
    //! Constructor
    AppInstaller();

    //! Destructor
    ~AppInstaller();

    //! open db file & initialize install map from db
    bool initialize();

    //! clear install map
    void finalize();

    //! Install app from local
    std::shared_ptr<Task> install(const std::string& appId,
                                  const std::string& ipkUrl,
                                  pbnjson::JValue appInfo,
                                  int &errorCode,
                                  std::string &errorText,
                                  bool verify = true,
                                  bool allowDowngrade = true,
                                  std::string taskName = "InstallTask");

    //! Remove app from given appId
    std::shared_ptr<Task> remove(const std::string &appId,
                                 pbnjson::JValue appInfo,
                                 int& errorCode,
                                 std::string& errorText,
                                 bool verify = true);

    //! Check contains task instance
    bool contains(Task *task);

    //! Get task instance
    std::shared_ptr<Task> get(const std::string &appId);

    //! cast Task instance to typename T and check it's valid
    template <typename T>
    static T safe_cast(void *data)
    {
        Task *task = reinterpret_cast<Task*>(data);
        if (!task)
            return NULL;

        if (!AppInstaller::instance().contains(task))
            return NULL;

        return static_cast<T>(task);
    }

    //! to pbnjson::JValue
    pbnjson::JValue toJValue() const;

    //! signal for notify started
    boost::signals2::signal<void (const Task&)> signalStarted;

    //! signal for notify finished
    boost::signals2::signal<void (const Task&)> signalFinished;

protected:
    //! It's called when Task started
    void onStartTask(const Task &task);

    //! It's called when Task's status has beed updated
    void onUpdateTask(const Task &task);

    //! It's called when Task finished
    void onFinishTask(const Task &task);

    //! It's called when DB read row
    //! check whether appId is known
    bool isAppKnown(const std::string& appId);

    //! create task from name with param
    std::shared_ptr<Task> createTask(const std::string &id, const std::string &name, pbnjson::JValue param);

    //! release accuired task
    bool releaseTask(const std::string appId);

    void writePerformanceLog(const Task &task);

private:
    std::string m_installerDataPath;

    std::map<std::string, std::shared_ptr<Task> > m_mapTask;
};

#endif
