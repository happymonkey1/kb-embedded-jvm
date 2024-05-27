//
// Created by happymonkey1 on 5/26/24.
//

// stb single include style
#define KB_CPP_BUILD_IMPLEMENTATION
#include "kablunk/cppbuild/kb_cpp_build.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>

auto main(int argc, const char **argv) -> int {
    // detects changing to the build file and rebuilds itself first
    // then tries rebuilding `kb-jvm`
    KB_GO_REBUILD_URSELF(argc, argv);

    kb::cppbuild::init(
        kb::cppbuild::cppbuild_specification{
            .m_tool_chain = kb::cppbuild::tool_chain_t::gcc,
            .m_target_platform = kb::cppbuild::target_platform_t::linux,
            .m_build_directory = ".kb-cppbuild",
        }
    );

    // included because bazel defines a working directory that we can not override
    auto project_path = std::getenv("KB_JVM_PROJECT_PATH");
    if (project_path == nullptr) {
        std::cerr << "env KB_JVM_PROJECT_PATH is not defined!\n";
        return 1;
    }

    auto res = kb::cppbuild::create_executable(
        kb::cppbuild::executable_specification{
            .m_name = "kb-jvm-out",
            .m_project_root = std::filesystem::path{ project_path } / "kb-jvm",
            .m_include_dirs = {
                "/include/",
                "/src/",
                "vendor/amazon-corretto-21.0.3.9.1-linux-x64/include",
                "vendor/amazon-corretto-21.0.3.9.1-linux-x64/include/linux"
            },
            .m_search_lib_dirs = {
                "vendor/amazon-corretto-21.0.3.9.1-linux-x64/lib/server"
            },
            .m_header_files = {},
            .m_source_files = { "src/kablunk/kb-jvm/kb_jvm_entrypoint.cpp" },
            .m_static_libs = {
                "jvm",
            }
        }
    );

    if (!res) {
        return 1;
    }

    return 0;
}