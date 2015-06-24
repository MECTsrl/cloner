# cloner
Application for copying and installing the software from/to the MECT panels.

The cloner manages all the software parts: Kernel, RootFS and local applications.

You build the cloner package from the developer's Linux PC also used for imx\_mect (LTIB). 

The build steps are:
 - rebuild by ltib the qt-everywhere package with the "-static" gcc option into the spec file;
 - run the following commands:
	export VERSION="xx"
	/usr/local/Trolltech/Qt-qvfb-version/bin/qmake -spec /usr/local/Trolltech/Qt-qvfb-version/mkspecs/qws/linux-arm-gnueabi-g++
	make
	./make_sysupdate.sh

The output is the "sysupdate\_cloner.sh" uuencoded script 

You need to copy the script to an empty USB pendrive and then boot the panel with the USB pendrive inserted: the cloner shall start instead of the panel applications (details into the pdf manual).
