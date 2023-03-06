#!/bin/bash
. ./scripts/_config.sh $1

bindir=${builddir}/emp-tool/bin
logfile=${logs_dir}/output_${target}.log

projs=(
  waesenc
  wclmul

  # prg
  # hash
  # prp
  # bit
  # ecc # some error mem oo
  # int
  # float
  # gen_circuit
  # mitccrh
  # f2k
  halfgate
  # to_bool
  aes_opt

  # ### io
  # wnetio # todo
  # netio # todo
  # netio2 # todo
  # garble # todo
)

if [ "${target}" = "wasm" ]; then
  cp -f ${scripts_dir}/entry.js ${bindir}/

  echo "" >${logfile}
  for proj in ${projs[@]}; do
    cd ${bindir}/
    if [ -f "test_${proj}.wasm" ]; then
      echo "run ${proj}"
      echo -e "\nrun ${proj}" >>${logfile}

      begtime=$(date "+%Y-%m-%d %H:%M:%S.%N")
      echo "begtime:" $begtime >>${logfile}

      node entry.js $proj >>${logfile}

      endtime=$(date "+%Y-%m-%d %H:%M:%S.%N")
      echo "endtime:" $endtime >>${logfile}

      end_t=$(date --date "${endtime}" +%s.%N)
      beg_t=$(date --date "${begtime}" +%s.%N)
      delta=$(echo $end_t - $beg_t | bc)
      echo "elapsed(s): $delta" >>${logfile}
    fi
  done
else
  echo "" >${logfile}
  for proj in ${projs[@]}; do
    cd ${bindir}/
    if [ -f "test_${proj}" ]; then
      echo "run ${proj}"
      echo -e "\nrun ${proj}" >>${logfile}

      begtime=$(date "+%Y-%m-%d %H:%M:%S.%N")
      echo "begtime:" $begtime >>${logfile}

      ./test_${proj} >>${logfile}

      endtime=$(date "+%Y-%m-%d %H:%M:%S.%N")
      echo "endtime:" $endtime >>${logfile}

      end_t=$(date --date "${endtime}" +%s.%N)
      beg_t=$(date --date "${begtime}" +%s.%N)
      delta=$(echo $end_t - $beg_t | bc)
      echo "elapsed(s): $delta" >>${logfile}
    fi
  done
fi
cd ${curdir}
