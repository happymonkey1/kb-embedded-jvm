#!/usr/bin/env bash
if [ ! -d .kb-cppbuild ] ; then
  mkdir .kb-cppbuild
fi
OUTPUT=$(g++ ./kb-jvm/src/kablunk/kb_jvm_build.cpp -Ikb-cppbuild/include/ -o .kb-cppbuild/kb-cppbuild-kb-jvm -std=c++20)
if [[ $? != 0 ]]; then
echo -e $OUTPUT
fi