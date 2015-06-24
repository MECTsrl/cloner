# cloner
Application for backup and restore (copy and install) of MECT panel

The package is created into the linux PC where is created the imx28 rootfs

To build it is necessary:
 - rebuild with ltib the the qt-everywhere package with the static option;
 - run the following commands;
	export VERSION; /usr/local/Trolltech/Qt-qvfb-version/bin/qmake -spec /usr/local/Trolltech/Qt-qvfb-version/mkspecs/qws/linux-arm-gnueabi-g++
	make
	./make_sysupdate.sh

The result is a uuencoded sh script calles sysupdate_cloner.sh

To use it follow the step into the pdf manual
