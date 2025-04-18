#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "fs_client.h"

void check(bool status) {
    if (!status) {
        std::cout << "FAILLL" << std::endl;
    }
}

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

    const char* user_valid = "u1";
    const char* user_invalid = "u2";

    // setup
    assert(!fs_create(user_valid, "/wtest", 'd'));
    assert(!fs_create(user_valid, "/wtest/file.txt", 'f'));

    const char* payload1 = "Hello EECS 482";
    const char* payload2 = "Byeee eecs 248";
    char readbuf[FS_BLOCKSIZE];
    int status;

    // write over size
    status = fs_writeblock(user_valid, "/wtest/file.txt", 2, payload1);
    check(status != 0);  // fail

    // write invalid user
    status = fs_writeblock(user_invalid, "/wtest/file.txt", 2, payload1);
    check(status != 0);  // fail

    // append
    status = fs_writeblock(user_valid, "/wtest/file.txt", 0, payload1);
    check(status == 0);  // success

    // oob write
    status = fs_writeblock(user_valid, "/wtest/file.txt", 513, payload1);  // max: 512
    check(status != 0);  // fail

    // overwrite
    status = fs_writeblock(user_valid, "/wtest/file.txt", 0, payload2);
    check(status == 0);  // success

    // read over size
    status = fs_readblock(user_valid, "/wtest/file.txt", 1, readbuf);
    check(status != 0);  // fail

    // valid read
    status = fs_readblock(user_valid, "/wtest/file.txt", 0, readbuf);
    check(status == 0);  // success

    // no null
    char raw_block[FS_BLOCKSIZE];
    for (int i = 0; i < FS_BLOCKSIZE; ++i) {
        raw_block[i] = 'X';  // Fill block with raw data
    }

    status = fs_writeblock(user_valid, "/wtest/file.txt", 1, raw_block);
    check(status == 0);

    // read back
    status = fs_readblock(user_valid, "/wtest/file.txt", 1, readbuf);
    check(status == 0);

    // additional check
    bool match = true;
    for (int i = 0; i < 10; ++i) {
        if (readbuf[i] != 'X') {
            match = false;
            break;
        }
    }
    check(match);  // Should succeed

    return 0;
}
