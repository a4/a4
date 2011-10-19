#! /usr/bin/env bash

set -u -e

# Why: This generates a list of used boost headers so that we can make a 
#      stripped down library containing just what we use.

# HOW TO:
# 1) Run this script
# 2) Commit the updated `common/used_boost_headers`
# 3) Run `common/make_miniboost.sh`

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
grep -sh '#include <boost/' $(find ${SCRIPT_DIR}/../a4* -name '*.cpp' -or -name '*.h') | 
    sort -u |
    sed -r 's|^#include <boost/(.*).hpp>|\1|' > ${SCRIPT_DIR}/used_boost_headers
