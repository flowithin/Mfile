#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

void check(bool status) {
    if (!status) {
        std::cout << "FAILLL" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    char* server;
    int server_port;
    const char* user_valid = "u1";
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    // create with path ending with /
    status = fs_create(user_valid, "/dir", 'd');
    check(!status); // success
    status = fs_create(user_valid, "/dir/", 'd');
    check(status !=0); // fail

    // create existing file
    status = fs_create(user_valid, "/dir/f1", 'f');
    check(!status); // success
    status = fs_create(user_valid, "/dir/f1", 'f');
    check(status !=0); // fail

    // create in non-existing dir
    status = fs_create(user_valid, "/dir1/f1", 'f');
    check(status !=0); // fail

    // create invalid filenames
    status = fs_create(user_valid, "/dir/f/1", 'f');
    check(status !=0); // fail
    status = fs_create(user_valid, "/dir/f 1", 'f');
    check(status !=0); // fail

}