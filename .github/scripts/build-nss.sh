#!/bin/env bash

set -e
set -x
set -o pipefail

echo "Cloning NSPR..."
hg clone https://hg.mozilla.org/projects/nspr

echo "Cloning NSS..."
hg clone https://hg.mozilla.org/projects/nss

echo "Building NSS..."
./nss/build.sh -c --fuzz --disable-tests
