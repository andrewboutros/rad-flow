#!/bin/bash
usage()
{
   echo ""
   echo "Usage: $0 -v version"
   echo -e "\t-v The SystemC version to setup (default: 2.3.4)"
}

while getopts "v:h" opt
do
   case "$opt" in
      v) SYSTEMC_VERSION="$OPTARG" ;;
      h) usage
         exit 0
         ;;
      *) usage
         exit 1
         ;;
   esac
done

if [ -z "${SYSTEMC_VERSION}" ]; then
    usage
    exit 1
fi

sudo apt-get update && apt-get upgrade -y
sudo apt install build-essential cmake

wget -O systemc-$SYSTEMC_VERSION.tar.gz https://github.com/accellera-official/systemc/archive/refs/tags/$SYSTEMC_VERSION.tar.gz
tar xzf systemc-$SYSTEMC_VERSION.tar.gz
rm -f systemc-$SYSTEMC_VERSION.tar.gz
cd systemc-$SYSTEMC_VERSION
mkdir build
cd build
cmake ../ -DCMAKE_CXX_STANDARD=17
cmake --build .
