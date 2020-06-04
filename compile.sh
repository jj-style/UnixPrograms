#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

SRC=src
DST=build

if [[ ! -d $SRC ]]; then
    echo "source folder does not exist!"
    exit 1;
fi

if [[ ! -d $DST ]]; then
    printf "build folder does not exist, creating now..."
    (mkdir $DST && echo "done") || (echo "failed to create build folder" && exit 1)
fi

for source_file in "$SRC"/*
do
    # echo "$source_file"
    file_name=$(basename $source_file)
    printf "compiling %s..." $file_name
    gcc -g -o $DST/${file_name%.*}.o $source_file && echo "ok" || echo "error"
done