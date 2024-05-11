import os
import shutil


Import("env")


def SDL2_copyDLL():
    build_dir = env.subst("$BUILD_DIR")

    shutil.copyfile("extern\\SDL2-2.28.5\\x86_64-w64-mingw32\\bin\\SDL2.dll", f"{build_dir}\\SDL2.dll")

    build_dir = env.subst("$BUILD_DIR")


def cURL_copyDLL():
    build_dir = env.subst("$BUILD_DIR")

    shutil.copyfile("extern\\curl-8.7.1_8-win64-mingw\\bin\\libcurl-x64.dll", f"{build_dir}\\libcurl-x64.dll")

    build_dir = env.subst("$BUILD_DIR")


def copy_dependencies():
    build_dir = env.subst("$BUILD_DIR")

    # Copy SDL2.dll
    if not os.path.exists(f"{build_dir}\\SDL2.dll"):
        SDL2_copyDLL()

    # Copy libcurl-x64.dll
    if not os.path.exists(f"{build_dir}\\libcurl-x64.dll"):
        cURL_copyDLL()


# Execute "copy_dependencies" before building program
env.AddPreAction("buildprog", copy_dependencies)
