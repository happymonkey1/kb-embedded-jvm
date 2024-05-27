//
// Created by happymonkey1 on 5/26/24.
//

#ifndef KABLUNK_CPP_BUILD_H
#define KABLUNK_CPP_BUILD_H

#include <string>
#include <vector>
#include <optional>
#include <cstdlib>
#include <format>
#include <filesystem>

#ifndef KB_CORE

#include <cassert>
#define KB_CORE_ASSERT(x) do { assert((x)); } while(0)

// #TODO: probably define an option to not include...
#include <iostream>
#define KB_CORE_INFO(x, ...) std::cout << std::format("[INFO]: {}", std::format(x, __VA_ARGS__)) << '\n'
#define KB_CORE_ERROR(x, ...) std::cerr << std::format("[ERROR]: {}", std::format(x, __VA_ARGS__)) << '\n'

#include <cstdint>

namespace kb {
using u32 = uint64_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;

template <typename T>
using option = ::std::optional<T>;
} // end namespace kb

#endif

#ifndef __cplusplus
#error "c++ toolchain required!"
#else

namespace kb::cppbuild {

enum class cppbuild_error_t {
    // unspecified internal error
    internal = 0,
};

enum class tool_chain_t {
    none = 0,
    gcc,
    clang,
    msvc
};

enum class target_platform_t {
    none = 0,
    linux,
    windows,
};

// config for cppbuild itself
struct cppbuild_specification {
    // tool chain used to build
    tool_chain_t m_tool_chain = tool_chain_t::none;
    // target platform to build
    target_platform_t m_target_platform = target_platform_t::none;
    // optional name of output build directory
    option<std::filesystem::path> m_build_directory{};
};

struct executable_specification {
    // name of the output executable
    std::string m_name{};
    // root directory of the project
    std::filesystem::path m_project_root{};
    // include directories
    std::vector<::std::string> m_include_dirs{};
    // directories to search for libs
    std::vector<std::string> m_search_lib_dirs{};
    // header files to include
    std::vector<::std::string> m_header_files{};
    // source files to include
    std::vector<::std::string> m_source_files{};
    // list of static libs to link
    std::vector<::std::string> m_static_libs{};
};

struct c_str_array_t {
    std::vector<const char*> m_elems;
    size_t m_count;
};

namespace {
cppbuild_specification s_cppbuild_specification{};
}

// --- implementation details -----------------

namespace details {

[[nodiscard]] auto build_executable_command(
        const executable_specification& p_specification
) noexcept -> option<std::string>;

[[nodiscard]] auto prefix_project_path(
        const executable_specification &p_specification,
        const std::filesystem::path &p_value
) noexcept -> std::filesystem::path;

[[nodiscard]] auto cstr_join(const char *p_sep, const std::vector<const char*>& p_c_strs) noexcept -> std::string;

[[nodiscard]] auto is_path1_modified_after_path2(
    const char *p_path1,
    const char *p_path2
) noexcept -> bool;

auto path_rename(
    const char *p_old_path,
    const char *p_new_path
) noexcept -> void;

} // end namespace ::details

// --------------------------------------------

// --- public api -----------------------------

#define KB_JOIN(sep, ...) ::kb::cppbuild::details::cstr_join(sep, std::vector<const char*>{ __VA_ARGS__ })
#define KB_CONCAT(...) KB_JOIN("", __VA_ARGS__)

auto init(cppbuild_specification p_specification) noexcept -> void;
[[nodiscard]] auto create_executable(const executable_specification& p_specification) noexcept -> option<cppbuild_error_t>;

#define KB_RUN_CMD(...) \
    do {         \
        const auto cmd = std::format("{}", __VA_ARGS__); \
        std::system(cmd.c_str()); \
    } while (0)

#define KB_RENAME(old_path, new_path) \
    do { \
        ::kb::cppbuild::details::path_rename(old_path, new_path); \
    } while (0)

#ifndef REBUILD_URSELF
#  if _WIN32
#    if defined(__GNUC__)
#       define KB_REBUILD_URSELF(binary_path, source_path) CMD("gcc", "-o", binary_path, source_path)
#    elif defined(__clang__)
#       define KB_REBUILD_URSELF(binary_path, source_path) CMD("clang", "-o", binary_path, source_path)
#    elif defined(_MSC_VER)
#       define KB_REBUILD_URSELF(binary_path, source_path) CMD("cl.exe", source_path)
#    endif
#  else
#    define KB_REBUILD_URSELF(binary_path, source_path) KB_RUN_CMD("{} {} {} {}", "cc", "-o", binary_path, source_path)
#  endif
#endif

// Implementation idea stolen from https://github.com/tsoding/nobuild/blob/master/nobuild.h
// TODO: implementing passing of cli arguments
#define KB_GO_REBUILD_URSELF(argc, argv) \
    do { \
        const char *source_path = __FILE__; \
        assert(argc >= 1); \
        const char *binary_path = argv[0]; \
        if (::kb::cppbuild::details::is_path1_modified_after_path2(source_path, binary_path)) { \
            const auto renamed = KB_CONCAT(binary_path, ".old"); \
            KB_RENAME(binary_path, renamed.c_str()); \
            KB_REBUILD_URSELF(binary_path, source_path); \
            exit(0); \
        } \
    } while(0)

// --------------------------------------------

} // end namespace kb::cppbuild

