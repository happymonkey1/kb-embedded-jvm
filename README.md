# kb-embedded-jvm

Hobby project to embed jvm in a c++ project. Includes a custom (extremely limited) build system

## References
- [JDK 21 Documentation](https://docs.oracle.com/en/java/javase/21/index.html)
  - [Java Invocation API](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html)
- [Amazon Corretto](https://aws.amazon.com/corretto/)

## kb-cppbuild

Small subproject for a simple build system written in c++. Used to build the kb-embedded-jvm project without
any extra build tools (see Why there are `Bazel` files below, if you are curious).

# Dependencies
- c++20 toolchain (ex: g++13)

# Platform support
- Linux (Yes, Tested on Ubuntu 22.04)
- Windows (Coming soon)
- MacOS (Probably never)

# Building from source

First we need to build our build system from source. 

- `g++ PATH_TO_CPP_BUILD_SCRIPT -I PATH_TO_CPPBUILD_HEADER -o OUTPUT_NAME -std=c++20`
  - Example from `kb-embedded-jvm` root: `g++ ./kb-jvm/src/kablunk/kb_jvm_build.cpp -Ikb-cppbuild/include/ -o kb_jvm_build -std=c++20`

Once built, we can run our build system program to build `kb-jvm`
Example: `./.kb-cppbuild/kb-cppbuild-kb-jvm`

## Building with Bazel

Bazel is used to generate IDE files (CLion) and was used to bootstrap `kb-cppbuild`.
It is no longer needed to build the project.

For posterity's sake, the commands to use Bazel are:
- `bazel build //kb-jvm:kb-jvm-cppbuild` to build `kb-cppbuild`
- `bazel build //kb-jvm:kb-jvm-cli-example` to build cli for `kb-embedded-jvm` 

# FAQ

## Common Errors

- `error while loading shared libraries: libjvm.so: cannot open shared object file: No such file or directory`
  - Set `LD_LIBRARY_PATH` with the path to `libjvm.so` before running the executable

## Why Bazel?

Well, I did not want to code without ide support, so I included Bazel :P
This project *should* (while `kb-cppbuild` is being worked on, Bazel is required to bootstrap) not need Bazel to compile