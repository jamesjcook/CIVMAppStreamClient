/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#ifndef _AUDIO_H_XSTXEXAMPLES
#define _AUDIO_H_XSTXEXAMPLES

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601         // _WIN32_WINNT_WIN7
#endif // #ifndef _WIN32_WINNT
#define WIN32_LEAN_AND_MEAN         // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// Include the Windows 7 version of XAudio (2.7) from the DirectX SDK to create a Windows 7 compatible binary
#include "XAudio2.h"

// Define the FOURCC codes for wav file data chunks
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);

#endif  // _AUDIO_H_XSTXEXAMPLES
