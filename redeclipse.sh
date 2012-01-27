#!/bin/sh
# RE_DATA should refer to the directory in which Red Eclipse data files are placed.
#RE_DATA=~/redeclipse
#RE_DATA=/usr/local/redeclipse
#RE_DATA=.
RE_DATA="$(cd "$(dirname "$0")" && pwd)"

# RE_BIN should refer to the directory in which Red Eclipse executable files are placed.
RE_BIN=${RE_DATA}/bin

# RE_OPTIONS contains any command line options you would like to start Red Eclipse with.
RE_OPTIONS="-r"

# SYSTEM_NAME should be set to the name of your operating system.
#SYSTEM_NAME=Linux
SYSTEM_NAME=`uname -s`

# MACHINE_NAME should be set to the name of your processor.
#MACHINE_NAME=i686
MACHINE_NAME=`uname -m`

case ${SYSTEM_NAME} in
Linux)
    SYSTEM_SUFFIX="_linux"
    ;;
FreeBSD)
    SYSTEM_SUFFIX="_freebsd"
    ;;
*)
    SYSTEM_SUFFIX="_unknown"
    ;;
esac

case ${MACHINE_NAME} in
i486|i586|i686)
    MACHINE_SUFFIX="_32"
    ;;
x86_64|amd64)
    MACHINE_SUFFIX="_64"
    ;;
*)
    SYSTEM_SUFFIX=""
    MACHINE_SUFFIX=""
    ;;
esac

#if [ -x ${RE_BIN}/reclient ]
#then
#    SYSTEM_SUFFIX=
#    MACHINE_SUFFIX=
#fi

if [ -x ${RE_BIN}/reclient_native ]
then
    SYSTEM_SUFFIX="_native"
    MACHINE_SUFFIX=""
fi

if [ -x ${RE_BIN}/reclient${SYSTEM_SUFFIX}${MACHINE_SUFFIX} ]
then
    cd ${RE_DATA} || exit 1
    exec ${RE_BIN}/reclient${SYSTEM_SUFFIX}${MACHINE_SUFFIX} ${RE_OPTIONS} "$@"
else
    echo "Your platform does not have a pre-compiled Red Eclipse client."
    echo -n "Would you like to build one now? [Yn] "
    read CC
    if [ "${CC}" != "n" ]; then
        cd ${RE_DATA}/src || exit 1
        make clean install
        echo "Build complete, please try running the script again."
    else
        echo "Please follow the following steps to build:"
        echo "1) Ensure you have the SDL, SDL image, SDL mixer, zlib, and OpenGL *DEVELOPMENT* libraries installed."
        echo "2) Change directory to src/ and type \"make clean install\"."
        echo "3) If the build succeeds, return to this directory and run this script again."
        exit 1
    fi
fi

