//
// Created by happymonkey1 on 5/26/24.
//

#include <jni.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <filesystem>

auto main(int argc, const char **argv) -> int {
    JavaVM *jvm = nullptr;
    JNIEnv *env = nullptr;

    const char* k_corretto_vendor_rel_path = "kb-jvm/vendor/amazon-corretto-21.0.3.9.1-linux-x64";

    const auto root_dir = std::getenv("KB_JVM_PROJECT_PATH");
    if (!root_dir) {
        std::cerr << "[ERROR]: env KB_JVM_PROJECT_PATH is not defined!\n";
        return 1;
    }
    const auto root_dir_path = std::filesystem::path{ root_dir };

    std::cout << std::format("[INFO]: Root project path KB_JVM_PROJECT_PATH={}\n", root_dir);
    const auto java_bin_path = root_dir_path / k_corretto_vendor_rel_path / "bin";
    const auto java_bin_path_str = java_bin_path.string();

    const auto java_project_dir = std::getenv("KB_JAVA_PROJECT_PATH");
    if (!java_project_dir) {
        std::cerr << "[ERROR]: env KB_JAVA_PROJECT_PATH is not defined!\n";
        return 1;
    }
    const auto java_project_path = std::filesystem::path{ java_project_dir };
    const auto java_project_path_str = java_project_path.string();

    const auto class_path_arg = std::format("-Djava.class.path={}build/libs/java-gradle-example-1.0.jar",
        java_project_path_str
    );
    std::cout << std::format("[INFO]: using user class path '{}'\n", class_path_arg);

    const auto k_java_lib_path = root_dir_path / std::filesystem::path{ k_corretto_vendor_rel_path } / "lib";
    const auto java_lib_path_str = k_java_lib_path.string();
    const auto java_lib_path_arg = std::format("-Djava.library.path={}", java_lib_path_str);

    constexpr size_t k_option_count = 2;
    JavaVMOption options[k_option_count] = {
        JavaVMOption{ .optionString = const_cast<char*>(class_path_arg.c_str()) },
        JavaVMOption{ .optionString = const_cast<char*>(java_lib_path_arg.c_str()) },
        // JavaVMOption{ .optionString = const_cast<char*>("-verbose:class") },
    };

    JavaVMInitArgs vm_args{
        .version = JNI_VERSION_21,
        .nOptions = k_option_count,
        .options = options,
        .ignoreUnrecognized = false
    };

    const auto create_vm_res = JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args);
    if (create_vm_res != JNI_OK) {
        std::cerr << "[ERROR]: Failed to create java VM!";
        return 1;
    }

    jint jvm_version = env->GetVersion();
    std::cout << std::format("[INFO]: JVM version {}.{}\n", (jvm_version>>16)&0x0f, jvm_version&0x0f);

    // invoke main class
    const char* main_class_cstr = "com/kablunk/KbJavaExample/Main";
    jclass cls = env->FindClass(main_class_cstr);
    if (!cls) {
        env->ExceptionDescribe();
        std::cerr << std::format("[ERROR]: Could not find '{}' class!\n", main_class_cstr);
        return 1;
    }

    jmethodID method_id = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
    if (!method_id) {
        env->ExceptionDescribe();
        std::cerr << "[ERROR]: Could not find 'main' method!\n";
        return 1;
    }

    jobjectArray args = env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    env->CallStaticVoidMethod(cls, method_id, args);

    jvm->DestroyJavaVM();

    return 0;
}