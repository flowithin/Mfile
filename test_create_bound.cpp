#include <cstdlib>
#include <iostream>

#include "fs_client.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    char* server = argv[1];
    int server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);

    // directory and file creation
    fs_create("u1", "/create_test", 'd');
    fs_create("u1", "/create_test/file.txt", 'f');

    // recreate existing
    fs_create("u1", "/create_test/file.txt", 'f');

    // create with wrong user
    fs_create("user2", "/create_test/other.txt", 'f');

    // nested directory and file
    fs_create("u1", "/create_test/subdir", 'd');
    fs_create("u1", "/create_test/subdir/deep.txt", 'f');

    return 0;
}
