#!/bin/bash
target=${1:-"local"} # local, wasm

curdir=$(pwd)
uname_s=$(uname -s)
echo "===== compile target: ${target}, on ${uname_s} ====="
#######################################################################
#######################################################################
builddir=${curdir}/build
if [ "${target}" = "wasm" ]; then
  builddir=${curdir}/build_${target}
fi
build_type=Debug
build_type=Release
installdir=${curdir}/install
depsdir=${curdir}/deps
scripts_dir=${curdir}/scripts
logs_dir=${builddir}/logs
mkdir -p ${logs_dir}
#######################################################################
#######################################################################
emp_repos=(
  emp-tool
  emp-ot
  emp-zk
)
