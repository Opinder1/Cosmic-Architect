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

def HeadersRecursive(nodes):
    headers = []
    
    for node in nodes:
        headers += GlobRecursive("lib", "*.h")
        headers += GlobRecursive("lib", "*.hpp")
        
    return headers

def SourcesRecursive(nodes):
    headers = []
    
    for node in nodes:
        headers += GlobRecursive("lib", "*.c")
        headers += GlobRecursive("lib", "*.cc")
        
    return headers

# source paths
headers = HeadersRecursive(["src", "lib"])
sources = SourcesRecursive(["src", "lib"])

if True:
    env = SConscript("godot-cpp/SConstruct")

    #include paths
    env.Append(CPPPATH=["src/", "lib/"])

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

if True: # VS project   
    env2 = Environment()

    str_headers = [path.get_path() for path in headers]
    str_sources = [path.get_path() for path in sources]
    inc_paths = ["src/", "lib/", "godot-cpp/gdextension", "godot-cpp/include", "godot-cpp/gen/include"]
    target = "demo/bin/voxelgame{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
    flags = ["/std:c++17"]
    defines = ["REAL_T_IS_DOUBLE", "ENTT_ID_TYPE=std::uint64_t"]
    
    debug_settings = {
        'LocalDebuggerCommand': "C:/Godot/Godot_v4.1.2-stable_win64.exe",
        'LocalDebuggerCommandArguments': "--path C:/Godot/Projects/VoxelGame/demo",
    }

    proj = env2.MSVSProject(
        target = '#Bar' + env2['MSVSPROJECTSUFFIX'],
        srcs = str_sources,
        incs = str_headers,
        variant = ['Release'],
        cpppaths = [inc_paths],
        cppflags = [flags],
        cppdefines = [defines],
        buildtarget = [target],
        DebugSettings = [debug_settings],
    )
        
    Default(proj)
else:
    Default(library)