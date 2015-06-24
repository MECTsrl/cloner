#!/bin/sh

ROOT_FS_DIR=${HOME}/ltib/rootfs
OUT_FILE_NAME=sysupdate_cloner.sh
OUT_DIR=/tmp/ramdir

echo ""

if [ ! -e cloner ]
then
	echo cannot find \"cloner\"
	return 1
fi

if [ ! -e bunzip2 ]
then
if [ -e ${ROOT_FS_DIR}/usr/bin/bunzip2 ]
then
	cp ${ROOT_FS_DIR}/usr/bin/bunzip2 .
else
	echo cannot find \"${ROOT_FS_DIR}/usr/bin/bunzip2\"
	return 1
fi
fi

if [ ! -e bzip2 ]
then
if [ -e ${ROOT_FS_DIR}/usr/bin/bzip2 ]
then
	cp ${ROOT_FS_DIR}/usr/bin/bzip2 .
else
	echo cannot find \"${ROOT_FS_DIR}/usr/bin/bzip2\"
	return 1
fi
fi

if [ ! -e flash_eraseall ]
then
if [ -e ${ROOT_FS_DIR}/usr/bin/flash_eraseall ]
then
	cp ${ROOT_FS_DIR}/usr/bin/flash_eraseall .
else
	echo cannot find \"${ROOT_FS_DIR}/usr/bin/flash_eraseall\"
	return 1
fi
fi


if [ ! -e kobs-ng ]
then
if [ -e ${ROOT_FS_DIR}/usr/bin/kobs-ng ]
then
	cp ${ROOT_FS_DIR}/usr/bin/kobs-ng .
else
	echo cannot find \"${ROOT_FS_DIR}/usr/bin/kobs-ng\"
	return 1
fi
fi

if [ -e ./update.tar.gz ]
then
	rm -f ./update.tar.gz
fi
if [ -e ./$OUT_FILE_NAME ]
then
	rm -f ./$OUT_FILE_NAME
fi

tar cf update.tar cloner flash_eraseall kobs-ng bzip2 bunzip2
gzip update.tar
cp sysupdate_script.sh $OUT_FILE_NAME
uuencode update.tar.gz $OUT_DIR/update.tar.gz >> $OUT_FILE_NAME
rm -f ./update.tar.gz
