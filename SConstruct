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
        headers += GlobRecursive(node, "*.h")
        headers += GlobRecursive(node, "*.hpp")
        
    return headers

def SourcesRecursive(nodes):
    headers = []
    
    for node in nodes:
        headers += GlobRecursive(node, "*.c")
        headers += GlobRecursive(node, "*.cpp")
        headers += GlobRecursive(node, "*.cc")
        
    return headers

# Main environment
env = SConscript("godot-cpp/SConstruct")

# source paths
sources = SourcesRecursive(["src", "lib"])

#include paths
env.Append(CPPPATH=["src", "lib"])
env.Append(CPPDEFINES=["FLECS_CPP_NO_AUTO_REGISTRATION", "ecs_ftime_t=double"])

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "VoxelGameDemo/bin/voxelgame.{}.{}.framework/voxelgame.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "VoxelGameDemo/bin/voxelgame{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )
    
if ARGUMENTS.get("vs_proj"): # Generate VS project for intellisense and nice building (Don't use for actual releases)  
    env2 = Environment()
    
    env2.Append(CPPPATH=env["CPPPATH"])
    env2.Append(CPPDEFINES=env["CPPDEFINES"])
    # env2.Append(CPPFLAGS=env["CPPFLAGS"])
    env2.Append(CXXFLAGS=env["CXXFLAGS"])
    env2.Append(CCFLAGS=env["CCFLAGS"])
    env2.Append(LINKFLAGS=env["LINKFLAGS"])
    
    # source paths
    headers = HeadersRecursive(["src", "lib", "godot-cpp/gdextension", "godot-cpp/include", "godot-cpp/gen/include"])

    str_headers = [path.get_path() for path in headers]
    str_sources = [path.get_path() for path in sources]
    target = "VoxelGameDemo/bin/voxelgame{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
    
    debug_settings = {
        'LocalDebuggerCommand': "C:/Godot/godotengine/bin/godot.windows.template_debug.dev.double.x86_64.exe",
        'LocalDebuggerCommandArguments': "--path C:/Godot/Projects/VoxelGame/VoxelGameDemo --debug --remote-debug tcp://127.0.0.1:6007",
    }

    proj = env2.MSVSProject(
        target = '#VoxelGame' + env2['MSVSPROJECTSUFFIX'],
        incs = str_headers,
        srcs = str_sources,
        variant = ['Release'],
        buildtarget = [target],
        DebugSettings = [debug_settings],
    )
        
    Default(proj)
    
    # Remeber to change the build command to the scons with all arguments except vs_proj
    
else:
    Default(library)