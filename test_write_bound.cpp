#include <cstdlib>
#include <cstring>
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

    char readdata[FS_BLOCKSIZE];
    char full_block[FS_BLOCKSIZE];

    const char* msg
      = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their "
        "Creator with certain unalienable Rights, that among ";
    for (int i = 0; i < FS_BLOCKSIZE; ++i) {
        full_block[i] = 'A' + (i % 26);   // A-Z cycle
    }

    fs_create("u1", "/comb", 'd');
    fs_create("u1", "/comb/file.txt", 'f');

    // valid write
    fs_writeblock("u1", "/comb/file.txt", 0, msg);

    // invalid writes
    fs_writeblock("u2", "/comb/file.txt", 0, msg);                      // wrong user
    fs_writeblock("u1", "/comb", 0, msg);                               // not a file
    fs_writeblock("u1", "/comb/file.txt", FS_MAXFILEBLOCKS + 1, msg);   // out of bounds

    // write full block with no null terminator
    fs_writeblock("u1", "/comb/file.txt", 1, full_block);

    // read and print first 50
    if (fs_readblock("u1", "/comb/file.txt", 0, readdata) == 0) {
        std::cout << "\nBlock 0 (text): ";
        std::cout.write(readdata, 50);
        std::cout << '\n';
    }

    if (fs_readblock("u1", "/comb/file.txt", 1, readdata) == 0) {
        std::cout << "Block 1 (no null): ";
        std::cout.write(readdata, 50);
        std::cout << '\n';
    }

    // read a block that's out of range
    fs_readblock("u1", "/comb/file.txt", 124, readdata);

    return 0;
}
