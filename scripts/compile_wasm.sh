#!/bin/bash
. ./scripts/_config.sh wasm

#
# Compile deps
# ######################
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

    export EMCC_CFLAGS="-O3 -s WASM=1 -s STANDALONE_WASM -s SIDE_MODULE=1"
    export EMCC_CFLAGS="-O3 -s WASM=1 -s STANDALONE_WASM"
    export EMCC_CFLAGS="-O3 -s WASM=1"
    export EMCC_CFLAGS="-O3 -s WASM=1 -s STANDALONE_WASM -s SIDE_MODULE=1"
    export EMCC_CFLAGS="-O3 -s STANDALONE_WASM"
    export EMCC_CFLAGS="-O3 -s WASM=1"
    export EMCC_CFLAGS="-O3 -s WASM=1 -s SIDE_MODULE=1"
    export EMCC_CFLAGS="-O0 -s WASM=1 -s STANDALONE_WASM -s WASM_BIGINT -s MODULARIZE=1"
    export EMCC_CFLAGS="-O0 -s WASM=1 -s STANDALONE_WASM -s WASM_BIGINT"
    export EMCC_CFLAGS="-O0 -s WASM=1 -s STANDALONE_WASM -s WASM_BIGINT -s EXPORTED_FUNCTIONS=_main"
    export EMCC_CFLAGS="-O3 -s WASM=1 -msimd128 -s EXPORTED_FUNCTIONS=_main,_run,_run1"
    export EMCC_CFLAGS="-O3 -s WASM=1 -msimd128 -s LEGALIZE_JS_FFI=0 -sERROR_ON_UNDEFINED_SYMBOLS=0"
    export EMCC_CFLAGS="-O3 -s WASM=1 -msimd128"
    # -DCMAKE_CROSSCOMPILING_EMULATOR=$NODE_HOME/bin/node
    emcmake cmake ${curdir}/${repo} \
      -DCMAKE_INSTALL_PREFIX=${builddir} \
      -DCMAKE_PREFIX_PATH=${builddir} \
      -DCMAKE_BUILD_TYPE=${build_type} \
      -DOPENSSL_INCLUDE_DIR=${ossl_root}/include \
      -DOPENSSL_SSL_LIBRARY=${ossl_root}/lib/libssl.a \
      -DOPENSSL_CRYPTO_LIBRARY=${ossl_root}/lib/libcrypto.a
    emmake make -j8
    make install
  fi
done

cd ${curdir}
exit 0
