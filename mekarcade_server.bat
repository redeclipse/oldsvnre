@ECHO OFF

set RE_DIR=.
set RE_OPTIONS=
set RE_ARCH=x86

IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
    set RE_ARCH=amd64
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
    set RE_ARCH=amd64
)

:RETRY
IF EXIST bin\%RE_ARCH%\mekarcade_server.exe (
    start bin\%RE_ARCH%\mekarcade_server.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\bin\%RE_ARCH%\mekarcade_server.exe (
        pushd %RE_DIR%
        start bin\%RE_ARCH%\mekarcade_server.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_ARCH% == amd64 (
            set RE_ARCH=x86
            goto RETRY
        )
        echo Unable to find the MekArcade server binary
        pause
    )
)
