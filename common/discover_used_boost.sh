#! /usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

ack --no-heading '#include <boost/' ${SCRIPT_DIR}/../a4* | cut -d: -f3- | sort -u | sed -r 's|^#include <boost/(.*).hpp>|\1|' | xargs
