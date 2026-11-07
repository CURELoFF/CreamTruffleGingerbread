@echo off
rem == dev.bat: launch Zed with MSVC env, emulate the 4 manual steps ==

rem == Your VS vcvar64.bat location ==
call "E:\VS2022\VC\Auxiliary\Build\vcvars64.bat" >NUL
if errorlevel 1 exit /b 1
rem == Your Project Location ==
cd /d "E:\!Codes\Cpp_toolchained_game\CreamTruffleGingerbread"
rem == Your IDE ==
start "Zed" zed .
rem == Remove this prompt window ==
exit
