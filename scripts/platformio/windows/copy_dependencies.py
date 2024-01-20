import os
import shutil


Import("env")


def SDL2_copyDLL():
    build_dir = env.subst("$BUILD_DIR")

    shutil.copyfile("extern\\SDL2-2.28.5\\x86_64-w64-mingw32\\bin\\SDL2.dll", f"{build_dir}\\SDL2.dll")

    build_dir = env.subst("$BUILD_DIR")

    # Copy SDL2.dll
    if not os.path.exists(f"{build_dir}\\SDL2.dll"):
        SDL2_copyDLL()


def copy_dependencies():
    build_dir = env.subst("$BUILD_DIR")

    # Copy SDL2.dll
    if not os.path.exists(f"{build_dir}\\SDL2.dll"):
        SDL2_copyDLL()


# Execute "copy_dependencies" before building program
env.AddPreAction("buildprog", copy_dependencies)
