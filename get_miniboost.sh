#!/bin/bash

miniboost_name=miniboost-1

miniboost_pack=$miniboost_name.tar.bz2
miniboost_url=http://www.ebke.org/$miniboost_pack

echo "-------------------------------------------------"
echo "A4: Acquiring source of needed Boost Libraries..."
echo "-------------------------------------------------"

if ! test -d "miniboost"; then
  if ! test -e $miniboost_pack; then
    echo "Downloading A4 miniboost library $miniboost_name..."
    curl -f $miniboost_url > $miniboost_pack
    if test $? != 0; then
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
  echo "Builtin Miniboost Library Setup complete!"
else
  echo "Existing ./miniboost directory found!"
fi

pushd miniboost

echo "--------------------------"
echo "A4: Bootstrapping Boost..."
echo "--------------------------"

if ! ./bootstrap.sh --prefix=$PWD; then
  echo "Boost bootstrap.sh failed! :("
  exit 1
fi

echo "---------------------------------------"
echo "A4: Compiling needed Boost Libraries..."
echo "---------------------------------------"

if ! ./b2 -j2 --prefix=$PWD; then
  echo "Boost compilation failed! :("
  exit 1
fi
popd

echo "------------------------------------"
echo "A4: Builtin Miniboost ready for use!"
echo "------------------------------------"
