# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Asimov\Desktop\develop\conways-opencl

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Asimov\Desktop\develop\conways-opencl\build

# Include any dependencies generated for this target.
include CMakeFiles/gol_opencl.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/gol_opencl.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/gol_opencl.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/gol_opencl.dir/flags.make

CMakeFiles/gol_opencl.dir/codegen:
.PHONY : CMakeFiles/gol_opencl.dir/codegen

CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj: CMakeFiles/gol_opencl.dir/flags.make
CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj: CMakeFiles/gol_opencl.dir/includes_CXX.rsp
CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj: C:/Users/Asimov/Desktop/develop/conways-opencl/src/gol_opencl.cpp
CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj: CMakeFiles/gol_opencl.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\Asimov\Desktop\develop\conways-opencl\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj"
	C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj -MF CMakeFiles\gol_opencl.dir\src\gol_opencl.cpp.obj.d -o CMakeFiles\gol_opencl.dir\src\gol_opencl.cpp.obj -c C:\Users\Asimov\Desktop\develop\conways-opencl\src\gol_opencl.cpp

CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.i"
	C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Asimov\Desktop\develop\conways-opencl\src\gol_opencl.cpp > CMakeFiles\gol_opencl.dir\src\gol_opencl.cpp.i

CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.s"
	C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Asimov\Desktop\develop\conways-opencl\src\gol_opencl.cpp -o CMakeFiles\gol_opencl.dir\src\gol_opencl.cpp.s

# Object files for target gol_opencl
gol_opencl_OBJECTS = \
"CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj"

# External object files for target gol_opencl
gol_opencl_EXTERNAL_OBJECTS =

gol_opencl.exe: CMakeFiles/gol_opencl.dir/src/gol_opencl.cpp.obj
gol_opencl.exe: CMakeFiles/gol_opencl.dir/build.make
gol_opencl.exe: C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v12.6/lib/x64/OpenCL.lib
gol_opencl.exe: CMakeFiles/gol_opencl.dir/linkLibs.rsp
gol_opencl.exe: CMakeFiles/gol_opencl.dir/objects1.rsp
gol_opencl.exe: CMakeFiles/gol_opencl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\Asimov\Desktop\develop\conways-opencl\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable gol_opencl.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\gol_opencl.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/gol_opencl.dir/build: gol_opencl.exe
.PHONY : CMakeFiles/gol_opencl.dir/build

CMakeFiles/gol_opencl.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\gol_opencl.dir\cmake_clean.cmake
.PHONY : CMakeFiles/gol_opencl.dir/clean

CMakeFiles/gol_opencl.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\Asimov\Desktop\develop\conways-opencl C:\Users\Asimov\Desktop\develop\conways-opencl C:\Users\Asimov\Desktop\develop\conways-opencl\build C:\Users\Asimov\Desktop\develop\conways-opencl\build C:\Users\Asimov\Desktop\develop\conways-opencl\build\CMakeFiles\gol_opencl.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/gol_opencl.dir/depend

