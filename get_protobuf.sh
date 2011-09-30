#!/bin/bash

protobuf_name=protobuf-2.4.1

protobuf_pack=$protobuf_name.tar.bz2
protobuf_url=http://protobuf.googlecode.com/files/$protobuf_pack

if ! test -d "protobuf"; then
  if ! test -e $protobuf_pack; then
    echo "Downloading google protobuf library $protobuf_name..."
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
  echo "It will be compiled & installed together with A4."
else
  echo "Existing ./protobuf directory found!"
  echo "It will be compiled & installed together with A4."
fi

