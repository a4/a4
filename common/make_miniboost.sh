#!/bin/sh
if test -z "$BOOST_ROOT"; then
  echo "Set BOOST_ROOT to your Boost directory!"
  echo "If you use your system Boost, export BOOST_ROOT=/usr/include"
  exit 1
fi
rm miniboost -rf && mkdir miniboost
bcp --boost=$BOOST_ROOT program_options foreach thread miniboost
cp -r $BOOST_ROOT/boost-build.jam  $BOOST_ROOT/boostcpp.jam $BOOST_ROOT/bootstrap.sh miniboost/
mkdir -p miniboost/tools/build
cp -r $BOOST_ROOT/tools/build/v2 miniboost/tools/build
