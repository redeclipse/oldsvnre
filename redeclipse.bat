@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
set BF_DIR=.
set BF_OPTIONS=-r

IF EXIST bin\reclient.exe (
    start bin\reclient.exe %BF_OPTIONS% %* 
) ELSE (
    IF EXIST %BF_DIR%\bin\reclient.exe (
        pushd %BF_DIR%
        start bin\reclient.exe %BF_OPTIONS% %*
        popd
    ) ELSE (
        echo Unable to find the Red Eclipse client
        pause
    )
)
