#!/bin/bash

set -e
set -u

target_name=miniboost
miniboost_name=miniboost-1.47

miniboost_pack=$miniboost_name.tar.bz2
miniboost_url=http://www.ebke.org/$miniboost_pack

if test -d $target_name; then
  echo "Existing $target_name/ directory found, exiting..."
  exit -1
fi

echo "-------------------------------------------------"
echo "A4: Acquiring source of needed Boost Libraries..."
echo "-------------------------------------------------"

if ! test -d $miniboost_name; then
  if ! test -e $miniboost_pack; then
    echo "Downloading A4 miniboost library $miniboost_name..."
    if ! curl -f $miniboost_url > $miniboost_pack; then
      echo "FATAL: Download failed! :( "
      exit 1
    fi
  else
    echo "Using existing file $miniboost_pack."
  fi
  echo "Unpacking $miniboost_pack..."
  if ! tar -xj -f $miniboost_pack; then
    echo "FATAL: Could not unpack $miniboost_pack!"
    exit 1
  fi
  echo "Extracted $miniboost_name"
else
  echo "Existing $miniboost_name directory found!"
fi

prefix=$PWD/$target_name

pushd $miniboost_name

echo "--------------------------"
echo "A4: Bootstrapping Boost..."
echo "--------------------------"

if ! ./bootstrap.sh --prefix=$prefix; then
  echo "Boost bootstrap.sh failed! :("
  exit 1
fi

echo "---------------------------------------"
echo "A4: Compiling needed Boost Libraries..."
echo "---------------------------------------"

if ! ./b2 --buildid=-a4-mt-1_47 --prefix=$prefix; then
  echo "Boost compilation failed! :("
  exit 1
fi

echo "----------------------------------------------"
echo "A4: Local install of needed Boost Libraries..."
echo "----------------------------------------------"

if ! ./b2 --buildid=-a4-mt-1_47 --prefix=$prefix install; then
  echo "Boost installation failed! :("
  exit 1
fi
popd

rm $miniboost_name/ -rf

bash ./common/make_boost_la.sh

echo "------------------------------------"
echo "A4: Builtin Miniboost ready for use!"
echo "------------------------------------"