// stb style single header implementation
#ifdef KB_CPP_BUILD_IMPLEMENTATION
namespace kb::cppbuild {

auto init(cppbuild_specification p_specification) noexcept -> void {
    KB_CORE_ASSERT(p_specification.m_tool_chain != tool_chain_t::none);

    s_cppbuild_specification = p_specification;
}

auto create_executable(const executable_specification& p_specification) noexcept -> option<cppbuild_error_t> {
    const auto cmd = details::build_executable_command(p_specification);
    if (!cmd) {
        return cppbuild_error_t::internal;
    }
    auto build_dir = s_cppbuild_specification.m_build_directory.has_value() &&
        !s_cppbuild_specification.m_build_directory->empty() ?
        *s_cppbuild_specification.m_build_directory :
        std::filesystem::path{ "" };
    const auto build_dir_path = details::prefix_project_path(
        p_specification,
        build_dir
    );
    if (!std::filesystem::exists(build_dir_path)) {
        KB_CORE_INFO(
            "[INFO]: kb-cppbuild build dir '{}' does not exist. Creating...",
            build_dir_path.string()
        );
        std::filesystem::create_directories(
            build_dir_path
        );
    }

    KB_CORE_INFO("g++ build command: '{}'", *cmd);
    auto ret = std::system(cmd->c_str());
    if (!ret) {
        // TODO: check specific values and return different errors
        return cppbuild_error_t::internal;
    }

    return std::nullopt;
}

namespace details {

auto is_path1_modified_after_path2(
    const char *p_path1,
    const char *p_path2
) noexcept -> bool {
    // TODO: should probably verify that both paths exist...
    const auto path1_last_write_time = std::filesystem::last_write_time(p_path1);
    const auto path2_last_write_time = std::filesystem::last_write_time(p_path2);

    return path1_last_write_time > path2_last_write_time;
}

auto path_rename(
    const char *p_old_path,
    const char *p_new_path
) noexcept -> void {
    // TODO: should probably verify that old path exists
    if (std::filesystem::exists(p_old_path)) {
        KB_CORE_INFO("Renaming {} -> {}", p_old_path, p_new_path);
        std::filesystem::rename(p_old_path, p_new_path);
    }
}

auto prefix_cpp_files_with_project_root(
    const executable_specification& p_specification
) noexcept -> std::vector<std::filesystem::path> {
    std::vector<std::filesystem::path> prefixed_sources{};
    prefixed_sources.reserve(p_specification.m_source_files.size());

    const auto prefix = p_specification.m_project_root.empty() ? "" : p_specification.m_project_root;
    for (const auto& source_file : p_specification.m_source_files) {
        prefixed_sources.emplace_back(
            std::filesystem::path{ prefix } / std::filesystem::path{ source_file }
        );
    }

    return prefixed_sources;
}

[[nodiscard]] auto cstr_join(const char *p_sep, const std::vector<const char*>& p_c_strs) noexcept -> std::string {
    std::stringstream ss{};

    for (const char* str : p_c_strs) {
        ss << str;
        ss << p_sep;
    }

    return ss.str();
}

[[nodiscard]] auto prefix_project_path(
    const executable_specification &p_specification,
    const std::filesystem::path &p_value
) noexcept -> std::filesystem::path {
    return (p_specification.m_project_root.empty() ? "" : p_specification.m_project_root) /
        p_value;
}

auto build_include_directories_str(
    const executable_specification &p_specification
) noexcept -> std::string {
    std::stringstream ss{};

    const auto prefix = p_specification.m_project_root.empty() ? "" : p_specification.m_project_root;
    for (size_t i = 0; i < p_specification.m_include_dirs.size(); ++i) {
        const auto &include_dir = p_specification.m_include_dirs[i];
        ss << "-I";

        const auto include_dir_path = prefix / include_dir;
        ss << include_dir_path;

        if (i < p_specification.m_include_dirs.size() - 1) {
            ss << " ";
        }
    }

    return ss.str();
}

auto build_link_libraries_str(
    const executable_specification &p_specification
) noexcept -> std::string {
    if (p_specification.m_search_lib_dirs.empty() && p_specification.m_static_libs.empty())
        return "";

    std::stringstream ss{};

    const auto prefix = p_specification.m_project_root.empty() ? "" : p_specification.m_project_root;
    for (size_t i = 0; i < p_specification.m_search_lib_dirs.size(); ++i) {
        const auto &include_dir = p_specification.m_search_lib_dirs[i];
        ss << "-L";

        const auto include_dir_path = prefix / include_dir;
        ss << include_dir_path;

        if (i < p_specification.m_search_lib_dirs.size() - 1) {
            ss << " ";
        }
    }

    if (!p_specification.m_search_lib_dirs.empty()) {
        ss << " ";
    }

    for (size_t i = 0; i < p_specification.m_static_libs.size(); ++i) {
        const auto &lib = p_specification.m_static_libs[i];
        ss << "-l";
        ss << lib;

        if (i < p_specification.m_static_libs.size() - 1) {
            ss << " ";
        }
    }

    return ss.str();
}

auto build_executable_command(const executable_specification& p_specification) noexcept -> option<std::string> {
    KB_CORE_ASSERT(!p_specification.m_source_files.empty());
    KB_CORE_ASSERT(!p_specification.m_name.empty());

    switch (s_cppbuild_specification.m_tool_chain) {
        case tool_chain_t::gcc: {

            // sources
            auto cpp_files = prefix_cpp_files_with_project_root(p_specification);
            std::stringstream ss{};
            for (size_t i = 0; i < cpp_files.size(); ++i) {
                const auto& source_file = cpp_files[i];
                ss << source_file;
                if (i < cpp_files.size() - 1) {
                    ss << " ";
                }
            }

            // prefix output with build dir
            auto build_dir = s_cppbuild_specification.m_build_directory.has_value() &&
                !s_cppbuild_specification.m_build_directory->empty() ?
                *s_cppbuild_specification.m_build_directory :
                std::filesystem::path{ "" };
            const auto build_dir_path = details::prefix_project_path(
                    p_specification,
                    build_dir
            );
            // output binary
            const auto output_str = (build_dir_path / p_specification.m_name).string();

            // include directories
            const auto include_dir_str = build_include_directories_str(p_specification);

            // link libraries
            const auto link_libraries_str = build_link_libraries_str(p_specification);

            // TODO: -std=c++20 should not be hardcoded
            auto build_cmd = std::format(
                "g++ {} {} {} -o {} -std=c++20",
                ss.str(),
                include_dir_str,
                link_libraries_str,
                output_str
            );

            return build_cmd;
        }
        case tool_chain_t::clang: {
            KB_CORE_ERROR("clang tool chain is not implemented! :(", "");
            KB_CORE_ASSERT(false); // Not implemented
            return std::nullopt;
        }
        case tool_chain_t::msvc: {
            KB_CORE_ERROR("msvc tool chain is not implemented! :(", "");
            KB_CORE_ASSERT(false); // Not implemented
            return std::nullopt;
        }
        case tool_chain_t::none: {
            KB_CORE_ERROR("no tool chain specified! Did you forget to call `init()`?", "");
            // TODO: error msg
            return std::nullopt;
        }
    }

    KB_CORE_ASSERT(false); // Unreachable
    return std::nullopt;
}

} // end namespace ::details
} // end namespace kb::cppbuild

#endif // KB_CPP_BUILD_IMPLEMENTATION

#endif // __cplusplus

#endif //KABLUNK_CPP_BUILD_H
