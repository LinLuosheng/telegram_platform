@echo on
setlocal enabledelayedexpansion
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
"C:\Program Files\Python310\python.exe" -u Telegram\build\prepare\prepare.py silent > prepare_log.txt 2>&1
if %errorlevel% neq 0 (
    echo Prepare script failed with code %errorlevel%
    type prepare_log.txt
    exit /b %errorlevel%
)
echo Success
