# Created by Charles Mahoudeau on 19/09/2024.


# We could also link statically
# https://stackoverflow.com/questions/18138635/mingw-exe-requires-a-few-gcc-dlls-regardless-of-the-code

import sys
import os
import shutil


REQUIRED_DLL_LIST = [
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll"
]


def get_compiler_bin_path():
    for path_value in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(path_value + os.sep + "gcc.exe") and os.path.exists(path_value + os.sep + "g++.exe"):
            return path_value

    return None


def get_dll_paths(compiler_bin_path):
    paths = []

    for dll_filename in REQUIRED_DLL_LIST:
        path = compiler_bin_path + os.sep + dll_filename
        if os.path.exists(path):
            paths.append(path)
            print("Found DLL:", dll_filename)
        else:
            print("Missing DLL:", dll_filename)

    return paths

def copy_files(file_paths, destination_directory):
    for file_path in file_paths:
        shutil.copy2(file_path, destination_directory)
        print("Copied", file_path, "to", destination_directory)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Missing target directory in arguments.")
        sys.exit(1)

    target_dir = sys.argv[1]

    if not os.path.isdir(target_dir):
        print("Target directory is not a directory.")
        sys.exit(1)

    compiler_path = get_compiler_bin_path()

    if compiler_path is None:
        print("Unable to find a compiler.")
        sys.exit(1)

    print("Using compiler:", compiler_path)

    dll_paths = get_dll_paths(compiler_path)

    copy_files(dll_paths, target_dir)

    print("Done.")
