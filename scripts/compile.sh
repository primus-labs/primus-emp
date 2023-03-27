#!/bin/bash
. ./scripts/_config.sh local

cd ${depsdir}
bash ./compile_openssl.sh
cd ${curdir}
ossl_root=${depsdir}/install

#
#
# ######################
for repo in ${emp_repos[@]}; do
  echo "compile ${repo}"
  cd ${curdir}
  if [ -d ${repo} ]; then
    mkdir -p ${builddir}/${repo}
    cd ${builddir}/${repo}

    cmake ${curdir}/${repo} \
      -DCMAKE_INSTALL_PREFIX=${builddir} \
      -DCMAKE_PREFIX_PATH=${builddir} \
      -DCMAKE_BUILD_TYPE=${build_type} \
      -DOPENSSL_INCLUDE_DIR=${ossl_root}/include \
      -DOPENSSL_SSL_LIBRARY=${ossl_root}/lib/libssl.a \
      -DOPENSSL_CRYPTO_LIBRARY=${ossl_root}/lib/libcrypto.a
    make -j8
    make install
  fi
done

cd ${curdir}
exit 0
