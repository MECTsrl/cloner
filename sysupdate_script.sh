#!/bin/sh

mount -orw,remount /tmp/mnt/

OUT_DIR=/tmp/ramdir

mkdir -p $OUT_DIR
if ! /bin/mount -t tmpfs -o size=32M tmpfs $OUT_DIR
then
	echo "ERROR: cannot mount ramdir"
	exit 1
fi
if ! uudecode $0
then
	echo "ERROR: corrupted data"
	exit 1
fi

cd $OUT_DIR
tar xzf $OUT_DIR/update.tar.gz
rm -f $OUT_DIR/update.tar.gz

chmod +x $OUT_DIR/cloner

# ts variable
export TSLIB_CONFFILE=/usr/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export TSLIB_TSDEVICE=/dev/input/ts0
export TSLIB_CONSOLEDEVICE=none
export TSLIB_CALIBFILE=/local/etc/sysconfig/pointercal

# qt variable
export QWS_MOUSE_PROTO=tslib:$TSLIB_TSDEVICE
export QT_QWS_FONTDIR=/usr/lib/fonts
export POINTERCAL_FILE=$TSLIB_CALIBFILE

/etc/rc.d/init.d/autoexec stop
killall -q hmi fcrts autoexec

#SIZE=`awk -F ',' '{print $1"x"$2}' /sys/class/graphics/fb0/virtual_size`
$OUT_DIR/cloner -qws -display "Multi: VNC:0:size=${SIZE} Transformed:rot0" || $OUT_DIR/cloner -qws
RET_VAL=$?

umount $OUT_DIR

exit $RET_VAL

