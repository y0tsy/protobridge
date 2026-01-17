#!/bin/bash
set -e

cd "$(dirname "$0")/.."

cmake -S . -B Build -DCMAKE_BUILD_TYPE=Release
cmake --build Build --config Release