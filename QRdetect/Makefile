# the compiler: gcc for C program, g++ for C++ program
CC = g++

# Include directories for compiling executable
INCLUDE_PATH = -I/usr/include/opencv2 -I/home/oliver/harriscorner/src/Modules/HarrisCorner

# Include path for library


# Compiler flags:
# 	-g 	adds debugging information to the executable file
#	-Wall turns on most, but not all, compiler warnings
CXXFLAGS = -g -Wall -fPIC 

LIBS = `pkg-config opencv --libs` -L/usr/local/lib -lzbar

# The build TARGET executable
TARGET = stitchTest

# Here the OBJS are the libraries used by make
OBJS = stitchimg.o \
	klargestheap.o \
	main.o

SOURCES = main.cpp \
	stitchimg.cpp \
	/home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.cpp 

# Here the REBUILDABLES are files which can be cleaned and rebuilt
REBUILDABLES = $(OBJS) $(TARGET)

# The rule for all is used to incrementally build the system
all: $(TARGET)
	echo All done

# Here is a RULE that uses Make Macros in its command:
#	$@ expants to the rule's target, in this case stitchTest
# 	$^ expands to the rule's dependances, in this case $OBJS
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(INCLUDE_PATH) -o $(TARGET) $(OBJS) $(LIBS)

# Here is a PATTERN RULE, which says how to create a file with a .o suffix,
# given a file with a .cpp suffix.
# It uses Make Macros:
# 	$@ for the pattern-matched target
# 	$< for the pattern-matched dependency
#%.o: %.cpp
#	$(CC) $(CXXFLAGS) -o $@ -c $<

# The dependency rules indicate that if any file to the right of the colon changes,
# the target to the left of the colon should be considered out-of-date.
# The commands for making an out-of-date target up-to-date may be found elsewhere
# (in this case, by the Pattern Rule above)
# Dependency Rules are often used to capture header file dependencies.
#main.o: stitchimg.h
#stitchimg.o: stitchimg.h /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.h
#klargestheap.o: /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.h

main.o: main.cpp
	$(CC) $(CXXFLAGS) $(INCLUDE_PATH) -c main.cpp

stitchimg.o: stitchimg.h stitchimg.cpp /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.cpp
	$(CC) $(CXXFLAGS) $(INCLUDE_PATH) -c stitchimg.cpp

klargestheap.o: /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.cpp /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.h
	$(CC) $(CXXFLAGS) $(INCLUDE_PATH) -c /home/oliver/harriscorner/src/Modules/HarrisCorner/klargestheap.cpp

clean:
	rm -f $(REBUILDABLES)
	echo Clean done

