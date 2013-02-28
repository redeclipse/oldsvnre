@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
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
IF EXIST %RE_ARCH%\bin\mekclient.exe (
    start %RE_ARCH%\bin\mekclient.exe %RE_OPTIONS% %*
) ELSE (
    IF EXIST %RE_DIR%\bin\%RE_ARCH%\mekclient.exe (
        pushd %RE_DIR%
        start bin\%RE_ARCH%\mekclient.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        IF %RE_ARCH% == amd64 (
            set RE_ARCH=x86
            goto RETRY
        )
        echo Unable to find the MekArcade client
        pause
    )
)
