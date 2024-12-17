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
  repo_dir=${curdir}/${repo}

  cd ${curdir}
  cp -f ${repo_dir}/run ${builddir}/${repo}/
  cd ${builddir}/${repo}/
  # make test
  ctest -V --rerun-failed --output-on-failure
done

cd ${curdir}
exit 0
