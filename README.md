# cloner

Application for copying and installing the system software
from/to the MECT panels.  It handles all software parts of the
TPAC: kernel, root file sytem and local applications.

## Build flow

The cloner package is automatically built within the imx\_mect
flow as follows:
- rebuild by ltib the qt-everywhere package with the "-static"
  gcc option into the spec file;
- run the following commands:
  - export VERSION="xx"
  - /opt/Trolltech/bin/qmake -spec /opt/Trolltech/mkspecs/qws/linux-g++-mx
  - make
  - ./make\_sysupdate.sh

The output is the "sysupdate\_cloner.sh" "uuencoded" script.

## Usage

Copy the script on an empty USB pendrive.  Insert the pendrive
in the TPAC and reboot.  The TPAC will start in cloner mode
(check the user manual for details).
