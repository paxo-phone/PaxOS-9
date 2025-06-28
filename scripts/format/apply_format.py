import os
import shutil
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
PROJECT_DIR = os.path.dirname(os.path.dirname(SCRIPT_DIR))


def get_clang_format_path():
    clang_format = shutil.which("clang-format")
    if not clang_format:
        raise EnvironmentError("clang-format not found in PATH. Please install it or add it to your PATH.")
    return clang_format


def is_source_file(file):
    return file.endswith('.c') or file.endswith('.cpp') or file.endswith('.h') or file.endswith('.hpp')


def get_files():
    output = []
    for path, dirs, files in os.walk(PROJECT_DIR):
        for file in files:
            if not is_source_file(file):
                continue
            source_path = os.path.relpath(str(os.path.join(path, file)), PROJECT_DIR)
            output.append(source_path)
    return output


def format_files(formatter_path, paths):
    for file in paths:
        full_path = os.path.join(PROJECT_DIR, file)
        subprocess.run([formatter_path, "-i", full_path], check=True)
        print(f"✓ {file}")


if __name__ == "__main__":
    clang_format_path = get_clang_format_path()
    print("Using clang-format at:", clang_format_path)
    files = get_files()
    format_files(clang_format_path, files)
    print(f"Done, formatted {len(files)} files.")
