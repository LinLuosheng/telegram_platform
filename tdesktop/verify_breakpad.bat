@echo on
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
"C:\Program Files\Python310\python.exe" -u Telegram\build\prepare\prepare.py silent > prepare_retry.log 2>&1
if %errorlevel% neq 0 (
    echo Prepare failed
    exit /b %errorlevel%
)
echo Prepare success
