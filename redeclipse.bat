@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
set RE_DIR=.
set RE_OPTIONS=-glog.txt
set RE_BINARY=bin

IF %PROCESSOR_ARCHITECTURE% == amd64 (
   set RE_BINARY=bin64
)

IF %PROCESSOR_ARCHITEW6432% == amd64 (
   set RE_BINARY=bin64
)

:RETRY
IF EXIST %RE_BINARY%\reclient.exe (
    start %RE_BINARY%\reclient.exe %RE_OPTIONS% %*
) ELSE (
    IF EXIST %RE_DIR%\%RE_BINARY%\reclient.exe (
        pushd %RE_DIR%
        start %RE_BINARY%\reclient.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_BINARY% == bin64 (
            set RE_BINARY=bin
            goto RETRY
        )
        echo Unable to find the Red Eclipse client
        pause
    )
)
