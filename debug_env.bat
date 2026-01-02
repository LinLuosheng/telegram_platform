@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
echo Platform=%Platform%
echo VSCMD_ARG_TGT_ARCH=%VSCMD_ARG_TGT_ARCH%
python -c "import os; print('Python Platform:', os.environ.get('Platform'))"
