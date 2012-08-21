#!/bin/bash

miniboost_name=miniboost-1.50

toolset=
if test "$CXX" = "clang" -o "$CXX" = "clang++"; then
  toolset=--with-toolset=clang
fi

target_name=miniboost
builddir_name=${miniboost_name}
if test "x$boostsuffix" != "x"; then
  target_name=miniboost-${boostsuffix}
  builddir_name=${miniboost_name}-${boostsuffix}
fi

set -e
set -u

miniboost_pack=$miniboost_name.tar.bz2
miniboost_url=http://www.ebke.org/$miniboost_pack

if test -d $target_name; then
  echo "Existing $target_name/ directory found, exiting..."
  exit -1
fi

echo "-------------------------------------------------"
echo "A4: Acquiring source of needed Boost Libraries..."
echo "-------------------------------------------------"

if ! test -d $builddir_name; then
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
  mkdir -p $builddir_name
  if ! tar -xj -C $builddir_name --strip-components 1 -f $miniboost_pack; then
    echo "FATAL: Could not unpack $miniboost_pack!"
    exit 1
  fi
  echo "Extracted $miniboost_name"
else
  echo "Existing $miniboost_name directory found!"
fi

prefix=$PWD/$target_name

pushd $builddir_name

echo "--------------------------"
echo "A4: Bootstrapping Boost..."
echo "--------------------------"

if ! ./bootstrap.sh $toolset --prefix=$prefix; then
  echo "Boost bootstrap.sh failed! :("
  exit 1
fi

echo "---------------------------------------"
echo "A4: Compiling needed Boost Libraries..."
echo "---------------------------------------"

if ! ./b2 --buildid=-a4-mt-1_50 --prefix=$prefix cflags=-fPIC $@; then
  echo "Boost compilation failed! :("
  exit 1
fi

echo "----------------------------------------------"
echo "A4: Local install of needed Boost Libraries..."
echo "----------------------------------------------"

if ! ./b2 --buildid=-a4-mt-1_50 --prefix=$prefix install; then
  echo "Boost installation failed! :("
  exit 1
fi
popd

#rm $builddir_name/ -rf

bash ./common/make_boost_la.sh

echo "------------------------------------"
echo "A4: Builtin Miniboost ready for use!"
echo "------------------------------------"
