#!/bin/bash
curdir=$(pwd)
installdir=${curdir}/install
uname_s=$(uname -s)

# openssl version
ov=3.0.8  # use todo(https://www.openssl.org/source/openssl-3.0.8.tar.gz)
ov=1.1.1t # use ./openssl-1.1.1t.tar.gz(https://www.openssl.org/source/openssl-1.1.1t.tar.gz)
ov=1.1.1  # use git@github.com:pado-labs/openssl.git -b ossl
echo "use openssl: ${ov}"

# uncompress opnessl
ossldir=${curdir}/openssl-${ov}

# compile openssl
cd ${ossldir}
./config --prefix=${installdir} -no-shared --debug
make -j8
make install_dev
cd ${curdir}
