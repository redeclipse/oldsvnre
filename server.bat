@ECHO OFF

set RE_DIR=.
set RE_OPTIONS=
set RE_ARCH=x86

IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
    set RE_ARCH=x64
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
    set RE_ARCH=x64
)

:RETRY
IF EXIST bin\%RE_ARCH%\reserver.exe (
    start bin\%RE_ARCH%\reserver.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\bin\%RE_ARCH%\reserver.exe (
        pushd %RE_DIR%
        start bin\%RE_ARCH%\reserver.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_ARCH% == x64 (
            set RE_ARCH=x86
            goto RETRY
        )
        echo Unable to find the Red Eclipse server binary
        pause
    )
)
