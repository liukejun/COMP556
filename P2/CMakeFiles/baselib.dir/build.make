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
include CMakeFiles/baselib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/baselib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/baselib.dir/flags.make

CMakeFiles/baselib.dir/ReceiverWindow.cc.o: CMakeFiles/baselib.dir/flags.make
CMakeFiles/baselib.dir/ReceiverWindow.cc.o: ReceiverWindow.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/baselib.dir/ReceiverWindow.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/baselib.dir/ReceiverWindow.cc.o -c "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/ReceiverWindow.cc"

CMakeFiles/baselib.dir/ReceiverWindow.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/baselib.dir/ReceiverWindow.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/ReceiverWindow.cc" > CMakeFiles/baselib.dir/ReceiverWindow.cc.i

CMakeFiles/baselib.dir/ReceiverWindow.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/baselib.dir/ReceiverWindow.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/ReceiverWindow.cc" -o CMakeFiles/baselib.dir/ReceiverWindow.cc.s

CMakeFiles/baselib.dir/ReceiverWindow.cc.o.requires:

.PHONY : CMakeFiles/baselib.dir/ReceiverWindow.cc.o.requires

CMakeFiles/baselib.dir/ReceiverWindow.cc.o.provides: CMakeFiles/baselib.dir/ReceiverWindow.cc.o.requires
	$(MAKE) -f CMakeFiles/baselib.dir/build.make CMakeFiles/baselib.dir/ReceiverWindow.cc.o.provides.build
.PHONY : CMakeFiles/baselib.dir/ReceiverWindow.cc.o.provides

CMakeFiles/baselib.dir/ReceiverWindow.cc.o.provides.build: CMakeFiles/baselib.dir/ReceiverWindow.cc.o


CMakeFiles/baselib.dir/SenderWindow.cc.o: CMakeFiles/baselib.dir/flags.make
CMakeFiles/baselib.dir/SenderWindow.cc.o: SenderWindow.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/baselib.dir/SenderWindow.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/baselib.dir/SenderWindow.cc.o -c "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/SenderWindow.cc"

CMakeFiles/baselib.dir/SenderWindow.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/baselib.dir/SenderWindow.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/SenderWindow.cc" > CMakeFiles/baselib.dir/SenderWindow.cc.i

CMakeFiles/baselib.dir/SenderWindow.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/baselib.dir/SenderWindow.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/SenderWindow.cc" -o CMakeFiles/baselib.dir/SenderWindow.cc.s

CMakeFiles/baselib.dir/SenderWindow.cc.o.requires:

.PHONY : CMakeFiles/baselib.dir/SenderWindow.cc.o.requires

CMakeFiles/baselib.dir/SenderWindow.cc.o.provides: CMakeFiles/baselib.dir/SenderWindow.cc.o.requires
	$(MAKE) -f CMakeFiles/baselib.dir/build.make CMakeFiles/baselib.dir/SenderWindow.cc.o.provides.build
.PHONY : CMakeFiles/baselib.dir/SenderWindow.cc.o.provides

CMakeFiles/baselib.dir/SenderWindow.cc.o.provides.build: CMakeFiles/baselib.dir/SenderWindow.cc.o


CMakeFiles/baselib.dir/Slot.cc.o: CMakeFiles/baselib.dir/flags.make
CMakeFiles/baselib.dir/Slot.cc.o: Slot.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/baselib.dir/Slot.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/baselib.dir/Slot.cc.o -c "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Slot.cc"

CMakeFiles/baselib.dir/Slot.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/baselib.dir/Slot.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Slot.cc" > CMakeFiles/baselib.dir/Slot.cc.i

CMakeFiles/baselib.dir/Slot.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/baselib.dir/Slot.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Slot.cc" -o CMakeFiles/baselib.dir/Slot.cc.s

CMakeFiles/baselib.dir/Slot.cc.o.requires:

.PHONY : CMakeFiles/baselib.dir/Slot.cc.o.requires

CMakeFiles/baselib.dir/Slot.cc.o.provides: CMakeFiles/baselib.dir/Slot.cc.o.requires
	$(MAKE) -f CMakeFiles/baselib.dir/build.make CMakeFiles/baselib.dir/Slot.cc.o.provides.build
.PHONY : CMakeFiles/baselib.dir/Slot.cc.o.provides

CMakeFiles/baselib.dir/Slot.cc.o.provides.build: CMakeFiles/baselib.dir/Slot.cc.o


CMakeFiles/baselib.dir/Window.cc.o: CMakeFiles/baselib.dir/flags.make
CMakeFiles/baselib.dir/Window.cc.o: Window.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/baselib.dir/Window.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/baselib.dir/Window.cc.o -c "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Window.cc"

CMakeFiles/baselib.dir/Window.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/baselib.dir/Window.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Window.cc" > CMakeFiles/baselib.dir/Window.cc.i

CMakeFiles/baselib.dir/Window.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/baselib.dir/Window.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/Window.cc" -o CMakeFiles/baselib.dir/Window.cc.s

CMakeFiles/baselib.dir/Window.cc.o.requires:

.PHONY : CMakeFiles/baselib.dir/Window.cc.o.requires

CMakeFiles/baselib.dir/Window.cc.o.provides: CMakeFiles/baselib.dir/Window.cc.o.requires
	$(MAKE) -f CMakeFiles/baselib.dir/build.make CMakeFiles/baselib.dir/Window.cc.o.provides.build
.PHONY : CMakeFiles/baselib.dir/Window.cc.o.provides

CMakeFiles/baselib.dir/Window.cc.o.provides.build: CMakeFiles/baselib.dir/Window.cc.o


# Object files for target baselib
baselib_OBJECTS = \
"CMakeFiles/baselib.dir/ReceiverWindow.cc.o" \
"CMakeFiles/baselib.dir/SenderWindow.cc.o" \
"CMakeFiles/baselib.dir/Slot.cc.o" \
"CMakeFiles/baselib.dir/Window.cc.o"

# External object files for target baselib
baselib_EXTERNAL_OBJECTS =

libbaselib.a: CMakeFiles/baselib.dir/ReceiverWindow.cc.o
libbaselib.a: CMakeFiles/baselib.dir/SenderWindow.cc.o
libbaselib.a: CMakeFiles/baselib.dir/Slot.cc.o
libbaselib.a: CMakeFiles/baselib.dir/Window.cc.o
libbaselib.a: CMakeFiles/baselib.dir/build.make
libbaselib.a: CMakeFiles/baselib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libbaselib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/baselib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/baselib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/baselib.dir/build: libbaselib.a

.PHONY : CMakeFiles/baselib.dir/build

CMakeFiles/baselib.dir/requires: CMakeFiles/baselib.dir/ReceiverWindow.cc.o.requires
CMakeFiles/baselib.dir/requires: CMakeFiles/baselib.dir/SenderWindow.cc.o.requires
CMakeFiles/baselib.dir/requires: CMakeFiles/baselib.dir/Slot.cc.o.requires
CMakeFiles/baselib.dir/requires: CMakeFiles/baselib.dir/Window.cc.o.requires

.PHONY : CMakeFiles/baselib.dir/requires

CMakeFiles/baselib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/baselib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/baselib.dir/clean

CMakeFiles/baselib.dir/depend:
	cd "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2" "/Users/shuozhao/Downloads/Courses/COMP 556 Network/group_projects_new/RGA208/P2/CMakeFiles/baselib.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/baselib.dir/depend
