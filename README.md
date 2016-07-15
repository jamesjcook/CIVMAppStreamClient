# CIVMAppStreamClient
Install Visual Studio 2013 Community 
Install CMake 
Get source code from github.
https://github.com/jamesjcook/CIVMAppStreamClient

Using Cmake configure the windows project in src/client/projects/windows/

Add to CMAKE_EXE_LINKER_FLAGS    “/NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:LIBCMT”

Compile for Release and 
Make a batch file with these contents(adjust to own environment).
8<---8<---8<---8<---8<---
::FFMPEG path
set PATH=C:\\src\\AppStreamSDK_1.6.0.135\\AppStreamSDK\\3rdparty\\windows\\ffmpeg\\bin\\x86;%PATH%
::XStxClientLibraryShared
set PATH=C:\\src\\AppStreamSDK_1.6.0.135\\AppStreamSDK\\bin\\windows\\x86;%PATH%
cd c:\\build\\AppStreamWindows_x86\\Release
8<---8<---8<---8<---8<---

The client should now start with this batch file.
See google doc for connection information.
