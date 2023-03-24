#!/bin/bash

#
# This script relies on the following directory hierarchy existing:
# /Volumes/devdrv (or /mnt/devdrv) (mount point)
# 	lib (dir)
#		MacOS (or Linux) (dir)
#			targets_m11.h (file) (edited to define "MACOS_m11" or "LINUX_m11")
#		medlib_m11.c (file)
#		medlib_m11.h (file)
#		medrec_m11.c (file)
#		medrec_m11.h (file)
#
# The library object code will be "/Volumes/devdrv/lib/MacOS/libmed_m11_mac.a" (or "/mnt/devdrv/lib/Linux/libmed_m11_lin.a").
# Link the library with "-L$LIBOBJ -lmed_m11$LIBSFX" in executables (see "compile.sh" for an example).
#



#########################
#### Start Edit Here ####
#########################

# compiler: "icc" or "clang"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m11.h file: "local" or "library"
TGT_FILE="library"

# binary: "x86", "arm", or "all"
BIN="all"

########################
#### Stop Edit Here ####
########################


# setup
if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/devdrv"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/devdrv"
	LIBSFX="_mac"
fi

if [ $CC = "icc" ]; then
	BIN="x86"
fi

LIBSRC=${DEVDRV}/lib
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

CC_OPT="-c -O3 -Wall -fms-extensions -Wno-microsoft-anon-tag"
if [ $CC = "icc" ]; then
	CC="/opt/intel/bin/icc"
	CC_OPT="${CC_OPT} -Qoption,cpp,--extended_float_types -static-intel"
	if [ $OS = "Linux" ]; then
		CC_OPT="${CC_OPT} -ipo"
	fi
elif [ $CC = "clang" ]; then
	CC="/usr/bin/clang"
fi

if [ $TGT_FILE = "library" ]; then
	TGTINC=$LIBOBJ
elif [ $TGT_FILE = "local" ]; then
	TGTINC=$PWD
fi


# delete old libraries
echo " "
CMD="rm *.a"
echo $CMD; echo " "
$CMD


# build libraries
LIBNAME="medlib_m11"
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi


LIBNAME="medrec_m11"
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi

CMD="ar rcs libmed_m11${LIBSFX}.a medlib_m11.o medrec_m11.o"
echo $CMD
$CMD
echo " "


# remove object files
CMD="rm *.o"
echo $CMD; echo " "
$CMD

