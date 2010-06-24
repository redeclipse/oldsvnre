@ECHO OFF

set RE_DIR=.
set RE_OPTIONS=

IF EXIST bin\reserver.exe (
    bin\reserver.exe %RE_OPTIONS% %* 
) ELSE (
    IF EXIST %RE_DIR%\bin\reserver.exe (
        pushd %RE_DIR%
        bin\reserver.exe %RE_OPTIONS% %*
        popd
    ) ELSE (
        echo Unable to find the Red Eclipse server binary
        pause
    )
)
