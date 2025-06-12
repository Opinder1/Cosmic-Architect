<h1>Cosmic Architect</h1>

<h2>A Sci-fi Universe simulation that loads level of detail around the camera.</h2>

The application is built using godot-cpp for cross platform application capabilities. The project uses the scons build system.

<h4>To build in debug:</h4>
scons platform=windows target=template_debug precision=double use_static_cpp=true dev_build=true debug_symbols=true profile=true

<h4>To build in release:</h4>
scons platform=windows target=template_release precision=double use_static_cpp=true optimize=speed lto=full

<h4>To generate a visual studio project:</h4>
scons platform=windows target=template_debug dev_build=true precision=double use_static_cpp=true vs_proj=true debug_symbols=true profile=true

<h4>The following libraries have been used:</h4>
<ul>
<li><a href="https://github.com/estraier/tkrzw">tkrzw</a></li>
<li><a href="https://github.com/yse/easy_profiler">easy profiler</a></li>
<li><a href="https://github.com/fmtlib/fmt">fmt</a></li>
<li><a href="https://github.com/martinus/robin-hood-hashing">robin_hood</li>
</ul>
