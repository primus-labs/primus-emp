#!/bin/bash
. ./scripts/_config.sh $1

bindir=${builddir}/emp-tool/bin
logfile=${logs_dir}/output_${target}.log

projs=(
  # ### perf test cases
  wperf
  prg
  hash
  prp
  mitccrh
  halfgate
  f2k

  # ### other test cases
  waesenc
  wclmul

  bit
  # ecc # some error mem oo
  # gen_circuit # file system
  int
  float
  to_bool
  aes_opt

  # ### other test cases need network
  # wnetio # todo
  # netio # todo
  # netio2 # todo
  # garble # todo
)

echo "" >${logfile}
if [ "${target}" = "wasm" ]; then
  cp -f ${scripts_dir}/entry.js ${bindir}/
  for proj in ${projs[@]}; do
    cd ${bindir}/
    if [ -f "test_${proj}.wasm" ]; then
      echo "run ${proj}"
      echo -e "\nrun ${proj}" >>${logfile}

      (time node entry.js $proj) >>${logfile} 2>&1
    fi
  done
else
  for proj in ${projs[@]}; do
    cd ${bindir}/
    if [ -f "test_${proj}" ]; then
      echo "run ${proj}"
      echo -e "\nrun ${proj}" >>${logfile}

      (time ./test_${proj}) >>${logfile} 2>&1
    fi
  done
fi
cd ${curdir}
