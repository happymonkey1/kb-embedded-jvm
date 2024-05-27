#!/usr/bin/env bash
if [ ! -d .kb-cppbuild ] ; then
  mkdir ./kb-jvm/vendor
fi
wget -P ./kb-jvm/vendor/ https://corretto.aws/downloads/latest/amazon-corretto-21-x64-linux-jdk.tar.gz
tar -xzvf ./kb-jvm/vendor/amazon-corretto-21-x64-linux-jdk.tar.gz -C ./kb-jvm/vendor/
rm ./kb-jvm/vendor/amazon-corretto-21-x64-linux-jdk.tar.gz