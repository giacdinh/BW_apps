# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/giacdinh/yocto/test/BW_apps/1_6_0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/giacdinh/yocto/test/BW_apps/1_6_0/build

# Include any dependencies generated for this target.
include capture/CMakeFiles/gst_capture.dir/depend.make

# Include the progress variables for this target.
include capture/CMakeFiles/gst_capture.dir/progress.make

# Include the compile flags for this target's objects.
include capture/CMakeFiles/gst_capture.dir/flags.make

capture/CMakeFiles/gst_capture.dir/capture.c.o: capture/CMakeFiles/gst_capture.dir/flags.make
capture/CMakeFiles/gst_capture.dir/capture.c.o: ../capture/capture.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/giacdinh/yocto/test/BW_apps/1_6_0/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object capture/CMakeFiles/gst_capture.dir/capture.c.o"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gst_capture.dir/capture.c.o   -c /home/giacdinh/yocto/test/BW_apps/1_6_0/capture/capture.c

capture/CMakeFiles/gst_capture.dir/capture.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gst_capture.dir/capture.c.i"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -E /home/giacdinh/yocto/test/BW_apps/1_6_0/capture/capture.c > CMakeFiles/gst_capture.dir/capture.c.i

capture/CMakeFiles/gst_capture.dir/capture.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gst_capture.dir/capture.c.s"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -S /home/giacdinh/yocto/test/BW_apps/1_6_0/capture/capture.c -o CMakeFiles/gst_capture.dir/capture.c.s

capture/CMakeFiles/gst_capture.dir/capture.c.o.requires:
.PHONY : capture/CMakeFiles/gst_capture.dir/capture.c.o.requires

capture/CMakeFiles/gst_capture.dir/capture.c.o.provides: capture/CMakeFiles/gst_capture.dir/capture.c.o.requires
	$(MAKE) -f capture/CMakeFiles/gst_capture.dir/build.make capture/CMakeFiles/gst_capture.dir/capture.c.o.provides.build
.PHONY : capture/CMakeFiles/gst_capture.dir/capture.c.o.provides

capture/CMakeFiles/gst_capture.dir/capture.c.o.provides.build: capture/CMakeFiles/gst_capture.dir/capture.c.o

# Object files for target gst_capture
gst_capture_OBJECTS = \
"CMakeFiles/gst_capture.dir/capture.c.o"

# External object files for target gst_capture
gst_capture_EXTERNAL_OBJECTS =

capture/gst_capture: capture/CMakeFiles/gst_capture.dir/capture.c.o
capture/gst_capture: capture/CMakeFiles/gst_capture.dir/build.make
capture/gst_capture: capture/libcapture.a
capture/gst_capture: capture/CMakeFiles/gst_capture.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable gst_capture"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gst_capture.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
capture/CMakeFiles/gst_capture.dir/build: capture/gst_capture
.PHONY : capture/CMakeFiles/gst_capture.dir/build

capture/CMakeFiles/gst_capture.dir/requires: capture/CMakeFiles/gst_capture.dir/capture.c.o.requires
.PHONY : capture/CMakeFiles/gst_capture.dir/requires

capture/CMakeFiles/gst_capture.dir/clean:
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture && $(CMAKE_COMMAND) -P CMakeFiles/gst_capture.dir/cmake_clean.cmake
.PHONY : capture/CMakeFiles/gst_capture.dir/clean

capture/CMakeFiles/gst_capture.dir/depend:
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/giacdinh/yocto/test/BW_apps/1_6_0 /home/giacdinh/yocto/test/BW_apps/1_6_0/capture /home/giacdinh/yocto/test/BW_apps/1_6_0/build /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture /home/giacdinh/yocto/test/BW_apps/1_6_0/build/capture/CMakeFiles/gst_capture.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : capture/CMakeFiles/gst_capture.dir/depend
