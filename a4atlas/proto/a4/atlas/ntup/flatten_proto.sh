#! /usr/bin/env bash

set -u -e

[[ -z "${1-}" ]] && { echo Specify a directory! >&2; exit 1; }

FILES="${1}/*.proto"
# Put Event.proto at the end of the list so it appears last in the file.
FILES="$(echo $FILES | sed -r "s| ${1}/Event.proto||") ${1}/Event.proto"

(
echo "package a4.atlas.ntup.${1};"
echo 'import "a4/root/RootExtension.proto";'

for file in $FILES;
do 
  echo Processing ${file} >&2
  tail -n+3 $file | grep -vE "^import" | sed -r 's@(optional|repeated)@// \1@'
  #echo
done && 
echo
) > ${1}_flat.proto
