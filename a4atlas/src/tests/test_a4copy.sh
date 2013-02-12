#! /usr/bin/env bash

set -e -u

export A4_LOG_LEVEL=5

# NOTE: This test is not valid unless "./waf install" has been done.

a4copy --per run data/salient-realworld.a4

