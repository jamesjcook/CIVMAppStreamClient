/*
 * NOTICE: Amazon has modified this file from the original.
 * Modifications Copyright (C) 2013 Amazon.com, Inc. or its
 * affiliates. All Rights Reserved.
 *
 * Modifications are licensed under the Amazon Software License
 * (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *  OR CONDITIONS OF ANY KIND, express or implied. See the License for
 *  the specific language governing permissions and limitations under the License.
 *
 * Original source (only excerpts were taken from this file; don't expect to be able
 * to diff the two):
 *   https://android.googlesource.com/platform/cts/+/jb-mr2-release/tests/tests/media/src/android/media/cts/EncodeDecodeTest.java
 */

/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.amazon.appstream;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class HardwareDecoder {

    public static final String TAG = "HardwareDecoder";
    public static final int MAX_BUFFER_SIZE = 24000;

    MediaCodec mCodec = null;
    String mMIMEType;
    ByteBuffer[] mInputBuffers;
    ByteBuffer[] mOutputBuffers;
    private int mWidth,mHeight;

    private static HardwareDecoder mTheHardwareDecoder = null;
    private boolean mFirstFrame = true;
    private CodecInfo mDecoderInfo;

    public static HardwareDecoder getInstance() {
        return mTheHardwareDecoder;
    }

    private int mCLevel = 0;
    private int mCProfile = 0;

    // type "video/avc"
    public HardwareDecoder(String type, int width, int height) {
        mWidth = width;
        mHeight = height;
        mMIMEType = type;

        mDecoderInfo = CodecInfo.getSupportedFormatInfo(mMIMEType, mWidth, mHeight, false);
        if (mDecoderInfo == null) {
            Log.e(TAG, "Codec " + mMIMEType + "with " + mWidth + "," + mHeight + " not supported");
            throw new RuntimeException("Failed to find appropriate hardware codec");
        }

        switch (mDecoderInfo.mLevel) {
        default :
        case CodecProfileLevel.AVCLevel31 :
            mCLevel = 31; break;
        case CodecProfileLevel.AVCLevel32 :
            mCLevel = 32; break;
        case CodecProfileLevel.AVCLevel4 :
            mCLevel = 40; break;
        case CodecProfileLevel.AVCLevel41 :
            mCLevel = 41; break;
        case CodecProfileLevel.AVCLevel5 :
            mCLevel = 50; break;
        case CodecProfileLevel.AVCLevel51 :
            mCLevel = 50; break;
        }

        switch (mDecoderInfo.mProfile) {
        default:
        case CodecProfileLevel.AVCProfileBaseline :
            mCProfile = 66; break;
        case CodecProfileLevel.AVCProfileMain:
            mCProfile = 77; break;
        case CodecProfileLevel.AVCProfileExtended:
        case CodecProfileLevel.AVCProfileHigh:
            mCProfile = 100; break;
        case CodecProfileLevel.AVCProfileHigh444:
            mCProfile = 244; break;
        }

        Log.v(TAG, "Profile: " + mCProfile + ":" + mCLevel);
    }

    public int getLevel() {
        return mCLevel;
    }

    public int getProfile() {
        return mCProfile;
    }

    int mActiveBuffer = -1;
    int mActiveOutputBuffer = -1;

    int[] mReleaseQueue = null;
    int mNextToRelease = 0;
    int mNextToInsert = 0;

    boolean mInitialized = false;

    public boolean commitFrame() {
        if (mDoRender.get() > 0) {
            mDoRender.decrementAndGet();
            if (mOutputSurface.awaitNewImage()) mOutputSurface.drawImage();
            return true;
        }
        return false;
    }

    public int getGLTexture(int textureID) {
        initCodecFromGL(0);
        return mOutputSurface.getSurfaceID();
    }

    public int initCodecFromGL(int textureID) {
        mInitialized = true;
        // If we get here twice, close down the old one
        if (mCodec != null) {
            Log.w(TAG, "Releasing old codec");
            mCodec.flush();
            mCodec.release();
        }

        mCodec = MediaCodec.createDecoderByType(mMIMEType);

        int colorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface;

        // We avoid the device-specific limitations on width and height by using values that
        // are multiples of 16, which all tested devices seem to be able to handle.
        MediaFormat format = MediaFormat.createVideoFormat(mMIMEType, mWidth, mHeight);

        // Set some properties.  Failing to specify some of these can cause the MediaCodec
        // configure() call to throw an unhelpful exception.
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, colorFormat);

        // 30 FPS
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        Log.d(TAG, "format: " + format);

        mOutputSurface = new OutputSurface();

        // Create a MediaCodec for the decoder, just based on the MIME type.  The various
        // format details will be passed through the csd-0 meta-data later on.
        mCodec = MediaCodec.createDecoderByType(mMIMEType);
        mCodec.configure(format, mOutputSurface.getSurface(), null, 0);

        mCodec.start();

        mInputBuffers = mCodec.getInputBuffers();
        mOutputBuffers = mCodec.getOutputBuffers();

        return mOutputSurface.getSurfaceID();
    }

    public ByteBuffer getNextBuffer() {
        if (mCodec == null) {
            Log.w(TAG, "getNextBuffer called with codec NULL!!");
            return null;
        }

        try {
            mActiveBuffer = mCodec.dequeueInputBuffer(-1);
            if (mActiveBuffer >= 0) {
                return mInputBuffers[mActiveBuffer];
            } else {
                // if we're not finding a buffer, poll commitBuffer twice
                commitBuffer(0, 0);
                commitBuffer(0, 0);
                // And try once more
                mActiveBuffer = mCodec.dequeueInputBuffer(-1);
                if (mActiveBuffer >= 0) {
                    return mInputBuffers[mActiveBuffer];
                }
            }

            Log.w(TAG, "getNextBuffer found no input buffers !!");
        } catch (IllegalStateException e) {
            Log.w(TAG, "Not ready to dequeue an input buffer. Delaying. " + e.getMessage());
        }

        return null;
    }

    MediaCodec.BufferInfo mBufferInfo = new MediaCodec.BufferInfo();
    MediaFormat mFormat = null;
    private OutputSurface mOutputSurface;
    AtomicInteger mDoRender = new AtomicInteger(0);

    public void commitBuffer(int size, int time) {
        if (mCodec == null) {
            return;
        }

        if (mActiveBuffer >= 0) {

            int flags = 0;
            if (mFirstFrame) {
                flags = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                mFirstFrame = false;
            }
            // Look for headers that indicate that flags should be set.
            byte typeFlag = mInputBuffers[mActiveBuffer].get(3);

            if (typeFlag == 0x1) typeFlag = mInputBuffers[mActiveBuffer].get(4);

            switch (typeFlag) {
            case 0x68:
            case 0x67:
            case 0x6:
                flags |= MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                break;
            case 0x65 :
                flags |= MediaCodec.BUFFER_FLAG_SYNC_FRAME;
                break;
            }

            // don't print for normal frames
            if (typeFlag != 0x41) {
                Log.v(TAG, "Queuing NAL type " + Integer.toHexString(typeFlag));
            }

            mCodec.queueInputBuffer(mActiveBuffer, 0, size, 0, flags);
            mActiveBuffer = -1;
        }

        int out = mCodec.dequeueOutputBuffer(mBufferInfo, time);
        if (out == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
            mOutputBuffers = mCodec.getOutputBuffers();
            mReleaseQueue = null;

            // Recurse to see if we can get a buffer now.
            commitBuffer(size, time);
            return;
        } else if (out == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            mFormat = mCodec.getOutputFormat();
            Log.v(TAG, "Format " + mFormat);

            // Recurse to see if we can get a buffer now.
            commitBuffer(size, time);
            return;
        } else if (out == MediaCodec.INFO_TRY_AGAIN_LATER) {
            if (time != 0) {
                Log.v(TAG, "No output buffer ready");
            }

            return;
        }

        if (out >= 0) {
            if (mFormat == null) {
            	try	{
            		mFormat = mCodec.getOutputFormat();
            	} catch (IllegalStateException e)
            	{
            		// Ignore
            	}
            }

            // Pre-Jelly Bean MR2, Android fails to set
            // mBufferInfo.size, so we're forcing it to true here.
            mCodec.releaseOutputBuffer(out, true);

            // Flag the buffer to render
            mDoRender.incrementAndGet();
        } else {
            Log.w(TAG, "Unknown response to dequeueOutputBuffer: " + out);
        }
    }

    public int getOutputBufferWidth() {
        if (mFormat != null) {
            return mFormat.getInteger(MediaFormat.KEY_WIDTH);
        }
        return mWidth;
    }
    public int getOutputBufferHeight() {
        if (mFormat != null) {
            return mFormat.getInteger(MediaFormat.KEY_HEIGHT);
        }
        return mHeight;
    }
    public boolean getOutputBufferIsPlanar() {
        return (mFormat.getInteger(MediaFormat.KEY_COLOR_FORMAT) == CodecCapabilities.COLOR_FormatYUV420Planar);
    }
    public boolean getOutputBufferIsValid() {
        return mFormat != null;
    }
    public int getOutputBufferSliceHeight() {
        int slice = mFormat.getInteger("slice-height");
        if (slice == 0) {
            return getOutputBufferHeight();
        }
        return slice;
    }
    public ByteBuffer getOutputBuffer() {
        if (mActiveOutputBuffer >= 0) {
            return mOutputBuffers[mActiveOutputBuffer];
        }
        return null;
    }

    public void releaseOutputBuffer() {
        if (mActiveOutputBuffer >= 0) {
            mCodec.releaseOutputBuffer(mActiveOutputBuffer, true);
            mActiveOutputBuffer = -1;
        }
    }

    public void close() {
        if (mCodec != null) {
            mCodec.stop();
            mCodec.release();
            mCodec = null;
        }
    }
}
