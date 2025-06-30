import os
import shutil
import zipfile

import requests

Import("env")


def SDL2_cleanup():
    try:
        if os.path.exists("temp"):
            if os.path.exists("temp\\SDL2.zip"):
                os.remove("temp\\SDL2.zip")
            os.remove("temp")
    except PermissionError:
        print("Could not delete temp directory or the SDL2.zip file. Please delete them manually.")
    


def SDL2_download():
    """
    Download SDL2
    """

    r = requests.get("https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-mingw.zip")

    with open("temp\\SDL2.zip", "wb") as file:
        file.write(r.content)


def SDL2_extract():
    with zipfile.ZipFile("temp\\SDL2.zip", "r") as zipFile:
        zipFile.extractall("extern")


def SDL2_exists():
    """
    :return: true, if SDL2 is installed, false, if SDL2 is not installed
    """

    return os.path.exists("extern") and os.path.exists("extern\\SDL2-2.28.5")


def SDL2_copyDLL():
    build_dir = env.subst("$BUILD_DIR")

    shutil.copyfile("extern\\SDL2-2.28.5\\x86_64-w64-mingw32\\bin\\SDL2.dll", f"{build_dir}\\SDL2.dll")

def setup_workspace():
    """
    Setup the workspace (Download SDL2, copy .dll, ...)
    """

    build_dir = env.subst("$BUILD_DIR")

    # Create the "extern" directory
    if not os.path.exists("extern"):
        os.mkdir("extern")

    # Download and setup SDL2
    if not SDL2_exists():
        # Create a temp directory
        if not os.path.exists("temp"):
            os.mkdir("temp")

        SDL2_download()
        SDL2_extract()
        SDL2_cleanup()

    # Copy SDL2.dll
    if not os.path.exists(f"{build_dir}\\SDL2.dll"):
        SDL2_copyDLL()


# Setup the workspace when importing the project
setup_workspace()
