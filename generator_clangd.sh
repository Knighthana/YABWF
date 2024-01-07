#!/usr/bin/env bash

# Brief: make clangd understand the situation
# Author: Knighthana (https://github.com/Knighthana)
# Version: docv1.0.0
# Date: 2024/01/07
# any problem please make an issue on https://github.com/Knighthana/YABWF

PROJECT_PATH=$(dirname $0)
cd $PROJECT_PATH
PROJECT_PATH=$(pwd)
if [ -e $PROJECT_PATH/.clangd ]; then
  rm $PROJECT_PATH/.clangd
else
  touch $PROJECT_PATH/.clangd
fi

echo "CompileFlags:" >> .clangd
echo "  Compiler: gcc" >> .clangd
echo "  Add: [\
-I$PROJECT_PATH/src
]" >> .clangd
