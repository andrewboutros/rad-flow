# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.21.0-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.21.0-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gtrieu/rad-flow/rad-sim/test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gtrieu/rad-flow/rad-sim/test

# Include any dependencies generated for this target.
include CMakeFiles/test_axis_adapters.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_axis_adapters.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_axis_adapters.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_axis_adapters.dir/flags.make

CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o: CMakeFiles/test_axis_adapters.dir/flags.make
CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o: axis_adapters_noc_test.cpp
CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o: CMakeFiles/test_axis_adapters.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gtrieu/rad-flow/rad-sim/test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o -MF CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o.d -o CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o -c /home/gtrieu/rad-flow/rad-sim/test/axis_adapters_noc_test.cpp

CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gtrieu/rad-flow/rad-sim/test/axis_adapters_noc_test.cpp > CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.i

CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gtrieu/rad-flow/rad-sim/test/axis_adapters_noc_test.cpp -o CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.s

# Object files for target test_axis_adapters
test_axis_adapters_OBJECTS = \
"CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o"

# External object files for target test_axis_adapters
test_axis_adapters_EXTERNAL_OBJECTS =

test_axis_adapters: CMakeFiles/test_axis_adapters.dir/axis_adapters_noc_test.cpp.o
test_axis_adapters: CMakeFiles/test_axis_adapters.dir/build.make
test_axis_adapters: /home/gtrieu/systemc-2.3.4/build/src/libsystemc.so.2.3.4
test_axis_adapters: CMakeFiles/test_axis_adapters.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gtrieu/rad-flow/rad-sim/test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test_axis_adapters"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_axis_adapters.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_axis_adapters.dir/build: test_axis_adapters
.PHONY : CMakeFiles/test_axis_adapters.dir/build

CMakeFiles/test_axis_adapters.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_axis_adapters.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_axis_adapters.dir/clean

CMakeFiles/test_axis_adapters.dir/depend:
	cd /home/gtrieu/rad-flow/rad-sim/test && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gtrieu/rad-flow/rad-sim/test /home/gtrieu/rad-flow/rad-sim/test /home/gtrieu/rad-flow/rad-sim/test /home/gtrieu/rad-flow/rad-sim/test /home/gtrieu/rad-flow/rad-sim/test/CMakeFiles/test_axis_adapters.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_axis_adapters.dir/depend
