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
include hashfile/CMakeFiles/hashfile.dir/depend.make

# Include the progress variables for this target.
include hashfile/CMakeFiles/hashfile.dir/progress.make

# Include the compile flags for this target's objects.
include hashfile/CMakeFiles/hashfile.dir/flags.make

hashfile/CMakeFiles/hashfile.dir/hashfile.c.o: hashfile/CMakeFiles/hashfile.dir/flags.make
hashfile/CMakeFiles/hashfile.dir/hashfile.c.o: ../hashfile/hashfile.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/giacdinh/yocto/test/BW_apps/1_6_0/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object hashfile/CMakeFiles/hashfile.dir/hashfile.c.o"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/hashfile.dir/hashfile.c.o   -c /home/giacdinh/yocto/test/BW_apps/1_6_0/hashfile/hashfile.c

hashfile/CMakeFiles/hashfile.dir/hashfile.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hashfile.dir/hashfile.c.i"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -E /home/giacdinh/yocto/test/BW_apps/1_6_0/hashfile/hashfile.c > CMakeFiles/hashfile.dir/hashfile.c.i

hashfile/CMakeFiles/hashfile.dir/hashfile.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hashfile.dir/hashfile.c.s"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile && /opt/l3mv-linux/2.1/sysroots/x86_64-l3sdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc  --sysroot=/opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi $(C_DEFINES) $(C_FLAGS) -S /home/giacdinh/yocto/test/BW_apps/1_6_0/hashfile/hashfile.c -o CMakeFiles/hashfile.dir/hashfile.c.s

hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.requires:
.PHONY : hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.requires

hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.provides: hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.requires
	$(MAKE) -f hashfile/CMakeFiles/hashfile.dir/build.make hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.provides.build
.PHONY : hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.provides

hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.provides.build: hashfile/CMakeFiles/hashfile.dir/hashfile.c.o

# Object files for target hashfile
hashfile_OBJECTS = \
"CMakeFiles/hashfile.dir/hashfile.c.o"

# External object files for target hashfile
hashfile_EXTERNAL_OBJECTS =

hashfile/hashfile: hashfile/CMakeFiles/hashfile.dir/hashfile.c.o
hashfile/hashfile: hashfile/CMakeFiles/hashfile.dir/build.make
hashfile/hashfile: lib/libFlash2AppSM.a
hashfile/hashfile: /opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/lib/libssl.so
hashfile/hashfile: /opt/l3mv-linux/2.1/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/lib/libcrypto.so
hashfile/hashfile: hashfile/CMakeFiles/hashfile.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable hashfile"
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hashfile.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
hashfile/CMakeFiles/hashfile.dir/build: hashfile/hashfile
.PHONY : hashfile/CMakeFiles/hashfile.dir/build

hashfile/CMakeFiles/hashfile.dir/requires: hashfile/CMakeFiles/hashfile.dir/hashfile.c.o.requires
.PHONY : hashfile/CMakeFiles/hashfile.dir/requires

hashfile/CMakeFiles/hashfile.dir/clean:
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile && $(CMAKE_COMMAND) -P CMakeFiles/hashfile.dir/cmake_clean.cmake
.PHONY : hashfile/CMakeFiles/hashfile.dir/clean

hashfile/CMakeFiles/hashfile.dir/depend:
	cd /home/giacdinh/yocto/test/BW_apps/1_6_0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/giacdinh/yocto/test/BW_apps/1_6_0 /home/giacdinh/yocto/test/BW_apps/1_6_0/hashfile /home/giacdinh/yocto/test/BW_apps/1_6_0/build /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile /home/giacdinh/yocto/test/BW_apps/1_6_0/build/hashfile/CMakeFiles/hashfile.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : hashfile/CMakeFiles/hashfile.dir/depend

