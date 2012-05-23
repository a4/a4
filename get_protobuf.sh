#!/bin/bash

target_name=protobuf
protobuf_name=protobuf-2.4.1

protobuf_pack=$protobuf_name.tar.bz2
protobuf_url=http://protobuf.googlecode.com/files/$protobuf_pack

if test -d $target_name; then
  echo "Existing $target_name/ directory found, exiting..."
  exit -1
fi

echo "-------------------------------------------"
echo "A4: Acquiring source of protobuf library..."
echo "-------------------------------------------"

if ! test -d $protobuf_name; then
  if ! test -e $protobuf_pack; then
    echo "Downloading A4 $target_name library $protobuf_name..."
    curl -f $protobuf_url > $protobuf_pack
    if test $? != 0; then
      echo "FATAL: Download failed! :( "
      exit 1
    fi
  else
    echo "Using existing file $protobuf_pack."
  fi
  echo "Unpacking $protobuf_pack..."
  if ! tar -xj -f $protobuf_pack; then
    echo "FATAL: Could not unpack $protobuf_pack!"
    exit 1
  fi
  echo "Extracted $protobuf_name."
else
  echo "Existing $protobuf_name directory found!"
fi

prefix=$PWD/$target_name
mkdir $prefix

pushd $protobuf_name

echo "-----------------------------------"
echo "A4: Configuring Protobuf Library..."
echo "-----------------------------------"

if ! ./configure --prefix=$prefix; then
  echo "Protobuf configure failed! :("
  echo "Edit ./get_protobuf.sh if you need to add additional options to ./configure"
  exit 1
fi

echo "---------------------------------"
echo "A4: Compiling Protobuf Library..."
echo "---------------------------------"

if ! make $@; then
  echo "Protobuf compilation failed! :("
  exit 1
fi

echo "----------------------------------------"
echo "A4: Local install of Protobuf Library..."
echo "----------------------------------------"

if ! make install; then
  echo "Protobuf installation into $PWD failed! :("
  exit 1
fi

pushd python

mkdir -p $prefix/python
if ! PYTHONPATH=$prefix/python python setup.py install --prefix $prefix --install-purelib $prefix/python; then
  echo "Protobuf python installation failed! :("
  exit 1
fi
popd

popd

rm -rf $protobuf_name/

echo "------------------------------------"
echo "A4: Builtin Protobuf ready for use!"
echo "------------------------------------"
