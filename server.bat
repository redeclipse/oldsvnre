@ECHO OFF

set RE_DIR=.
set RE_OPTIONS=-gservlog.txt
set RE_BINARY=bin

IF %PROCESSOR_ARCHITECTURE% == amd64 OR %PROCESSOR_ARCHITEW6432% == amd64 (
   set RE_BINARY=bin64
)

:RETRY
IF EXIST %RE_BINARY%\reserver.exe (
    start %RE_BINARY%\reserver.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\%RE_BINARY%\reserver.exe (
        pushd %RE_DIR%
        start %RE_BINARY%\reserver.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_BINARY% == 64 (
            set RE_BINARY=bin
            goto RETRY
        )
        echo Unable to find the Red Eclipse server binary
        pause
    )
)
