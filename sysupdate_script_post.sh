
/etc/rc.d/init.d/autoexec stop
killall -q hmi fcrts autoexec

SIZE=`awk -F ',' '{print $1"x"$2}' /sys/class/graphics/fb0/virtual_size`
test -z $SIZE && SIZE=480x272

# tslib configuration
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/usr/etc/ts.conf
export TSLIB_CONSOLEDEVICE=none
export TSLIB_PLUGINDIR=$(pwd)/ts
export TSLIB_TSDEVICE=/dev/input/ts0

# Qt configuration
export POINTERCAL_FILE=$TSLIB_CALIBFILE
export QT_QWS_FONTDIR=$(pwd)/fonts
export QWS_MOUSE_PROTO=tslib:$TSLIB_TSDEVICE

# Use our libraries.
export LD_LIBRARY_PATH=$(pwd)

QWS_DISPLAY="Multi:VNC:0:size=${SIZE} Transformed:rot0" ./cloner -qws || ./cloner -qws

exit $?
