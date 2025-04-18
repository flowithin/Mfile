#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

int main(int argc, char* argv[]) {
    char* server;
    int server_port;
    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    // start with a full dir

    char filename[64];
    int count = 0;

    while (true) {
        snprintf(filename, sizeof(filename), "/dir/f%d", count);
        int status = fs_create("user1", filename, 'f');
        if (status != 0) {
            break;  // should break at first it
        }
        count++;
    }
    std::cout << count << std::endl;

    return 0;
}