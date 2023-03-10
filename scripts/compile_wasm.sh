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

    EMCC_CFLAGS="-O3 -sWASM=1 -sWASM_BIGINT"
    EMCC_CFLAGS="-O3 -sWASM=1"

    # sse* simd
    SIMD_CFLAGS=" -msimd128 -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx"
    EMCC_CFLAGS+=${SIMD_CFLAGS}

    # WebSocket/POSIX Socket
    SOCKET_FLAGS=" -lwebsocket.js -sPROXY_POSIX_SOCKETS -sUSE_PTHREADS -sPROXY_TO_PTHREAD"
    # EMCC_CFLAGS+=${SOCKET_FLAGS}

    # multi-thread
    PARALLEL_CFLAGS=" -pthread -sPTHREAD_POOL_SIZE=4 -sPROXY_TO_PTHREAD=1 -sEXIT_RUNTIME=1"
    # EMCC_CFLAGS+=${PARALLEL_CFLAGS}

    # memory (do not use `ALLOW_MEMORY_GROWTH` with pthread)
    MEM_CFLAGS=" -sALLOW_MEMORY_GROWTH"
    # EMCC_CFLAGS+=${MEM_CFLAGS}

    export EMCC_CFLAGS=${EMCC_CFLAGS}
    echo "EMCC_CFLAGS: ${EMCC_CFLAGS}"

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
