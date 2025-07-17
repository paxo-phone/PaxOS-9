import os

Import("env")

if "MSYS2_ROOT" in os.environ:
    msys2_root = os.environ["MSYS2_ROOT"]
    env.Append(CPPPATH=[
        os.path.join(msys2_root, "mingw64", "include"),
    ])
    env.Append(LIBS=["curl"])
    env.Append(LIBPATH=[
        os.path.join(msys2_root, "mingw64", "lib"),
    ])
