# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/cmake-gui

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/espinm2/Code/adv_gfx_repo/hw00/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/espinm2/Code/adv_gfx_repo/hw00/build

# Include any dependencies generated for this target.
include CMakeFiles/ifs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ifs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ifs.dir/flags.make

CMakeFiles/ifs.dir/main.cpp.o: CMakeFiles/ifs.dir/flags.make
CMakeFiles/ifs.dir/main.cpp.o: /home/espinm2/Code/adv_gfx_repo/hw00/src/main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/espinm2/Code/adv_gfx_repo/hw00/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ifs.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -o CMakeFiles/ifs.dir/main.cpp.o -c /home/espinm2/Code/adv_gfx_repo/hw00/src/main.cpp

CMakeFiles/ifs.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ifs.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -E /home/espinm2/Code/adv_gfx_repo/hw00/src/main.cpp > CMakeFiles/ifs.dir/main.cpp.i

CMakeFiles/ifs.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ifs.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -S /home/espinm2/Code/adv_gfx_repo/hw00/src/main.cpp -o CMakeFiles/ifs.dir/main.cpp.s

CMakeFiles/ifs.dir/main.cpp.o.requires:
.PHONY : CMakeFiles/ifs.dir/main.cpp.o.requires

CMakeFiles/ifs.dir/main.cpp.o.provides: CMakeFiles/ifs.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/ifs.dir/build.make CMakeFiles/ifs.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/ifs.dir/main.cpp.o.provides

CMakeFiles/ifs.dir/main.cpp.o.provides.build: CMakeFiles/ifs.dir/main.cpp.o

CMakeFiles/ifs.dir/glCanvas.cpp.o: CMakeFiles/ifs.dir/flags.make
CMakeFiles/ifs.dir/glCanvas.cpp.o: /home/espinm2/Code/adv_gfx_repo/hw00/src/glCanvas.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/espinm2/Code/adv_gfx_repo/hw00/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ifs.dir/glCanvas.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -o CMakeFiles/ifs.dir/glCanvas.cpp.o -c /home/espinm2/Code/adv_gfx_repo/hw00/src/glCanvas.cpp

CMakeFiles/ifs.dir/glCanvas.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ifs.dir/glCanvas.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -E /home/espinm2/Code/adv_gfx_repo/hw00/src/glCanvas.cpp > CMakeFiles/ifs.dir/glCanvas.cpp.i

CMakeFiles/ifs.dir/glCanvas.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ifs.dir/glCanvas.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -S /home/espinm2/Code/adv_gfx_repo/hw00/src/glCanvas.cpp -o CMakeFiles/ifs.dir/glCanvas.cpp.s

CMakeFiles/ifs.dir/glCanvas.cpp.o.requires:
.PHONY : CMakeFiles/ifs.dir/glCanvas.cpp.o.requires

CMakeFiles/ifs.dir/glCanvas.cpp.o.provides: CMakeFiles/ifs.dir/glCanvas.cpp.o.requires
	$(MAKE) -f CMakeFiles/ifs.dir/build.make CMakeFiles/ifs.dir/glCanvas.cpp.o.provides.build
.PHONY : CMakeFiles/ifs.dir/glCanvas.cpp.o.provides

CMakeFiles/ifs.dir/glCanvas.cpp.o.provides.build: CMakeFiles/ifs.dir/glCanvas.cpp.o

CMakeFiles/ifs.dir/camera.cpp.o: CMakeFiles/ifs.dir/flags.make
CMakeFiles/ifs.dir/camera.cpp.o: /home/espinm2/Code/adv_gfx_repo/hw00/src/camera.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/espinm2/Code/adv_gfx_repo/hw00/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ifs.dir/camera.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -o CMakeFiles/ifs.dir/camera.cpp.o -c /home/espinm2/Code/adv_gfx_repo/hw00/src/camera.cpp

CMakeFiles/ifs.dir/camera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ifs.dir/camera.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -E /home/espinm2/Code/adv_gfx_repo/hw00/src/camera.cpp > CMakeFiles/ifs.dir/camera.cpp.i

CMakeFiles/ifs.dir/camera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ifs.dir/camera.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -S /home/espinm2/Code/adv_gfx_repo/hw00/src/camera.cpp -o CMakeFiles/ifs.dir/camera.cpp.s

CMakeFiles/ifs.dir/camera.cpp.o.requires:
.PHONY : CMakeFiles/ifs.dir/camera.cpp.o.requires

CMakeFiles/ifs.dir/camera.cpp.o.provides: CMakeFiles/ifs.dir/camera.cpp.o.requires
	$(MAKE) -f CMakeFiles/ifs.dir/build.make CMakeFiles/ifs.dir/camera.cpp.o.provides.build
.PHONY : CMakeFiles/ifs.dir/camera.cpp.o.provides

