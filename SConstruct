#!/usr/bin/env python
import os
import sys

from SCons.Script import *

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

if False: # VS project   
    env2 = Environment()

    str_sources = [path.get_path() for path in sources]

    proj = env2.MSVSProject(
        target = '#Bar' + env2['MSVSPROJECTSUFFIX'],
        srcs = str_sources,
        incs = ["src/", "lib/"],
        buildtarget = "demo/bin/voxelgame{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        variant = 'Release')
        
    Default(proj)
else:
    Default(library)