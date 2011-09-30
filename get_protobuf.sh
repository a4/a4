#!/bin/bash

protobuf_name=protobuf-2.4.1

protobuf_pack=$protobuf_name.tar.bz2
protobuf_url=http://protobuf.googlecode.com/files/$protobuf_pack

echo "-------------------------------------------------"
echo "A4: Acquiring source of needed Boost Libraries..."
echo "-------------------------------------------------"

if ! test -d "protobuf"; then
  if ! test -e $protobuf_pack; then
    echo "Downloading A4 protobuf library $protobuf_name..."
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
  mv $protobuf_name protobuf
  echo "Builtin Protobuf Library Setup complete!"
else
  echo "Existing ./protobuf directory found!"
fi

pushd protobuf

echo "---------------------------"
echo "A4: Configuring Protobuf..."
echo "---------------------------"

if ! ./configure --prefix=$PWD; then
  echo "Protobuf configure failed! :("
  echo "Edit ./get_protobuf.sh if you need to add additional options to ./configure"
  exit 1
fi

echo "---------------------------------"
echo "A4: Compiling Protobuf Library..."
echo "---------------------------------"

if ! make -j4 && make install; then
  echo "Protobuf compilation failed! :("
  exit 1
fi
popd

echo "------------------------------------"
echo "A4: Builtin Protobuf ready for use!"
echo "------------------------------------"
