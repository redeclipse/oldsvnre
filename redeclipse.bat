@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
set RE_DIR=.
set RE_OPTIONS=-glog.txt

IF EXIST bin\reclient.exe (
    start bin\reclient.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\bin\reclient.exe (
        pushd %RE_DIR%
        start bin\reclient.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        echo Unable to find the Red Eclipse client
        pause
    )
)
