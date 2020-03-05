#!/bin/bash

set -e
set -u

cd "$(dirname "$(readlink -f "$0")")"

mkdir lib/
cd lib/
git clone https://github.com/DaveGamble/cJSON.git
