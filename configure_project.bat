@echo on
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd tdesktop
if not exist out mkdir out
cmake -B out -S . -D TDESKTOP_API_ID=2040 -D TDESKTOP_API_HASH=b18441a1ff607e10a989891a5462e627 -G "Visual Studio 17 2022" -A x64
