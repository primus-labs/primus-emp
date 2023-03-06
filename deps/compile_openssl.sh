#!/bin/bash
curdir=$(pwd)
installdir=${curdir}/install

# openssl version
ov=3.0.8  # use todo(https://www.openssl.org/source/openssl-3.0.8.tar.gz)
ov=1.1.1t # use ./openssl-1.1.1t.tar.gz(https://www.openssl.org/source/openssl-1.1.1t.tar.gz)
ov=1.1.1  # use git@github.com:pado-labs/openssl.git -b ossl
echo "use openssl: ${ov}"

# uncompress opnessl
ossldir=${curdir}/openssl-${ov}
if [ ! -d "${ossldir}" ]; then
  if [ -f "openssl-${ov}.tar.gz" ]; then
    tar -zxf openssl-${ov}.tar.gz
  else
    echo "can not found openssl-${ov}"
    exit 1
  fi
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
