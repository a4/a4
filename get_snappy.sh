#!/bin/bash

target_name=snappy
snappy_name=snappy-1.0.4

snappy_pack=$snappy_name.tar.gz
snappy_url=http://snappy.googlecode.com/files/$snappy_pack

threads=4

if test -d $target_name; then
  echo "Existing $target_name/ directory found, exiting..."
  exit -1
fi

echo "-------------------------------------------"
echo "A4: Acquiring source of snappy library..."
echo "-------------------------------------------"

if ! test -d $snappy_name; then
  if ! test -e $snappy_pack; then
    echo "Downloading A4 $target_name library $snappy_name..."
    curl -f $snappy_url > $snappy_pack
    if test $? != 0; then
      echo "FATAL: Download failed! :( "
      exit 1
    fi
  else
    echo "Using existing file $snappy_pack."
  fi
  echo "Unpacking $snappy_pack..."
  if ! tar -xf $snappy_pack; then
    echo "FATAL: Could not unpack $snappy_pack!"
    exit 1
  fi
  echo "Extracted $snappy_name."
else
  echo "Existing $snappy_name directory found!"
fi

prefix=$PWD/$target_name
mkdir $prefix

pushd $snappy_name

echo "-----------------------------------"
echo "A4: Configuring snappy Library..."
echo "-----------------------------------"

if ! ./configure --prefix=$prefix; then
  echo "snappy configure failed! :("
  echo "Edit ./get_snappy.sh if you need to add additional options to ./configure"
  exit 1
fi

echo "---------------------------------"
echo "A4: Compiling snappy Library..."
echo "---------------------------------"

if ! make -j$threads; then
  echo "snappy compilation failed! :("
  exit 1
fi

echo "----------------------------------------"
echo "A4: Local install of snappy Library..."
echo "----------------------------------------"

if ! make install; then
  echo "snappy installation into $PWD failed! :("
  exit 1
fi

rm -rf $snappy_name/

echo "------------------------------------"
echo "A4: Builtin snappy ready for use!"
echo "------------------------------------"
