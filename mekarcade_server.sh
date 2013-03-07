#!/bin/sh
# RE_PATH should refer to the directory in which MekArcade data files are placed.
#RE_PATH=~/mekarcade
#RE_PATH=/usr/local/mekarcade
RE_PATH="$(cd "$(dirname "$0")" && pwd)"

# RE_OPTIONS contains any command line options you would like to start MekArcade with.
RE_OPTIONS=""

# SYSTEM_NAME should be set to the name of your operating system.
#SYSTEM_NAME=Linux
SYSTEM_NAME=`uname -s`

# MACHINE_NAME should be set to the name of your processor.
#MACHINE_NAME=i686
MACHINE_NAME=`uname -m`

if [ -x ${RE_PATH}/bin/mekarcade_server_native ]
then
    SYSTEM_SUFFIX="_native"
    RE_ARCH=""
else
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
        RE_ARCH="x86/"
        ;;
    x86_64|amd64)
        RE_ARCH="amd64/"
        ;;
    *)
        SYSTEM_SUFFIX="_native"
        RE_ARCH=""
        ;;
    esac
fi

if [ -x ${RE_PATH}/bin/${RE_ARCH}mekarcade_server${SYSTEM_SUFFIX} ]
then
    cd ${RE_PATH} || exit 1
    exec ${RE_PATH}/bin/${RE_ARCH}mekarcade_server${SYSTEM_SUFFIX} ${RE_OPTIONS} "$@"
else
    echo "Your platform does not have a pre-compiled MekArcade server."
    echo -n "Would you like to build one now? [Yn] "
    read CC
    if [ "${CC}" != "n" ]; then
        cd ${RE_PATH}/src || exit 1
        make APPNAME=mekarcade clean install-server
        echo "Build complete, please try running the script again."
    else
        echo "Please follow the following steps to build:"
        echo "1) Ensure you have the zlib *DEVELOPMENT* libraries installed."
        echo "2) Change directory to src/ and type \"make clean install-server\"."
        echo "3) If the build succeeds, return to this directory and run this script again."
        exit 1
    fi
fi

