@ECHO OFF
REM
REM Run a programm with all path dependencies
REM

SET PROG=ExternalTool.sln
SET PROG_DIR=D:\hachmann\SVN\ThreeDEditing\Tools\AddThreeDTool\bin

SET ITK_AUTOLOAD_PATH=D:\hachmann\SVN\ThreeDEditing\Tools\AddThreeDTool\bin\Debug

PUSHD %PROG_DIR%
%PROG%
