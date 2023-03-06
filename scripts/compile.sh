#!/bin/bash
. ./scripts/_config.sh local

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
      -DCMAKE_BUILD_TYPE=${build_type}
    make -j8
    make install
  fi
done

cd ${curdir}
exit 0
