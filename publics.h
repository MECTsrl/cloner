#ifndef PUBLICS_H
#define PUBLICS_H

#include <QFile>
#include <QDir>

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>


#ifndef MECT_BUILD_MAJOR
#define MECT_BUILD_MAJOR 3
#endif
#ifndef MECT_BUILD_MINOR
#define MECT_BUILD_MINOR 3
#endif
#ifndef MECT_BUILD_BUILD
#define MECT_BUILD_BUILD 0
#endif

#define STR_LEN 256

#define ROOTFS_VERSION          "/rootfs_version"
#define SERIAL_FILE             "/etc/serial.conf"

#define MOUNTED_FS              "/tmp/cloner/"
#define MOUNTED_USB             "/tmp/mnt/"
#define CLONED_IMAGES_DIR       MOUNTED_USB"cloner/"
#define TMP_DIR                 "/tmp/tmp"
#define SIMPLE_DIR              MOUNTED_FS"Simples/"
#define MODEL_SYSUPDATE_FILE    "img_sysupdate-%1-%2.ext2"
#define MODEL_IMAGE_DIR         SIMPLE_DIR"%1_ms%2/"
#define LOCAL_FS_TAR            "localfs.tar"

#define SSH_KEY_DIR             MOUNTED_USB"keys/"
#define SSH_KEY_FILE            "/etc/dropbear/authorized_keys"
#define SSH_KEY_SMILY           "root@vpndev.vpn-smily.com"

#define OVPN_CERT_DIR           MOUNTED_USB"ovpn/"
#define OVPN_CONF_DIR           "/etc/openvpn/"
#define OVPN_COMMAND            "/etc/rc.d/init.d/openvpn %1"
#define OVPN_CERT_NEWFILE       OVPN_CERT_DIR"%1.ovpn"
#define OVPN_CERT_LOCFILE       OVPN_CONF_DIR"%1.ovpn"
#define OVPN_NO_ACTION          0
#define OVPN_CERT_REMOVE        1
#define OVPN_CERT_ADD           2

// Columns in SSH Keys
#define SSH_KEY_TYPE            0
#define SSH_KEY_VALUE           1
#define SSH_KEY_COMMENT         2

#define COLOR_OK                "color: LimeGreen;"
#define COLOR_FAIL              "color: OrangeRed;"

#define EXCLUDES_RFS            "excludes_rootfs.lst"
#define EXCLUDES_LFS            "excludes_localfs.lst"

#define REFRESH_MS              1000

#define START_SYSUPDATE         42

#define RETENTIVE_FILE          "retentive" // in "/local/"
#define INI_FILE                "flash/root/hmi.ini"
#define TAR_STORE_DIR           "flash/data/store"
#define RESTORE_IGNORE          0
#define RESTORE_RESET           1
#define RESTORE_RESTORE         2

#define ACTION_NONE             0
#define ACTION_BACKUP           1
#define ACTION_RESTORE          2
#define ACTION_OVPN             3

#define RAMDISK_SIZE            48

#define MOUNT_ROOTFS_RW         "mount -o remount,rw /"
#define MOUNT_ROOTFS_RO         "mount -o remount,ro /"


extern QString     szModel;                     // Target Model
extern QString     szSerialNO;                  // Target Serial #
extern QString     szTargetVersion;             // MS Target Version
extern QString     szQtVersion;                 // Qt Release
extern QString     szClonerVersion;             // Cloner App Version
extern QString     sysUpdateModelFile;          // Sysupdate Model File
extern QString     mfgToolsModelDir;            // MFG Tools Model Directory (contains Local)
extern QString     szAlphaStyle;                // Alphanumpad Stylesheet String
extern QString     szVPNOriginalFile;           // OpenVPN Certificate original file
extern QString     szVPNNewFile;                // OpenVPN Certificate new file
extern int         screen_width;                // Screen width  in Pixel
extern int         screen_height;               // Screen height in Pixel


#endif // PUBLICS_H
