@echo off

set cpu=64

set vc2017="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars%cpu%.bat"
set vc2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars%cpu%.bat"
set vc2020="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars%cpu%.bat"



if exist %vc2020% (call %vc2020% & goto Build)
if exist %vc2019% (call %vc2019% & goto Build)
if exist %vc2017% (call %vc2017% & goto Build)

:Build
rc res\km.rc
cl -O2 -GS- -permissive- -nologo km64.c res\km.res -link -out:km%cpu%.exe

del *.obj

pause
