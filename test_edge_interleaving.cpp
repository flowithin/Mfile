#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

int main(int argc, char* argv[]) {
    // assert usage
    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }

    // get server and port and initialize client
    char* server = argv[1];
    const char* user = "user1";
    int server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);

    // initialize
    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char filename[FS_MAXFILENAME + 1];
    assert(fs_create(user, "/dir", 'd') == 0);

    // create -> delete -> reuse
    for (size_t i = 0; i < 100; ++i) {
        snprintf(filename, sizeof(filename), "/dir/f%zu", i);
        assert(fs_create(user, filename, 'f') == 0);
        assert(fs_writeblock(user, filename, 0, writedata) == 0);
        // delete half
        if (i % 2 == 0) {
            assert(fs_delete(user, filename) == 0);
        }
        // remaining 1, 3, 5, ...
    }

    // delayed delete
    for (size_t i = 0; i < 100; ++i) {
        if (i % 4 == 1){ // 1, 5, 9, ...
            snprintf(filename, sizeof(filename), "/dir/fd%zu", i);
            assert(fs_create(user, filename, 'f') == 0);
            assert(fs_writeblock(user, filename, 0, writedata) == 0);

            // delete original ones
            snprintf(filename, sizeof(filename), "/dir/f%zu", i);
            assert(fs_delete(user, filename) == 0);
        }
        // remaining f3, 7, 11... | fd1, 5, 9...
    }

    // clean up
    for (size_t i = 0; i < 100; ++i) {
        if (i % 4 == 1) {
            snprintf(filename, sizeof(filename), "/dir/fd%zu", i);
            assert(fs_delete(user, filename) == 0);
        }
        if (i % 4 == 3) {
            snprintf(filename, sizeof(filename), "/dir/f%zu", i);
            assert(fs_delete(user, filename) == 0);
        }
    }

    return 0;
}