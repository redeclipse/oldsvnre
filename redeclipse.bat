@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
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
IF EXIST %RE_ARCH%\bin\reclient.exe (
    start %RE_ARCH%\bin\reclient.exe %RE_OPTIONS% %*
) ELSE (
    IF EXIST %RE_DIR%\bin\%RE_ARCH%\reclient.exe (
        pushd %RE_DIR%
        start bin\%RE_ARCH%\reclient.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_ARCH% == x64 (
            set RE_ARCH=x86
            goto RETRY
        )
        echo Unable to find the Red Eclipse client
        pause
    )
)
