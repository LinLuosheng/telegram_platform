@echo on
cd /d %~dp0
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd Telegram
"C:\Program Files\Python310\python.exe" configure.py x64 -D TDESKTOP_API_ID=611335 -D TDESKTOP_API_HASH=d524b414d21f4d37f08684c1df41ac9c -D TDESKTOP_API_TEST=ON
if %errorlevel% neq 0 (
    echo Configuration failed with code %errorlevel%
    exit /b %errorlevel%
)
cd ..\out
cmake --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo Build failed with code %errorlevel%
    exit /b %errorlevel%
)
echo BUILD FINISHED SUCCESS

