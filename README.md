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
- Amazon Corretto 21 (JDK 21)

# Platform support
- Linux (Yes, Tested on Ubuntu 22.04)
- Windows (Coming soon)
- MacOS (Probably never)

# Building from source

First we need to build our build system from source.
- `g++ PATH_TO_CPP_BUILD_SCRIPT -I PATH_TO_CPPBUILD_HEADER -o OUTPUT_NAME -std=c++20`
- Example from `kb-embedded-jvm` root: `g++ ./kb-jvm/src/kablunk/kb_jvm_build.cpp -Ikb-cppbuild/include/ -o kb_jvm_build -std=c++20`  
There is also an helper script: `/kb-jvm/scripts/build-kb-cppbuild-for-kb-jvm.sh` which can bootsrap the build system.
- By default, this creates a build directory `.kb-cppbuild` and puts the build system executable there.

`kb-jvm` requires two environment variables to be set
  - `KB_JVM_PROJECT_PATH` set to the root of the `kb-jvm` project
  - `KB_JAVA_PROJECT_PATH` set to the root of the java project to be loaded by jni (Example: `java-gradle-example`)

We also need to retrieve Java 21 JDK as a dependency.  
There is an included helper script `kb-jvm/scripts/get-vendors.sh` to do this automatically.
The script must be run from the root of the entire repository (`kb-embedded-jvm`).
Example: `cd PATH/TO/kb-embedded-jvm && ./kb-jvm/scripts/get-vendors.sh`

Once our build system is built, we can run it to build `kb-jvm`
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

## Why Write This?

I like doing hobby project in c++, and wanted to explore embedding Java as that is something I might implement for my Game Engine ([Kablunk Engine](https://github.com/happymonkey1/KablunkEngine))

## Why Bazel?

Well, I did not want to code without ide support, so I included Bazel :P  
This project not need Bazel to compile.