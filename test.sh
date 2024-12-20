#!/bin/bash
curdir=$(pwd)
builddir=${curdir}/build
# ######################
emp_repos=(
  emp-tool
  emp-ot
  emp-zk
)
# ######################

#
#
# ######################
for repo in ${emp_repos[@]}; do
  echo "test ${repo}"
  repo_build_dir=${builddir}/${repo}
  if [ -d "${repo_build_dir}" ]; then
    repo_dir=${curdir}/${repo}

    cd ${curdir}
    cp -f ${repo_dir}/run ${repo_build_dir}/
    cd ${repo_build_dir}/
    # make test
    ctest -V --rerun-failed --output-on-failure
  else
    echo "please compile first"
  fi
done

cd ${curdir}
exit 0
