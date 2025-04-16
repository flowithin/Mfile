UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE
    LIBFSCLIENT=libfs_client_macos.o
    LIBFSSERVER=libfs_server_macos.o
    BOOST_THREAD=boost_thread-mt
else
    CC=g++
    LIBFSCLIENT=libfs_client.o
    LIBFSSERVER=libfs_server.o
    BOOST_THREAD=boost_thread
endif

CC+=-g -Wall -std=c++17 -Wno-deprecated-declarations

# List of source files for your file server
FS_SOURCES=server.cpp disk.cpp server_run.cpp

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:.cpp=.o}

TESTS=${patsubst %.cpp, %, $(wildcard test*.cpp)}
all: fs $(TESTS)

# Compile the file server and tag this compilation
#
# Remember to set the CPLUS_INCLUDE_PATH, LIBRARY_PATH, and LD_LIBRARY_PATH
# environment variables to include your Boost installation directory.
fs: ${FS_OBJS} ${LIBFSSERVER}
	./autotag.sh 
	${CC} -o $@ $^ -l${BOOST_THREAD} -lboost_system -lboost_regex -pthread -ldl

%: %.cpp ${LIBVMAPP}
	${CC} -o $@ $^ ${LIBFSCLIENT} -ldl 

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${FS_OBJS} fs test1
