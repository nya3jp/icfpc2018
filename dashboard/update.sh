#!/bin/bash

set -ex

path=$1

cd "$(dirname "$0")"

./generate.py > "$path/dashboard.html"
cp debug.html "$path/"
rsync -r ../data "$path/"
gzip -fdn "$path"/data/{models,traces}/*.gz
