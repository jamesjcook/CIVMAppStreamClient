#!/bin/bash -e

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

# build_ffmpeg_ios.sh 
# Builds libopus fat library for iphoneos and iphonesimulator

SRCDIR=ffmpeg

echo "Building FFmpeg"

# Get the path to the iPhone and Simulator SDKs
# If using Xcode 5 or higher, this is an alternative command to use
# `xcrun -sdk iphonesimulator --show-sdk-path`
IOS_FULL_SDK_PATH=`xcodebuild -sdk iphoneos -version Path | head -1`
IOS_SIMULATOR_FULL_SDK_PATH=`xcodebuild -sdk iphonesimulator -version Path | head -1`

# Use xcrun to get the path to GCC
IOS_GCC_PATH=`xcrun -find -sdk iphoneos gcc`
IOS_SIMULATOR_GCC_PATH=`xcrun -find -sdk iphonesimulator gcc`


# Make sure the iOS SDK exists
if [ ! -d "$IOS_FULL_SDK_PATH" ]
then
    echo "Could not detect valid iOS SDK path. Tried: $IOS_FULL_SDK_PATH"
    exit 1
fi

#Make sure the Simulator SDK exists
if [ ! -d $IOS_SIMULATOR_FULL_SDK_PATH ]
then
    echo "Could not detect valid iOS Simulator SDK path. Tried: $IOS_SIMULATOR_FULL_SDK_PATH"
    exit 1
fi

#And make sure GCC exists (and is executable)
if [ ! -x $IOS_GCC_PATH ]
then
    echo "Could not find gcc when building FFmpeg. Tried: $IOS_GCC_PATH"
    exit 1
fi

# Last check for Simulator's GCC exists (and is executable)
if [ ! -x $IOS_SIMULATOR_GCC_PATH ]
then
    echo "Could not find Simulator gcc when building FFmpeg. Tried: $IOS_SIMULATOR_GCC_PATH"
    exit 1
fi

# set a build dir for intermediary library files
BASE_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DIR=${BASE_DIR}/${SRCDIR}/build
echo Using build dir:            ${BUILD_DIR}

cd ${SRCDIR}

#i386 (Simulator)
echo "Starting i386 build"
make clean || true
./configure \
--prefix=${BUILD_DIR}/i386 \
--disable-programs \
--disable-postproc \
--disable-swresample \
--disable-filters \
--disable-avfilter \
--disable-encoders \
--disable-decoders \
--enable-decoder=h264 \
--disable-parsers \
--enable-parser=h264 \
--disable-muxers \
--enable-muxer=h264 \
--disable-demuxers \
--enable-demuxer=h264 \
--enable-parser=h264 \
--disable-bsfs \
--disable-doc \
--disable-bzlib \
--enable-cross-compile \
--sysroot=${IOS_SIMULATOR_FULL_SDK_PATH} \
--target-os=darwin \
--cc=$IOS_SIMULATOR_GCC_PATH \
--extra-cflags="-arch i386 -miphoneos-version-min=6.0" \
--extra-ldflags="-arch i386 -isysroot ${IOS_SIMULATOR_FULL_SDK_PATH} -miphoneos-version-min=6.0" \
--arch=i386 \
--cpu=i386 \
--disable-asm \
--enable-pic &&
make &&
make install

if [[ $? != 0 ]];  then
    echo "Building i386 architecture of FFmpeg failed."
    exit 1;
fi


# ARMv7
echo "Starting ARMv7 build"
make clean || true
./configure \
--prefix=${BUILD_DIR}/armv7 \
--disable-programs \
--disable-postproc \
--disable-swresample \
--disable-filters \
--disable-avfilter \
--disable-encoders \
--disable-decoders \
--disable-parsers \
--disable-muxers \
--disable-demuxers \
--enable-decoder=h264 \
--enable-parser=h264 \
--enable-muxer=h264 \
--enable-demuxer=h264 \
--enable-parser=h264 \
--disable-bsfs \
--disable-doc \
--disable-bzlib \
--enable-cross-compile \
--sysroot="${IOS_FULL_SDK_PATH}" \
--target-os=darwin \
--cc=${IOS_GCC_PATH} \
--extra-cflags="-arch armv7 -mfpu=neon -miphoneos-version-min=6.0" \
--extra-ldflags="-arch armv7 -isysroot "${IOS_FULL_SDK_PATH}" -miphoneos-version-min=6.0" \
--arch=arm \
--cpu=cortex-a9 \
--enable-neon \
--disable-asm \
--enable-pic &&
make &&
make install

if [[ $? != 0 ]];  then
    echo "Building armv7 of FFmpeg failed."
    exit 1;
fi


#ARMv7s
echo "Starting ARMv7s build"
make clean || true
./configure \
--prefix=${BUILD_DIR}/armv7s \
--disable-programs \
--disable-postproc \
--disable-swresample \
--disable-filters \
--disable-avfilter \
--disable-encoders \
--disable-decoders \
--enable-decoder=h264 \
--disable-parsers \
--enable-parser=h264 \
--disable-muxers \
--enable-muxer=h264 \
--disable-demuxers \
--enable-demuxer=h264 \
--enable-parser=h264 \
--disable-bsfs \
--disable-doc \
--disable-bzlib \
--enable-cross-compile \
--sysroot="${IOS_FULL_SDK_PATH}" \
--target-os=darwin \
--cc=$IOS_GCC_PATH \
--extra-cflags="-arch armv7s -mfpu=neon -miphoneos-version-min=6.0" \
--extra-ldflags="-arch armv7s -isysroot "${IOS_FULL_SDK_PATH}" -miphoneos-version-min=6.0" \
--arch=arm \
--cpu=cortex-a9 \
--enable-neon \
--disable-asm \
--enable-pic &&
make &&
make install

if [[ $? != 0 ]];  then
    echo "Building armv7s of FFmpeg failed."
    exit 1;
fi


if [ ! -d ${BUILD_DIR}/universal ] ; then 
    mkdir -p ${BUILD_DIR}/universal/lib;
fi 

cd ${BUILD_DIR}/armv7/lib
# merge files into a fat binary using lipo
LIB_FILES="*.a"
for file in $LIB_FILES
do
    echo "Lipo'ing $file"
    xcrun -sdk iphoneos lipo -output ${BUILD_DIR}/universal/lib/$file  -create \
    -arch armv7 $file \
    -arch armv7s ${BUILD_DIR}/armv7s/lib/$file \
    -arch i386 ${BUILD_DIR}/i386/lib/$file
    echo "Universal $file created."
    xcrun -sdk iphoneos lipo -info ${BUILD_DIR}/universal/lib/$file
done

echo "FFmpeg build succeeded."

exit 0