CMakeFiles/ifs.dir/camera.cpp.o.provides.build: CMakeFiles/ifs.dir/camera.cpp.o

CMakeFiles/ifs.dir/ifs.cpp.o: CMakeFiles/ifs.dir/flags.make
CMakeFiles/ifs.dir/ifs.cpp.o: /home/espinm2/Code/adv_gfx_repo/hw00/src/ifs.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/espinm2/Code/adv_gfx_repo/hw00/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ifs.dir/ifs.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -o CMakeFiles/ifs.dir/ifs.cpp.o -c /home/espinm2/Code/adv_gfx_repo/hw00/src/ifs.cpp

CMakeFiles/ifs.dir/ifs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ifs.dir/ifs.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -E /home/espinm2/Code/adv_gfx_repo/hw00/src/ifs.cpp > CMakeFiles/ifs.dir/ifs.cpp.i

CMakeFiles/ifs.dir/ifs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ifs.dir/ifs.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -g -Wall -pedantic -S /home/espinm2/Code/adv_gfx_repo/hw00/src/ifs.cpp -o CMakeFiles/ifs.dir/ifs.cpp.s

CMakeFiles/ifs.dir/ifs.cpp.o.requires:
.PHONY : CMakeFiles/ifs.dir/ifs.cpp.o.requires

CMakeFiles/ifs.dir/ifs.cpp.o.provides: CMakeFiles/ifs.dir/ifs.cpp.o.requires
	$(MAKE) -f CMakeFiles/ifs.dir/build.make CMakeFiles/ifs.dir/ifs.cpp.o.provides.build
.PHONY : CMakeFiles/ifs.dir/ifs.cpp.o.provides

CMakeFiles/ifs.dir/ifs.cpp.o.provides.build: CMakeFiles/ifs.dir/ifs.cpp.o

# Object files for target ifs
ifs_OBJECTS = \
"CMakeFiles/ifs.dir/main.cpp.o" \
"CMakeFiles/ifs.dir/glCanvas.cpp.o" \
"CMakeFiles/ifs.dir/camera.cpp.o" \
"CMakeFiles/ifs.dir/ifs.cpp.o"

# External object files for target ifs
ifs_EXTERNAL_OBJECTS =

ifs: CMakeFiles/ifs.dir/main.cpp.o
ifs: CMakeFiles/ifs.dir/glCanvas.cpp.o
ifs: CMakeFiles/ifs.dir/camera.cpp.o
ifs: CMakeFiles/ifs.dir/ifs.cpp.o
ifs: /usr/lib/x86_64-linux-gnu/libGLU.so
ifs: /usr/lib/x86_64-linux-gnu/libGL.so
ifs: /usr/lib/x86_64-linux-gnu/libSM.so
ifs: /usr/lib/x86_64-linux-gnu/libICE.so
ifs: /usr/lib/x86_64-linux-gnu/libX11.so
ifs: /usr/lib/x86_64-linux-gnu/libXext.so
ifs: /usr/lib/x86_64-linux-gnu/libGLEW.so
ifs: /usr/lib/x86_64-linux-gnu/libGL.so
ifs: /usr/lib/x86_64-linux-gnu/libGLEW.so
ifs: /usr/lib/x86_64-linux-gnu/libSM.so
ifs: /usr/lib/x86_64-linux-gnu/libICE.so
ifs: /usr/lib/x86_64-linux-gnu/libX11.so
ifs: /usr/lib/x86_64-linux-gnu/libXext.so
ifs: CMakeFiles/ifs.dir/build.make
ifs: CMakeFiles/ifs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ifs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ifs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ifs.dir/build: ifs
.PHONY : CMakeFiles/ifs.dir/build

CMakeFiles/ifs.dir/requires: CMakeFiles/ifs.dir/main.cpp.o.requires
CMakeFiles/ifs.dir/requires: CMakeFiles/ifs.dir/glCanvas.cpp.o.requires
CMakeFiles/ifs.dir/requires: CMakeFiles/ifs.dir/camera.cpp.o.requires
CMakeFiles/ifs.dir/requires: CMakeFiles/ifs.dir/ifs.cpp.o.requires
.PHONY : CMakeFiles/ifs.dir/requires

CMakeFiles/ifs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ifs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ifs.dir/clean

CMakeFiles/ifs.dir/depend:
	cd /home/espinm2/Code/adv_gfx_repo/hw00/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espinm2/Code/adv_gfx_repo/hw00/src /home/espinm2/Code/adv_gfx_repo/hw00/src /home/espinm2/Code/adv_gfx_repo/hw00/build /home/espinm2/Code/adv_gfx_repo/hw00/build /home/espinm2/Code/adv_gfx_repo/hw00/build/CMakeFiles/ifs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ifs.dir/depend

