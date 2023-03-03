#!/bin/bash
curdir=$(pwd)
installdir=${curdir}/install

# uncompress opnessl
ossldir=${curdir}/openssl-1.1.1t
if [ ! -d "${ossldir}" ]; then
  tar -zxf openssl-1.1.1t.tar.gz
fi

# compile openssl
if [ ! -f "${installdir}/lib/libssl.a" ]; then
  cd ${ossldir}
  export CFLAGS="-O3 -msse -msse2 -msse3 -mavx -msimd128"
  emconfigure ./config -no-asm -no-shared --prefix=${installdir}
  # set CROSS_COMPILE=
  sed -i 's/CROSS_COMPILE=.*/CROSS_COMPILE=/' Makefile
  emmake make -j8 build_generated libssl.a libcrypto.a
  make install_dev
  cd ${curdir}
fi
