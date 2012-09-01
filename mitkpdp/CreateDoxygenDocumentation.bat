@echo off
REM run doxygen
IF NOT EXIST ..\mitkpdp-doc (
	MKDIR ..\mitkpdp-doc
)

D:\hachmann\Programs\doxygen\bin\doxygen Doxyfile
"..\mitkpdp-doc\html\index.html"