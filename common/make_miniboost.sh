#! /bin/sh

set -u -e

# HOWTO:
# 1) Install boost/bcp
#  1a) If bcp isn't installed, build it and run "./b2 tools/bcp install"
# 2) Check that "common/used_boost_headers" is uptodate by running "common/discover_used_boost"
# 3) Make sure bcp is in your path and set BOOST_ROOT to boost's installation include directory
#  3a) or do "BOOST_ROOT=. PATH=dist/bin:$PATH ~/Projects/a4/common/make_miniboost.sh"
#      from boost's installation directory 

mboost=miniboost-1.47
if test -z "$BOOST_ROOT"; then
  echo "Set BOOST_ROOT to your Boost directory!"
  exit 1
fi
rm $mboost -rf && mkdir $mboost
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
USED_PACKAGES=$(cat ${SCRIPT_DIR}/used_boost_headers | xargs)
bcp --boost=$BOOST_ROOT $USED_PACKAGES $mboost
cp -r $BOOST_ROOT/boost-build.jam  $BOOST_ROOT/boostcpp.jam $BOOST_ROOT/bootstrap.sh $mboost/
mkdir -p $mboost/tools/build
cp -r $BOOST_ROOT/tools/build/v2 $mboost/tools/build
tar cjf $mboost.tar.bz2 $mboost
