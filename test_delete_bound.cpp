#include <cstdlib>
#include <iostream>

#include "fs_client.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server> <serverPort>\n";
        return 1;
    }

    char* server = argv[1];
    int server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);


    fs_create("u1", "/delete_test", 'd');
    fs_create("u1", "/delete_test/file.txt", 'f');
    // assume this structure exists from test_create
    fs_delete("u1", "/delete_test/file.txt");

    // delete again (should fail)
    fs_delete("u1", "/delete_test/file.txt");

    // delete file in subdir
    fs_delete("u1", "/delete_test/subdir/deep.txt");

    // delete subdir and parent dir
    fs_delete("u1", "/delete_test/subdir");
    fs_delete("u1", "/delete_test");

    // try deleting non-existent paths
    fs_delete("u1", "/delete_test/ghost.txt");
    fs_delete("u2", "/delete_test");   // wrong user

    return 0;
}
