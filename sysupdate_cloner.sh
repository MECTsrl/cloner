#!/bin/sh

# Clean up dos2unix temp file.
sync

test -n "$0" || exit 1

FROMWD="$(pwd)"

WORKDIR="$(dirname $0)"
CLONERDIR="/tmp/cloner"
CLONEIMG=${WORKDIR}/img_cloner_@@CLONER_VERSION@@.ext2
CLONER=${CLONERDIR}/cloner
SYSUPDATE_CLONER=_ysupdate_img_@@CLONER_VERSION@@.sh

trap cleanup EXIT
cleanup()
{
    cd /

    sleep 2                     # Wait for UI to shut down

    losetup | grep -q . && losetup -d $(losetup | awk -F: '{ print $1}')

    sync
}

cd "$WORKDIR"
mount -o remount,ro /

test -d "$CLONERDIR" && rm -rf "$CLONERDIR"
mkdir -p "$CLONERDIR"
test -d "$CLONERDIR" || { echo "*** ERROR: cannot create image directory ${CLONERDIR}."; exit 1; }

losetup | grep -q . && losetup -d $(losetup | awk -F: '{ print $1}')
mount -o ro,loop "$CLONEIMG" "$CLONERDIR" || { echo "*** ERROR: cannot mount cloner image ${CLONEIMG}."; exit 1; }

test -x ${CLONER} || { echo "*** ERROR: missing cloner ${CLONER}."; exit 1; }

/etc/rc.d/init.d/autoexec stop
killall -q hmi fcrts autoexec

SIZE=$(sed 's/,/x/' /sys/class/graphics/fb0/virtual_size)
test -z $SIZE && SIZE=480x272

# tslib configuration
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/usr/etc/ts.conf
export TSLIB_CONSOLEDEVICE=none
export TSLIB_PLUGINDIR=${CLONERDIR}/ts
export TSLIB_TSDEVICE=/dev/input/ts0

# Qt configuration
export POINTERCAL_FILE=$TSLIB_CALIBFILE
export QT_QWS_FONTDIR=${CLONERDIR}/fonts
export QWS_MOUSE_PROTO=tslib:$TSLIB_TSDEVICE

# Use our libraries.
export LD_LIBRARY_PATH=${CLONERDIR}

QWS_DISPLAY="Multi:VNC:0:size=${SIZE} Transformed:rot0" ${CLONER} -qws || ${CLONER} -qws

RESULT=$?

if [ ${RESULT} -eq 42 ]
then
	umount ${CLONERDIR}
	dd if=/dev/zero of=/dev/fb0 
	if ! /bin/mount -o remount,rw / 2>/dev/null
	then
		echo "Cannot mount the file system in read/write." | tee -a $logfile
		if [ -x /usr/bin/splash ]
		then
			killall splash
			/usr/bin/splash -qws --text "<font color=\"red\"><h1>Cannot mount the filesystem in read/write mode.</h1></font>" --dimension 12 >/dev/null 2>&1
		fi
		/bin/umount $mntdir
		/bin/mount -oro,remount / 2>&1 | tee -a $logfile
		/etc/rc.d/init.d/autoexec start >/dev/null 2>&1
		exit 1
	fi
	/usr/bin/dos2unix ${SYSUPDATE_CLONER} 2>&1 | tee -a $logfile
	/bin/sh ${SYSUPDATE_CLONER} 2>&1 | tee -a $logfile
	/bin/mount -o remount,ro / 2>/dev/null
	#Sync for local
	/bin/sync	
else
	echo "Return:" ${RESULT}
	echo "Turn off the device and remove the USB Stick"
fi
exit $?
