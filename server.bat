@ECHO OFF

set SDL_STDIO_REDIRECT=0
set RE_DIR=.
set RE_OPTIONS=-gservlog.txt

IF EXIST bin\reserver.exe (
    start bin\reserver.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\bin\reserver.exe (
        pushd %RE_DIR%
        start bin\reserver.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        echo Unable to find the Red Eclipse server binary
        pause
    )
)
