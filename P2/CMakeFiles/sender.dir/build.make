# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.9.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.9.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2"

# Include any dependencies generated for this target.
include CMakeFiles/sender.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sender.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sender.dir/flags.make

CMakeFiles/sender.dir/sendFile.cpp.o: CMakeFiles/sender.dir/flags.make
CMakeFiles/sender.dir/sendFile.cpp.o: sendFile.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sender.dir/sendFile.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sender.dir/sendFile.cpp.o -c "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/sendFile.cpp"

CMakeFiles/sender.dir/sendFile.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sender.dir/sendFile.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/sendFile.cpp" > CMakeFiles/sender.dir/sendFile.cpp.i

CMakeFiles/sender.dir/sendFile.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sender.dir/sendFile.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/sendFile.cpp" -o CMakeFiles/sender.dir/sendFile.cpp.s

CMakeFiles/sender.dir/sendFile.cpp.o.requires:

.PHONY : CMakeFiles/sender.dir/sendFile.cpp.o.requires

CMakeFiles/sender.dir/sendFile.cpp.o.provides: CMakeFiles/sender.dir/sendFile.cpp.o.requires
	$(MAKE) -f CMakeFiles/sender.dir/build.make CMakeFiles/sender.dir/sendFile.cpp.o.provides.build
.PHONY : CMakeFiles/sender.dir/sendFile.cpp.o.provides

CMakeFiles/sender.dir/sendFile.cpp.o.provides.build: CMakeFiles/sender.dir/sendFile.cpp.o


# Object files for target sender
sender_OBJECTS = \
"CMakeFiles/sender.dir/sendFile.cpp.o"

# External object files for target sender
sender_EXTERNAL_OBJECTS =

sender: CMakeFiles/sender.dir/sendFile.cpp.o
sender: CMakeFiles/sender.dir/build.make
sender: libbaselib.a
sender: CMakeFiles/sender.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable sender"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sender.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sender.dir/build: sender

.PHONY : CMakeFiles/sender.dir/build

CMakeFiles/sender.dir/requires: CMakeFiles/sender.dir/sendFile.cpp.o.requires

.PHONY : CMakeFiles/sender.dir/requires

CMakeFiles/sender.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sender.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sender.dir/clean

CMakeFiles/sender.dir/depend:
	cd "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles/sender.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/sender.dir/depend
