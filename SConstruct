#!/usr/bin/env python
import os
import sys

def GlobRecursive(node, pattern):
    results = []
    for f in Glob(str(node) + "/*", source=True):
        if f.isdir():
            results += GlobRecursive(f, pattern)
    results += Glob(str(node) + "/" + pattern, source=True)
    return results

env = SConscript("godot-cpp/SConstruct")

#include paths
env.Append(CPPPATH=["src/", "lib/"])

# source paths
sources = GlobRecursive("src", "*.cpp") + GlobRecursive("lib", "*.cpp") + GlobRecursive("lib", "*.cc")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/voxelgame.{}.{}.framework/voxelgame.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "demo/bin/voxelgame{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
