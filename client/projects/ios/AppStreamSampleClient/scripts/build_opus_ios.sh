#!/bin/sh 

#  Copyright 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved.
# 
#  Licensed under the Amazon Software License (the "License"). You may
#  not use this file except in compliance with the License. A copy of
#  the License is located at
# 
#  http://aws.amazon.com/asl/
# 
#  This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
#  OR CONDITIONS OF ANY KIND, express or implied. See the License for
#  the specific language governing permissions and limitations under
#  the License.

# build_opus_ios.sh 
# Builds libopus fat library for iphoneos and iphonesimulator

LIBNAME=opus
SRCDIR=opus-1.0.3

# set a build dir for intermediary library files
BASE_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DIR=${BASE_DIR}/${SRCDIR}/build
if [ ! -d "${BUILD_DIR}" ] ; then
    mkdir  "${BUILD_DIR}"
fi

echo Using build dir:            ${BUILD_DIR}

# Xcode / sdk path options -- to build system root 
# final path at the time of this writing (Xcode 4.6.3 Build version 4H1503 iphone 6.1 sdk ): for example:
# /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.1.sdk 
SYSROOT=`xcodebuild -sdk iphoneos -version Path | head -1`

echo Using system root:         ${SYSROOT}

# use clang as front end to compiler for cross compiling
export CC=`xcrun -sdk iphoneos -find clang`
export LD=`xcrun -sdk iphoneos -find ld`

# cflags common to all architectures
BASE_CFLAGS='-g  -pipe '
BASE_CONFIG='--enable-shared=no --enable-static=yes --disable-doc --with-pic'

echo using CC                   ${CC}
echo using LD                   ${LD}
echo using CFLAGS               ${BASE_CFLAGS}
echo using CONFIG               ${BASE_CONFIG}


cd ${SRCDIR}

######################################################
#####################  ARMv7  ########################
######################################################

unset CFLAGS
unset LDFLAGS
export CFLAGS="${BASE_CFLAGS} -arch armv7 -mcpu=cortex-a8 -mfpu=neon -isysroot ${SYSROOT} -mfpu=neon " 
export LDFLAGS="-arch armv7 -isysroot ${SYSROOT} " 

if [ -f Makefile ] ; then 
    make -s clean
fi    

./configure ${BASE_CONFIG} \
--host=armv7-apple-darwin  \
--prefix=${BUILD_DIR}/armv7 \
--with-sysroot="${SYSROOT}" &&
make &&
make install

rc=$?
if [[ $rc != 0 ]]; then
    echo -e "\n\n Error: Problem when building Opus for the armv7."
    exit 1
fi

######################################################
###################  ARMv7s ##########################
######################################################

unset CFLAGS
unset LDFLAGS

export CFLAGS="${BASE_CFLAGS} -arch armv7s -mcpu=cortex-a9 -mfpu=neon -isysroot ${SYSROOT} -miphoneos-version-min=6.0" 
export LDFLAGS="-arch armv7s -isysroot ${SYSROOT} -miphoneos-version-min=6.0" 

make -s clean
./configure ${BASE_CONFIG} \
--host=armv7s-apple-darwin  \
--prefix=${BUILD_DIR}/armv7s \
--with-sysroot="${SYSROOT}" &&
make &&
make install

rc=$?
if [[ $rc != 0 ]]; then
    echo -e "\n\n Error: Problem when building Opus for armv7s."
    exit 1
fi


######################################################
################# i386 (Simulator)  ##################
######################################################

# change sysroot to point to simulator sdk
SYSROOT=`xcodebuild -sdk iphonesimulator -version Path | head -1`
unset CFLAGS
unset LDFLAGS
export CC=`xcrun -sdk iphonesimulator -find clang`


export CFLAGS="${BASE_CFLAGS} -arch i386 -isysroot ${SYSROOT} -miphoneos-version-min=6.0"
export LDFLAGS="-arch i386 -isysroot ${SYSROOT} -miphoneos-version-min=6.0"

make -s clean
./configure ${BASE_CONFIG} \
--build=i386  \
--prefix=${BUILD_DIR}/i386 \
--with-sysroot="${SYSROOT}"  &&
make &&
make install

rc=$?
if [[ $rc != 0 ]]; then
    echo -e "\n\n Error: Problem when building Opus for simulator."
    exit 1
fi


######################################################
###################### LIPO  #########################
######################################################


cd ${BUILD_DIR}

if [ ! -d universal ] ; then 
    mkdir -p universal/lib;
fi 

xcrun -sdk iphoneos lipo -output universal/lib/lib${LIBNAME}.a -create \
-arch armv7 armv7/lib/lib${LIBNAME}.a \
-arch armv7s armv7s/lib/lib${LIBNAME}.a \
-arch i386 i386/lib/lib${LIBNAME}.a
rc=$?
if [[ $rc != 0 ]]; then
    echo -e "\n\n Error: Problem when lipo'ing fat Opus library."
    exit 1
fi


if [[ $? != 0 ]]; then
    echo "Opus Build failed."
    exit 1;
fi

echo "Opus build completed."
echo "Universal file created."
cd ..
xcrun -sdk iphoneos lipo -info ${BUILD_DIR}/universal/lib/lib${LIBNAME}.a
echo "Headers in: ${BUILD_DIR}/i386/include"



