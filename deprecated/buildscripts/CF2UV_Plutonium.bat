@echo off
goto :CopyStepFilters
==========================================================================================
CopyStepFilters
  Usage:
    This script copies the Plutonium specific Visual Studio debugger step filter file
    to the required folder.

  Notes:
   

==========================================================================================
:CopyStepFilters

echo %SystemRoot%\system32\xcopy "%~dp0..\visualizers\PuStepFilters.natstepfilter" %1 /s /y /d /r
%SystemRoot%\system32\xcopy "%~dp0..\visualizers\PuStepFilters.natstepfilter" %1 /s /y /d /r

@echo off