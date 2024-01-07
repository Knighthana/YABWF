#!/usr/bin/env bash

# make clangd understand the situation

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
