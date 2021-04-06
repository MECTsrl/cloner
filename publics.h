#ifndef PUBLICS_H
#define PUBLICS_H

#include "ntpclient.h"


#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMessageBox>

#ifndef MECT_BUILD_MAJOR
#define MECT_BUILD_MAJOR 3
#endif
#ifndef MECT_BUILD_MINOR
#define MECT_BUILD_MINOR 3
#endif
#ifndef MECT_BUILD_BUILD
#define MECT_BUILD_BUILD 0
#endif


#define ROOTFS_VERSION          "/rootfs_version"
#define MOUNTED_FS              "/tmp/cloner/"
#define MOUNTED_USB             "/tmp/mnt/"
#define CLONED_IMAGES_DIR       MOUNTED_USB"cloner/"
#define TMP_DIR                 "/tmp/tmp"
#define SIMPLE_DIR              MOUNTED_FS"Simples/"
#define MODEL_SYSUPDATE_FILE    "img_sysupdate-%1-%2.ext2"
#define MODEL_IMAGE_DIR         SIMPLE_DIR"%1_ms%2/"
#define LOCAL_FS_TAR            "localfs.tar"

#define COLOR_OK        "color: LimeGreen;"
#define COLOR_FAIL      "color: OrangeRed;"

#define EXCLUDES_RFS    "excludes_rootfs.lst"
#define EXCLUDES_LFS    "excludes_localfs.lst"

#define REFRESH_MS      1000

#define START_SYSUPDATE     42

#define RETENTIVE_IGNORE    0
#define RETENTIVE_RESET     1
#define RETENTIVE_RESTORE   2

#define ACTION_NONE         0
#define ACTION_BACKUP       1
#define ACTION_RESTORE      2

#define RAMDISK_SIZE        48

extern QString     szModel;                     // Target Model
extern QString     szTargetVersion;             // MS Target Version
extern QString     szClonerVersion;             // Cloner App Version
extern QString     sysUpdateModelFile;          // Sysupdate Model File
extern QString     mfgToolsModelDir;            // MFG Tools Model Directory (contains Local)
extern int         screen_width;                // Screen width  in Pixel
extern int         screen_height;               // Screen height in Pixel
extern NtpClient   *ntpclient;                  // NTP Interface


extern QStringList excludesRFSList;             // Root file system.
extern QStringList excludesLFSList;             // Local file system Exclude
// extern NtpClient   *ntpclient;                  // NTP Interface


#endif // PUBLICS_H
