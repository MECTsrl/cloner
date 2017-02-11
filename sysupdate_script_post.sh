
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

cd "$WORKDIR"
QWS_DISPLAY="Multi:VNC:0:size=${SIZE} Transformed:rot0" ${CLONERDIR}/cloner -qws || ${CLONERDIR}/cloner -qws

exit $?
