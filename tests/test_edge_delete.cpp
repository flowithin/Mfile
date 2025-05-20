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
    const char* user_invalid = "u2";
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    // delete existing file
    status = fs_delete(user_valid, "/dir/f1");
    check(!status); // success

    // delete non-existing file
    status = fs_delete(user_valid, "/dir/fdd1");
    check(status!=0); // fail

    // delete non-empty dir
    assert(!fs_create(user_valid, "/dirnonempty", 'd'));
    assert(!fs_create(user_valid, "/dirnonempty/filee", 'f'));
    status = fs_delete(user_valid, "dirnonempty");
    check(status!=0); // fail

    // delete invalid user
    status = fs_delete(user_invalid, "dir");
    check(status!=0); // fail


    // delete empty dir
    status = fs_delete(user_valid, "dir");
    check(!status); // success

    // dir shrink
    assert(!fs_create(user_valid, "/dirnonempty/file", 'f'));
    assert(!fs_create(user_valid, "/dirnonempty/fileee", 'f'));
    assert(!fs_delete(user_valid, "/dirnonempty/file"));
    assert(!fs_delete(user_valid, "/dirnonempty/filee"));
    assert(!fs_delete(user_valid, "/dirnonempty/fileee"));

}