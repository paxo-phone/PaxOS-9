import argparse
import fnmatch
import os
import shutil
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__)).replace("\\", "/")
PROJECT_DIR = os.path.dirname(os.path.dirname(SCRIPT_DIR)).replace("\\", "/")
CLANG_FORMAT_FILE = os.path.join(PROJECT_DIR, ".clang-format").replace("\\", "/")
CLANG_FORMAT_IGNORE_FILE = os.path.join(PROJECT_DIR, ".clang-format-ignore").replace("\\", "/")


def get_clang_format_path():
    clang_format = shutil.which("clang-format")
    if not clang_format:
        raise EnvironmentError("clang-format not found in PATH. Please install it or add it to your PATH.")
    version_result = subprocess.run(
        [
            clang_format,
            "--version"
        ],
        check=True,
        capture_output=True,
        text=True
    )
    if version_result.returncode != 0:
        raise RuntimeError("Failed to get clang-format version. Ensure clang-format is installed correctly.")
    version_string = version_result.stdout.strip()
    version = version_string.split("version ")[1]
    major_version = version.split(".")[0]
    minor_version = version.split(".")[1]
    patch_version = version.split(".")[2]
    if int(major_version) < 20:
        raise EnvironmentError("clang-format version 20.1.5 or higher is required. Please update clang-format.")
    if int(major_version) == 20:
        if int(minor_version) < 1:
            raise EnvironmentError("clang-format version 20.1.5 or higher is required. Please update clang-format.")
        if int(minor_version) == 1:
            if int(patch_version) < 5:
                raise EnvironmentError("clang-format version 20.1.5 or higher is required. Please update clang-format.")
    if not os.path.isfile(CLANG_FORMAT_FILE):
        raise FileNotFoundError(f".clang-format file not found at {CLANG_FORMAT_FILE}. Please create one.")
    return clang_format


def get_ignore_list():
    with open(CLANG_FORMAT_IGNORE_FILE, "r") as f:
        ignore_list = [line.strip() for line in f if line.strip() and not line.startswith("#")]
    return ignore_list


def is_file_ignored(ignore_list, file):
    for pattern in ignore_list:
        if fnmatch.fnmatch(file, pattern):
            return True
    return False


def is_source_file(file):
    return file.endswith(".c") or file.endswith(".cpp") or file.endswith(".h") or file.endswith(".hpp")


def get_files():
    output = []
    for path, dirs, files in os.walk(PROJECT_DIR):
        for file in files:
            if not is_source_file(file):
                continue
            source_path = os.path.relpath(str(os.path.join(path, file)), PROJECT_DIR).replace("\\", "/")
            output.append(source_path)
    return output


def format_files(formatter_path, paths):
    for file in paths:
        full_path = os.path.join(PROJECT_DIR, file).replace("\\", "/")
        subprocess.run(
            [
                formatter_path,
                f"--style=file:{CLANG_FORMAT_FILE}",
                "-i",
                full_path
            ],
            check=True
        )
        print(f"✓ {file}")


def check_files(formatter_path, paths):
    is_format_correct = True
    for file in paths:
        full_path = os.path.join(PROJECT_DIR, file).replace("\\", "/")
        result = subprocess.run(
            [
                formatter_path,
                f"--style=file:{CLANG_FORMAT_FILE}",
                "--dry-run",
                "--Werror",
                full_path
            ],
            check=False,
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            is_format_correct = False
            print(f"✗ {file} - Formatting issues found:")
            print(result.stderr)
        else:
            print(f"✓ {file}")
    return is_format_correct


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='CLang Format Script',
        description='Apply or check formatting of source files using clang-format.'
    )
    parser.add_argument("--check", action="store_true", help="Check formatting without applying changes.")
    args = parser.parse_args()
    os.chdir(PROJECT_DIR)
    clang_format_path = get_clang_format_path()
    print("Using clang-format at:", clang_format_path)
    target_files = get_files()
    ignore_files = get_ignore_list()
    target_files = [file for file in target_files if not is_file_ignored(ignore_files, file)]
    if args.check:
        print("Checking formatting of files...")
        formatting_valid = check_files(clang_format_path, target_files)
        print(f"Done, checked {len(target_files)} files.")
        if formatting_valid:
            print("All files are formatted correctly.")
        else:
            print("Some files are not formatted correctly. Please fix them.")
            exit(1)
    else:
        print("Formatting files...")
        format_files(clang_format_path, target_files)
        print(f"Done, formatted {len(target_files)} files.")
