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

#ifndef LOGGING_H
#define LOGGING_H

#include <PmLogLib.h>

#include "PmTrace.h"

/* Logging for appInstallD main context ********
 * The parameters needed are
 * msgid - unique message id
 * kvcount - count for key-value pairs
 * ... - key-value pairs and free text. key-value pairs are formed using PMLOGKS or PMLOGKFV
 * e.g.)
 * LOG_CRITICAL(msgid, 2, PMLOGKS("key1", "value1"), PMLOGKFV("key2", "%s", value2), "free text message");
 **********************************************/
#define LOG_CRITICAL(msgid, kvcount, ...) \
        PmLogCritical(getPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_ERROR(msgid, kvcount, ...) \
        PmLogError(getPmLogContext(), msgid, kvcount,##__VA_ARGS__)

#define LOG_WARNING(msgid, kvcount, ...) \
        PmLogWarning(getPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_INFO(msgid, kvcount, ...) \
        PmLogInfo(getPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_NORMAL(msgid, kvcount, ...) \
        PmLogInfo(getPmLogContext(), "NL_" msgid, kvcount, ##__VA_ARGS__)

#define LOG_DEBUG(...) \
        PmLogDebug(getPmLogContext(), ##__VA_ARGS__)

/* Logging for history context ********/
#define LOG_HISTORY_CRITICAL(...) \
        PmLogCritical(getPmLogHistoryContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_HISTORY_ERROR(msgid, kvcount, ...) \
        PmLogError(getPmLogHistoryContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_HISTORY_WARNING(msgid, kvcount, ...) \
        PmLogWarning(getPmLogHistoryContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_HISTORY_INFO(msgid, kvcount, ...) \
        PmLogInfo(getPmLogHistoryContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_HISTORY_DEBUG(...) \
        PmLogDebug(getPmLogHistoryContext(), ##__VA_ARGS__)

#define LOG_PERFORMANCE(...) \
        PmtPerfLog(getPmLogContext(), ##__VA_ARGS__)

/**list of MSGID's pairs */
#define MSGID_SRVC_INIT_FAIL            "SRVC_INIT_FAIL"        /* Appinstalld service initialization failed. */
#define MSGID_CONF_FILE_ERR             "CONF_FILE_ERR"         /* Conf file data is empty to load settings */
#define MSGID_LSCALL_ERR                "LSCALL_ERR"            /* ls call error */

/** ServiceBase.cpp */
#define MSGID_SRVC_REG_FAIL             "SRVC_REG_FAIL"         /* Failed to register appinstalld service on luna-bus. */
#define MSGID_SRVC_CATEGORY_FAIL        "SRVC_CATGRY_FAIL"      /* Failed to register category on luna-bus */
#define MSGID_SRVC_CATEGORY_DATA_FAIL   "SRVC_CATGRY_DAT_FAIL"  /* Failed to set category data on luna-bus */
#define MSGID_SRVC_ATACH_FAIL           "SRVC_ATTACH_FAIL"      /* Failed to attach on luna-bus */
#define MSGID_SRVC_DETACH_FAIL          "SRVC_DETACH_FAIL"      /* Failed to detach from luna-bus */

/** AppInstaller.cpp */
#define MSGID_REPLY_SUBSCR_FAIL         "SUBSCRIBE_FAIL"        /* Reply Subscription Failed */

/** ServiceInstallerUtility.cpp */
#define MSGID_WRONG_SERVICEID           "WRONG_SERVICEID"       /* Service id should starts with app id */

/** InstallHistory.cpp */
#define MSGID_APPINSTALL_FAIL           "APPINSTALL_FAIL"       /* ApplicationInstallerUtility Execution failed - For Install */
#define MSGID_APPREMOVE_FAIL            "APPREMOVE_FAIL"        /* ApplicationInstallerUtility Execution failed - For remove */

/** InstallTask.cpp */
#define MSGID_IPK_DOWNLD_JSON_PARSE_ERR         "IPK_DM_JSONERR"            /* Ipk download JSON parse error */
#define MSGID_IPK_DOWNLD_ERR                    "IPK_DOWNLD_ERR"            /* Ipk download failed */
#define MSGID_INSTALL_IPK_IO_ERR                "IPK_INST_IOERR"            /* IO channel read error during IPK installation */
#define MSGID_INSTALL_RO_FAIL                   "INSTALL_RO_FAIL"           /* Install RO fail error */
#define MSGID_UNINSTALL_RO_FAIL                 "UNINSTALL_RO_FAIL"         /* Uninstall RO fail error */
#define MSGID_UNINSTALL_SERVICE_FAIL            "UNINSTALL_SERVICE_FAIL"    /* Uninstall Service fail error */
#define MSGID_IPK_DOWNLD_PAUS_ERR               "IPK_DOWNLD_PAUS_ERR"       /* Icon Download pause error */
#define MSGID_IPK_DOWNLD_CANCEL_ERR             "IPK_DOWNLD_CANCL_ERR"      /* Ipk Download cancel error */
#define MSGID_IPK_INSTALL_FAIL                  "IPK_INSTALL_FAIL"          /* Ipk install failed */
#define MSGID_PACKAGE_INFO                      "PACKAGE_INFO"              /* Ipk file info */
#define MSGID_IPK_VERIFY_ERR                    "IPK_VERIFY_ERR"            /* Ipk verification failed */
#define MSGID_IPK_SIZE_MISMATCH                 "IPK_SIZE_MISMATCH"         /* Ipk file size mismatch */
#define MSGID_POSTDONE_SKIP                     "POSTDONE_SKIP"             /* Skip post done event to server */
#define MSGID_INSTALL_RESTORED                  "INSTALL_RESTORED"          /* Install restored from db */
#define MSGID_IPK_DOWNLD_RESUME                 "IPK_DOWNLD_RESUME"         /* Ipk file download resume */
#define MSGID_SYMLINK_ERR                       "SYMLINK_ERR"               /* Make symbolic link error */

/** RemoveTask.cpp */
#define MSGID_REMOVE_IPK_IO_ERR         "IPK_RM_ERR"            /* IO channel read error during IPK uninstallation */
#define MSGID_IPK_REMOVE_INFO           "IPK_RM_INFO"           /* Ipk remove status info on completion */
#define MSGID_JAILER_REMOVE_FAIL        "JAILER_RM_ERR"         /* Removing jailer directory failed */

/** Task.cpp */
#define MSGID_TASK_ERROR                        "TASK_ERROR"            /* General error id from task */
#define MSGID_STATUS_CHANGE_UNDEFINED           "STATUS_UNDEF"          /* next step(status) is not defined */
#define MSGID_PREPARE_CURRENT_STATUS_FAIL       "STATUS_UNKNOWN"        /* prepare current status fail */
#define MSGID_COMPLETE_CURRENT_STATUS_FAIL      "STATUS_UNKNOWN"        /* complete current status fail */
#define MSGID_TASK_PAUSE                        "TASK_PAUSE"            /* Pause task */
#define MSGID_TASK_RESUME                       "TASK_RESUME"           /* Resume task */

/** AppinstallService.cpp */
#define MSGID_APP_INSTALLED              "APP_INSTALLED"                   /* Application install complete */
#define MSGID_APP_INSTALL_REQ            "APP_INSTALL_REQ"                 /* Application install request received*/
#define MSGID_APP_REMOVE_REQ             "APP_REMOVE_REQ"                  /* Application remove request received*/
#define MSGID_APP_REMOVED                "APP_REMOVED"                     /* Application removed */
#define MSGID_APP_INSTALL_PAUSED         "APP_INSTALL_PAUSED"              /* Application install paused */
#define MSGID_APP_INSTALL_RESUMED        "APP_INSTALL_RESUMED"             /* Application install resumed */
#define MSGID_APP_INSTALL_CANCEL_REQ     "APP_INSTALL_CANCEL_REQ"          /* Application cancel request received */
#define MSGID_APP_INSTALL_CANCELED       "APP_INSTALL_CANCELED"            /* Application install canceled */
#define MSGID_APP_INSTALL_ERR            "APP_INSTALL_ERR"                 /* Application install error */
#define MSGID_APP_REMOVE_ERR             "APP_REMOVE_ERR"                  /* Application remove error */
#define MSGID_APP_INSTALL_PAUSE_ERR      "APP_INSTALL_PAUSE_ERR"           /* Application install pause error */
#define MSGID_APP_INSTALL_RESUME_ERR     "APP_INSTALL_RESUME_ERR"          /* Application install resume error */
#define MSGID_APP_INSTALL_CANCEL_ERR     "APP_INSTALL_CANCEL_ERR"          /* Application install cancel error */

/** ASM.cpp */
#define MSGID_STORAGE_DEVICE_TYPE           "STORAGE_DEVICE_TYPE"       /** storage device type */
#define MSGID_STATFS_FAIL                   "STATFS_FAIL"               /** statfs() failed on storage path */
#define MSGID_INT_STORAGE_USED              "INT_STORAGE_USED"          /** Internal storage used*/
#define MSGID_EXT_STORAGE_USED              "EXT_STORAGE_USED"          /** External storage used since internal memory is full*/
#define MSGID_NOT_ENOUGH_STORAGE            "NOT_ENOUGH_STORAGE"        /** Internal or External Storage is Full*/
#define MSGID_DATA_STORAGE_FULL             "DATA_STORAGE_FULL"         /** Data folder is full */

/** AppPackage.cpp */
#define MSGID_APPUNPACK_IPK_FAIL         "APPUNPACK_IPK_FAIL"              /* Failed to unpack ipk file */
#define MSGID_APPUNPACK_TAR_FAIL         "APPUNPACK_TAR_FAIL"              /* Failed to unzip tar file */

/** Settings.cpp */
#define MSGID_SETTINGS_PARSE_FAIL        "SETTINGS_PARSE_FAIL" /** Failed to parse file */

#define MSGID_PMTRACE                "APPINSTALL_PMTRACE"

/** list of logkey ID's */
#define FILENAME            "file_name"
#define PATH                "path"
#define LOGKEY_ERRCODE      "error_code"
#define REASON              "reason"
#define LOGKEY_ERRTEXT      "error_text"
#define FUNCTION            "function"
#define PID                 "pid"
#define STATUS              "status"
#define CHLD_PID            "child_pid"
#define KEY                 "key"
#define PAYLOAD             "paylod"
#define APP_ID              "app_id"
#define SERVICE             "service"
#define TASK_NAME           "task_name"
#define DEVICE_NAME         "device_name"
#define DEVICE_STATUS       "device_status"
#define DEVICE_ID           "deviceId"
#define DRIVE_ID            "driveId"
#define STORAGE_SIZE        "storage_size"
#define INT_STORAGE_SIZE    "int_storage_size"
#define EXT_STORAGE_SIZE    "ext_storage_size"
#define DATA_STORAGE_SIZE   "data_storage_size"
#define REQUIRED_SIZE       "required_size"
#define CALLER              "caller"

extern PmLogContext getPmLogContext();
extern PmLogContext getPmLogHistoryContext();

#endif // LOGGING_H
