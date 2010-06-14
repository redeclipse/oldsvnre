@ECHO OFF

set BF_DIR=.
set BF_OPTIONS=

IF EXIST bin\reserver.exe (
    bin\reserver.exe %BF_OPTIONS% %* 
) ELSE (
    IF EXIST %BF_DIR%\bin\reserver.exe (
        pushd %BF_DIR%
        bin\reserver.exe %BF_OPTIONS% %*
        popd
    ) ELSE (
        echo Unable to find the Red Eclipse server binary
        pause
    )
)
