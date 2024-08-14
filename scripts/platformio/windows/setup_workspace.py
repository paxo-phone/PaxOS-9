import os
import requests
import zipfile
import shutil

Import("env")


def SDL2_cleanup():
    """
    Remove artifacts generated from "SDL2_download"
    """

    try:
        if os.path.exists("temp\\SDL2.zip"):
            os.remove("temp\\SDL2.zip")
    except PermissionError:
        print("Could not delete temp/SDL2.zip. Please delete them manually.")
    

def SDL2_download():
    """
    Download SDL2
    """

    r = requests.get("https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-mingw.zip")

    with open("temp\\SDL2.zip", "wb") as file:
        file.write(r.content)


def SDL2_extract():
    """
    Extract SDL2 to the "extern" directory
    """
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


def cURL_cleanup():
    """
    Remove artifacts generated from "cURL_download"
    """

    try:
        if os.path.exists("temp\\cURL.zip"):
            os.remove("temp\\cURL.zip")
    except PermissionError:
        print("Could not delete temp/cURL.zip. Please delete them manually.")


def cURL_download():
    """
    Download cURL
    """

    r = requests.get("https://curl.se/windows/dl-8.7.1_8/curl-8.7.1_8-win64-mingw.zip")

    with open("temp\\cURL.zip", "wb") as file:
        file.write(r.content)


def cURL_extract():
    """
    Extract cURL to the "extern" directory
    """
    with zipfile.ZipFile("temp\\cURL.zip", "r") as zipFile:
        zipFile.extractall("extern")


def cURL_exists():
    """
    :return: true, if cURL is installed, false, if cURL is not installed
    """

    return os.path.exists("extern") and os.path.exists("extern\\curl-8.7.1_8-win64-mingw")


def cURL_copyDLL():
    build_dir = env.subst("$BUILD_DIR")

    shutil.copyfile("extern\\curl-8.7.1_8-win64-mingw\\bin\\libcurl-x64.dll", f"{build_dir}\\libcurl-x64.dll")


def cleanup():
    """
    Cleanup
    """

    try:
        if os.path.exists("temp"):
            os.rmdir("temp")
    except PermissionError:
        print("Could not delete temp directory. Please delete it manually.")


def setup_workspace():
    """
    Set up the workspace (Download SDL2, copy .dll, ...)
    """

    build_dir = env.subst("$BUILD_DIR")

    # Create the "extern" directory
    if not os.path.exists("extern"):
        os.mkdir("extern")

    # Create a temp directory
    if not os.path.exists("temp"):
        os.mkdir("temp")

    # Download and setup SDL2
    if not SDL2_exists():
        # Create a temp directory
        if not os.path.exists("temp"):
            os.mkdir("temp")

        SDL2_download()
        SDL2_extract()
        SDL2_cleanup()

    # Download and setup cURL
    if not cURL_exists():
        cURL_download()
        cURL_extract()
        cURL_cleanup()

    # Cleanup
    cleanup()

    # Copy SDL2.dll
    if not os.path.exists(f"{build_dir}\\SDL2.dll"):
        SDL2_copyDLL()

    # Copy libcurl-x64.dll
    if not os.path.exists(f"{build_dir}\\libcurl-x64.dll"):
        cURL_copyDLL()


# Set up the workspace when importing the project
setup_workspace()
