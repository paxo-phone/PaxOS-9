import os
import shutil
import zipfile

import requests

Import("env")

PROJECT_DIR = env.subst("$PROJECT_DIR")
BUILD_DIR = env.subst("$BUILD_DIR")

libraries = [
    (
        "https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-mingw.zip",
        "SDL2",
        [
            ("x86_64-w64-mingw32/bin/SDL2.dll", "SDL2.dll")
        ]
    ),
    (
        "https://curl.se/windows/latest.cgi?p=win64-mingw.zip",
        "curl",
        [
            ("bin/libcurl-x64.dll", "libcurl-x64.dll")
        ]
    )
]


def cleanup():
    """
    Cleanup the temp directory.
    """

    try:
        shutil.rmtree("temp")
    except PermissionError:
        print("Could not delete temp directory. Please delete them manually.")


def download():
    """
    Download libraries.
    """

    for library_info in libraries:
        url, folder_name, files_to_copy = library_info

        print(f"Downloading {folder_name} from {url}...")

        r = requests.get(url)
        zip_path = os.path.join("temp", f"{folder_name}.zip")

        with open(zip_path, "wb") as file:
            file.write(r.content)

        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            topdir = zip_ref.namelist()[0]
            zip_ref.extractall("extern")

            extracted_folder = os.path.join("extern", topdir)
            if os.path.exists(extracted_folder):
                os.rename(extracted_folder, os.path.join("extern", folder_name))

        for src, dst in files_to_copy:
            src_path = os.path.join("extern", folder_name, src)
            dst_path = os.path.join(BUILD_DIR, dst)
            shutil.copyfile(src_path, dst_path)

    print("All libraries downloaded and extracted successfully.")


def check_extern_integrity():
    """
    Check if the extern directory and its contents are valid.
    :return: True if the extern directory is valid, False otherwise.
    """

    if not os.path.exists("extern"):
        return False

    for library_info in libraries:
        url, folder_name, files_to_copy = library_info
        folder_path = os.path.join("extern", folder_name)

        if not os.path.exists(folder_path):
            return False

        for src, _ in files_to_copy:
            src_path = os.path.join(folder_path, src)
            if not os.path.exists(src_path):
                return False

    return True


def copy_files():
    """
    Copy files from the extern directory to the build directory.
    """

    for library_info in libraries:
        url, folder_name, files_to_copy = library_info

        for src, dst in files_to_copy:
            src_path = os.path.join("extern", folder_name, src)
            dst_path = os.path.join(BUILD_DIR, dst)
            shutil.copyfile(src_path, dst_path)


def setup_workspace():
    """
    Setup the workspace (Download libraries, copy .dll, ...)
    """

    if check_extern_integrity():
        return

    if os.path.exists("extern"):
        shutil.rmtree("extern")
    if os.path.exists("temp"):
        shutil.rmtree("temp")

    os.mkdir("extern")
    os.mkdir("temp")

    download()
    cleanup()
    copy_files()


def copy_dependencies():
    """
    Copy dependencies before building the program.
    """

    # Ensure the extern directory exists
    if not os.path.exists("extern"):
        raise FileNotFoundError("The 'extern' directory does not exist. Please run setup_workspace first.")

    copy_files()


setup_workspace()
env.AddPreAction("buildprog", copy_dependencies)
print(env.items())
