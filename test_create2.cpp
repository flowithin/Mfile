#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    char readdata[FS_BLOCKSIZE];
    strcpy(readdata, "hellow");
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    /*status = fs_create("user1", "/dir", 'd');*/
    /*assert(!status);*/
  /**/
  /*  status = fs_create("user1", "/dir/file", 'f');*/
  /*  assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2", 'd');*/
  /*  assert(!status);*/
  /*//NOTE:same file name in different dir*/
  /*  status = fs_create("user1", "/dir/dir2/file", 'f');*/
  /*  assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2/dir3", 'd');*/
  /*  assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2/dir3/file2", 'f');*/
  /*  assert(!status);*/
    /*status = fs_create("user2", "/dir22", 'd');*/
    /*assert(!status);*/

    /**/
    /*status = fs_create("user2", "/dir22/file22", 'f');*/
    /*assert(!status);*/
    /*status = fs_create("user2", "/dir22/dir33", 'd');*/
    /*assert(!status);*/
    /*status = fs_create("user2", "/dir22/dir33/file", 'f');*/
    /*assert(!status);*/
    /*status = fs_create("user2", "/dir22/dir33/dir3", 'd');*/
    /*assert(!status);*/
    /*status = fs_create("user2", "/dir22/dir33/dir3/file2", 'f');*/
    /*assert(!status);*/
    status = fs_create("user2", "/file", 'f');
    assert(!status);
    /**/
    /**/


}
