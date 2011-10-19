#! /usr/bin/env bash

set -u -e

# Why: This generates a list of used boost headers so that we can make a 
#      stripped down library containing just what we use.

# HOW TO:
# 1) Run this script
# 2) Commit the updated `common/used_boost_headers`
# 3) Run `common/make_miniboost.sh`

hash ack 2>&- || { echo >&2 "I use \`ack\` to discover files, but it isn't available. Aborting."; exit 1; }

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

ack --no-heading '#include <boost/' ${SCRIPT_DIR}/../a4* | 
    cut -d: -f3- | 
    sort -u | 
    sed -r 's|^#include <boost/(.*).hpp>|\1|' > ${SCRIPT_DIR}/used_boost_headers
