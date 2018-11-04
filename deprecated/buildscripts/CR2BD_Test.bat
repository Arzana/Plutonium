@echo off
goto :CopyResources
==========================================================================================
CopyResources
  Usage:
    This script sets copies the required files for the project into the "Build Directory"

  Notes:
   

==========================================================================================
:CopyResources

echo %SystemRoot%\system32\xcopy "%~dp0..\assets\"*.* %1 /s /y /d /r 
%SystemRoot%\system32\xcopy "%~dp0..\assets\"*.* %1 /s /y /d /r 

@echo off
