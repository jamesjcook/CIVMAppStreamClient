/*
 * Copyright 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *      http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

/**
 * @file jniBindings.h
 *
 * Android-specific Bindings.
 */

#ifndef _included_jniBindings_h
#define _included_jniBindings_h

// Android-specific JNI bindings
void * androidGetHWBuffer( int * size );
void androidReleaseHWBuffer( int size, int time );
bool androidHWDecodeAvailable();
void * androidGetOutputBuffer( int * size );
void androidReleaseOutputBuffer();
bool androidGetOutputBufferIsValid();

int androidGetLevel() ;
int androidGetProfile() ;

int androidGetOutputBufferWidth();
int androidGetOutputBufferSliceHeight();
int androidGetOutputBufferHeight();
bool androidGetOutputBufferIsPlanar();

#endif // _included_jniBindings_h
