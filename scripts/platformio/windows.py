import os

Import("env")

run_actions = []
copy_dependencies_actions = []

# Get the build directory
build_dir = env.subst("$BUILD_DIR")

# Get the library directories
lib_dir_list = env["LIBSOURCE_DIRS"]

# Find SDL2.dll
for lib_dir in lib_dir_list:
    path = "extern\\SDL2-2.28.5\\x86_64-w64-mingw32\\bin\\SDL2.dll"

    if os.path.isfile(path):
        # Copy SDL2.dll to build directory
        target_path = build_dir + "\\SDL2.dll"

        # Commands : https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/copy

        # Check if the build dir has SDL2.dll
        if not os.path.isfile(build_dir + "\\SDL2.dll"):
            # Only add to run actions if not already available
            run_actions.append(f"copy /-y /b \"{path}\" /b \"{target_path}\"")

        # Add to copy dependencies action, with verify (/v)
        copy_dependencies_actions.append(f"copy /v /-y /b \"{path}\" /b \"{target_path}\"")

        break


# Launch .exe
run_actions.append("$BUILD_DIR\\program.exe")

# Add custom Run target
env.AddCustomTarget(
    name="run",
    dependencies=None,
    actions=run_actions,
    title="Run",
    description="Run the .exe"
)

# Add custom Copy Dependencies target
env.AddCustomTarget(
    name="copy_dependencies",
    dependencies=None,
    actions=copy_dependencies_actions,
    title="Copy Dependencies",
    description="Copy all dependencies for the .exe"
)