#include <fs_param.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);


    fs_clientinit(server, server_port);
  //should occupy the file system
  for(int j=0; j < 8 * FS_MAXFILEBLOCKS; j++){
    std::stringstream ss;
    ss << "/dir" << j;
    status = fs_create("user2", ss.str().c_str(), 'f');
  }
  for(int i=0; i < 4096 - 8* FS_MAXFILEBLOCKS - 1; i++){
    std::stringstream ss;
    ss << "/dir" << i;
    std::stringstream ss2(ss.str());
  for(int j=0; j < 8 * FS_MAXFILEBLOCKS; j++){
    std::stringstream ss;
    ss2 << "/dir" << j;
    status = fs_create("user2", ss2.str().c_str(), 'f');
  }

  }


}
