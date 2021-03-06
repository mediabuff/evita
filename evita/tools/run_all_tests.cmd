@if "%_echo%"=="" echo off
call ../setenv.cmd
if "%outdir%"=="" goto no_outdir

setlocal
setlocal enabledelayedexpansion

set config=%1
if "%config%"=="" set config=debug

set exedir=%outdir%\%config%
set tests=base dom gfx layout regex text visuals
echo exedir=%exedir% tests=%tests%

set start=%TIME%

for %%x in (%tests%) do (
  echo Running %%x...
  set test_exe=%exedir%\evita_%%x_tests.exe
  if not exist !test_exe! goto no_test_exe

  set test_log=%TEMP%\evita_%%x_tests.log
  !test_exe! ^
      --single-process-tests ^
      --test-launcher-retry-limit=1 > !test_log! 2>&1
  echo    ERRORLEVEL=%ERRORLEVEL% !test_log!
  grep -E -e "PASSED|CRASHED|FAIL|Fatal|FATAL" !test_log!
)

set end=%TIME%

echo.
echo Start %start%
echo End %end%

endlocal
exit/b

:no_outdir
echo.
echo No OUTDIR
echo.
exit/b 1

:no_test_exe
echo.
echo No test exe %test_exe%
echo.
echo Available test exes are:
dir/b %exedir%\*.exe
exit/b 1
