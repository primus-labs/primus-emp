#!/bin/bash

rm -rf build build_wasm
rm -rf deps/install
rm -rf deps/openssl-1.1.1t

find . -name "build" -type d | xargs rm -rf
find . -name "build_wasm" -type d | xargs rm -rf
find . -name "node_modules" -type d | xargs rm -rf
