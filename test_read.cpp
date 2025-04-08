#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    char readdata[FS_BLOCKSIZE];
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    status = fs_readblock("user1", "/", 1, readdata);
    assert(!status);
}
