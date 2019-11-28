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

#ifndef APPPACKAGE_H
#define APPPACKAGE_H

#include <deque>
#include <functional>
#include <glib.h>
#include <string>

//! AppPackage class helps extracting ipk file and parse items
class AppPackage {
public:
    const static int CONTROL = 0x01;
    const static int DATA    = 0x02;
    const static int DEBIAN  = 0x04;

    class Control {
    friend class AppPackage;

    public:
        Control()
            : m_installedSize(0)
        {}

        std::string getPackage() const;
        std::string getVersion() const;
        std::string getArchitecture() const;
        uint64_t getInstalledSize() const;

    private:
        std::string m_package;
        std::string m_version;
        std::string m_architecture;
        uint64_t m_installedSize;
    };

    //! Extract targetFile(*.ipk) to targetPath
    bool extract(std::string targetFile,
                 int targetItem,
                 std::string targetPath,
                 std::function<void (bool)> onExtract);

    //! Parse control file
    bool parseControl(std::string controlFilePath, AppPackage::Control &control);

    //! Save installed size in control file
    void saveInstalledSizeToControlFile(std::string controlFilePath, uint64_t unpackFileSize);

    //! Cancel processing
    void cancel();

    //! Check is canceled
    bool isCanceled() const;

protected:
    //! it's called when file extract complete
    static void cbExtractComplete(GPid pid, gint status, gpointer user_data);

    //! Extract item file
    bool extractOneItem();

private:
    std::string m_targetFile;
    std::string m_targetPath;
    std::deque<std::string> m_targetItemList;
    std::function<void (bool)> m_funcExtracted;

    bool m_canceled = false;
};

#endif
