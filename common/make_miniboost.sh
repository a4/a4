#!/bin/sh
mboost=miniboost-1.47
if test -z "$BOOST_ROOT"; then
  echo "Set BOOST_ROOT to your Boost directory!"
  echo "If you use your system Boost, export BOOST_ROOT=/usr/include"
  exit 1
fi
rm $mboost -rf && mkdir $mboost
bcp --boost=$BOOST_ROOT program_options foreach thread $mboost
cp -r $BOOST_ROOT/boost-build.jam  $BOOST_ROOT/boostcpp.jam $BOOST_ROOT/bootstrap.sh $mboost/
mkdir -p $mboost/tools/build
cp -r $BOOST_ROOT/tools/build/v2 $mboost/tools/build
tar cjf $mboost.tar.bz2 $mboost
