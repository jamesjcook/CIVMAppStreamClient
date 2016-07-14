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

#include <stdio.h>
#include <Windows.h>
#include "GDICapture.h"

void GDICapture::getWindowDimensions(HWND hWnd, int& horizontal, int& vertical)
{
    if ((hWnd == NULL) || (hWnd == GetDesktopWindow()))
    {
        // Note that this only works for physical resolution of monitor 0 on multi-monitor setups.
        // Getting the width of height of the desktop window will give us
        // the width/height of the bounding rectangle of all monitors (or virtual desktop)
        // which is not what we want

        HDC hScreen = GetDC(GetDesktopWindow());
        horizontal = GetDeviceCaps(hScreen, HORZRES);
        vertical = GetDeviceCaps(hScreen, VERTRES);
        ReleaseDC(GetDesktopWindow(), hScreen);
    }
    else
    {
        // Do this differently here because this is a window, not the desktop
        RECT rect;
        GetClientRect(hWnd, &rect);
        horizontal = rect.right - rect.top;
        vertical = rect.bottom - rect.top;
    }
}

int GDICapture::getBitsPerPixel(HWND hWnd)
{
    HDC hDC = GetDC(hWnd);
    return GetDeviceCaps(hDC, BITSPIXEL);
    ReleaseDC(hWnd, hDC);
}

GDICapture::GDICapture(HWND hWnd)
{
    _windowHandle = hWnd == NULL ? GetDesktopWindow() : hWnd;
    _windowDC = GetDC(_windowHandle);
    getWindowDimensions(_windowHandle, _windowWidth, _windowHeight);
    _windowDCBpp = getBitsPerPixel(_windowHandle);
    _windowDC = GetDC(_windowHandle);
    _memDC = CreateCompatibleDC(_windowDC);
    _compatibleBitmap = CreateCompatibleBitmap(_windowDC, _windowWidth, _windowHeight);
        
    _pixels = (unsigned char *)_aligned_malloc((_windowDCBpp / 8) * _windowWidth * _windowHeight, 32);

    printf("Window is %d pixels by %d pixels and %d bits per pixel\n", _windowWidth, _windowHeight, _windowDCBpp);
}

bool GDICapture::supported()
{
    // We only support these bit depths. See getCapturePixelFormat for a detailed
    // description of our pixelformat support
    return (_windowDCBpp == 32) || (_windowDCBpp == 24);
}

const unsigned char* GDICapture::capture()
{
    if (!captureInternal(_windowDC, _memDC, _compatibleBitmap, _pixels, false))
        return NULL;
    return _pixels;
}

GDICapture::~GDICapture()
{
    // VirtualFree(_pixels, (_desktopBpp / 8) * _desktopWidth * _desktopHeight, MEM_DECOMMIT | MEM_RELEASE);
    _aligned_free(_pixels);
    DeleteObject(_memDC);
    DeleteObject(_compatibleBitmap);
    ReleaseDC(NULL, _windowDC);
}

bool GDICapture::captureInternal(HDC hdcWindow, HDC hdcMemDC, HBITMAP hbmWindow, unsigned char* lpbitmap, bool save)
{
    // Select the compatible bitmap into the compatible memory DC.
    HBITMAP oldBitmap = (HBITMAP) SelectObject(hdcMemDC, hbmWindow);
    
    // Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC, 
               0,0, 
               _windowWidth, _windowHeight, 
               hdcWindow, 
               0,0,
               SRCCOPY
               ))
        return false;

    BITMAPFILEHEADER   bmfHeader;    
    BITMAPINFOHEADER   bi;
     
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = _windowWidth;    
    bi.biHeight = -_windowHeight; 
    bi.biPlanes = 1;    
    bi.biBitCount = _windowDCBpp;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd375448(v=vs.85).aspx
    DWORD stride = (_windowWidth * _windowDCBpp / 8);
    while (stride % sizeof(DWORD) != 0) // round up to next DWORD size
        stride++;
    DWORD dwBmpSize = stride * _windowHeight;

    HBITMAP rawBitmap2 = (HBITMAP) SelectObject(hdcMemDC, oldBitmap);

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, rawBitmap2, 0,
        (UINT)_windowHeight,
        lpbitmap,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    
    if (save)
    {
        // A file is created, this is where we will save the screen capture.
        HANDLE hFile = CreateFile("captureqwsx.bmp",
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);   
    
        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
    
        //Size of the file
        bmfHeader.bfSize = dwSizeofDIB; 
    
        //bfType must always be BM for Bitmaps
        bmfHeader.bfType = 0x4D42; //BM   
 
        DWORD dwBytesWritten = 0;
        WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
    
        //Close the handle for the file that was created
        CloseHandle(hFile);
    }

    return true;
}

CapturePixelFormat GDICapture::getCapturePixelFormat()
{
    // We assume component layout is always BGR(A), never RGB(A)
    // We also don't support 16 bit RGB565 formats
    if (_windowDCBpp == 32)
        return CAPTURE_PIXELFORMAT_B8G8R8A8;
    if (_windowDCBpp == 24)
        return CAPTURE_PIXELFORMAT_B8G8R8;

    return CAPTURE_PIXELFORMAT_UNKNOWN;
}
