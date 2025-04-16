#include <cstring>
#include <fs_param.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <string>
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
  //filename root 
    status = fs_create("user1", "/", 'd');
  //filename not valid
    status = fs_create("user1", "/dir/", 'd');
  //not found
    status = fs_create("user1", "/dir/file", 'd');
  //username incorrect
    status = fs_create("", "/dir", 'd');
  //correct 
    status = fs_create("user1", "/dir", 'd');
    status = fs_create("user1", "/dir/file", 'd');
  //incorrect username
    status = fs_create("user2", "/dir/file", 'd');
  //excceed file max block number
  for(int i=0; i < FS_MAXFILEBLOCKS * 8; i++){
    std::stringstream ss;
    ss << "/dir" << i;
    status = fs_create("user2", ss.str().c_str(), 'f');
  }


}
