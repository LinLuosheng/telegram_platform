@echo on
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd tdesktop\out
cmake --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo Build failed with code %errorlevel%
    exit /b %errorlevel%
)
echo Build success
