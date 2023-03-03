#!/bin/bash
curdir=$(pwd)
builddir=${curdir}/build
bindir=${builddir}/bin
installdir=${curdir}/install
build_type=Debug
build_type=Release

#
#
# repos
repos=(
  emp-tool
  emp-ot
  emp-zk
)

#
#
# ######################
for repo in ${repos[@]}; do
  echo "compile ${repo}"
  cd ${curdir}
  if [ -d ${repo} ]; then
    mkdir -p ${builddir}/${repo}
    cd ${builddir}/${repo}

    cmake ${curdir}/${repo} \
      -DCMAKE_INSTALL_PREFIX=${builddir} \
      -DCMAKE_PREFIX_PATH=${builddir} \
      -DCMAKE_BUILD_TYPE=${build_type}
    make -j8
    make install
  fi
done

exit 0
