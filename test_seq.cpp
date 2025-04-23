#include <cstring>
#include "fs_param.h"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    const char* writedata1 = "hhhWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    char readdata[FS_BLOCKSIZE];
    strcpy(readdata, "hellow");
    int status;

    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);
  fs_readblock("user1", "/dir/file.txt", 0,readdata);
  fs_writeblock("user1", "/dir/file.txt", 0,writedata);
  fs_create("user1", "/dir", 'd');
  fs_create("user1", "/dir/file.txt", 'f');
  status = fs_create("user1", "/dir", 'd');
  status = fs_create("user1", "/dir/file.txt", 'f');
  status = fs_writeblock("user1", "/dir/file.txt", 0, writedata);
  status = fs_writeblock("user1", "/dir", 0, writedata);
  status = fs_readblock("user1", "/dir", 0, readdata);
  status = fs_writeblock("user1", "/dir/file.txt", 10, writedata1);
  status = fs_create("user1", "/dir/dir_dir", 'd');
  status = fs_create("user1", "/dir/dir_dir/ddd", 'd');
  status = fs_create("user1", "/dir/dir_dir/ddd/dd.txt", 'f');
  status = fs_create("user1", "/dir/dd1", 'd');
  status = fs_create("user1", "/dir/dd1/dd2", 'd');
  status = fs_create("user1", "/dir/dd1/dd2/dd3", 'f');
  status = fs_create("user1", "/dir2", 'd');
  status = fs_create("user1", "/dir2/dir3", 'f');
  status = fs_writeblock("user1", "/dir/dir_dir/ddd/dd.txt", 0, writedata1);
  status = fs_writeblock("user1", "/dir2/dir3", 0, writedata1);
  status = fs_readblock("user1", "/dir2/dir3", 0, readdata);
  status = fs_delete("user1", "/dir2/dir3");
  status = fs_delete("user1", "/dir2");
  status = fs_create("user1", "/UM", 'd');
  status = fs_create("user1", "/UM/EECS482", 'd');
  status = fs_create("user1", "/UM/EECS482/project4", 'd');
  status = fs_create("user1", "/UM/EECS482/project4/jtw", 'd');
  status = fs_create("user1", "/UM/EECS482/project4/jtw/disk.cpp", 'f');
  status = fs_writeblock("user1", "/dir/dd1/dd2/dd3", 0, writedata);
  status = fs_writeblock("user1", "/dir/dir_dir/ddd/dd.txt", 1, writedata1);
  status = fs_writeblock("user1", "/dir/dd1/dd2/dd3", 0, writedata1);
  status = fs_readblock("user1", "/dir/dd1/dd2/dd3", 0, readdata);
  status = fs_writeblock("user1", "/dir", 10, writedata1);
  status = fs_writeblock("user1", "/dir/dd1/dd2", 0, writedata1);
  status = fs_delete("user1", "/dir/dir_dir/ddd/");
  status = fs_delete("user1", "/dir/dir_dir/ddd/dd3");
  status = fs_delete("user1", "/dir/dir_dir/ddd/dd.txt");
  status = fs_writeblock("user1", "/dir/dd1/dd2/dd3", 0, writedata);
  status = fs_writeblock("user1", "/UM/EECS482/project4/jtw/disk.cpp", 0, writedata1);
  status = fs_writeblock("user1", "/UM/EECS482/project4/jtw/disk.cpp", 1, writedata1);
  status = fs_writeblock("user1", "/dir/dd1/dd2/dd3", 0, writedata1);
  status = fs_readblock("user1", "/dir/dd1/dd2/dd3", 0, readdata);
  status = fs_writeblock("user1", "/dir", 10, writedata1);

  status = fs_readblock("user1", "/UM/EECS482/project4/jtw/disk.cpp", 0, readdata);
  status = fs_readblock("user1", "/UM/EECS482/project4/jtw/disk.cpp", 1, readdata);

  status = fs_create("user2", "/diir", 'd');
  status = fs_create("user2", "/diir/file.txt", 'f');
  status = fs_writeblock("user2", "/diir/file.txt", 0, writedata);
  status = fs_writeblock("user2", "/diir", 0, writedata);
  status = fs_writeblock("user2", "/diir/file.txt", 10, writedata1);
  status = fs_create("user2", "/diir/diir_diir", 'd');
  status = fs_create("user2", "/diir/diir_diir/ddd", 'd');
  status = fs_create("user2", "/diir/diir_diir/ddd/dd.txt", 'f');
  status = fs_create("user2", "/diir/dd1", 'd');
  status = fs_create("user2", "/diir/dd1/dd2", 'd');
  status = fs_create("user2", "/diir/dd1/dd2/dd3", 'f');
  status = fs_writeblock("user2", "/diir/diir_diir/ddd/dd.txt", 0, writedata1);
  status = fs_writeblock("user2", "/diir/diir_diir/ddd/dd.txt", 1, writedata1);
  status = fs_writeblock("user2", "/diir/dd1/dd2/dd3", 0, writedata1);
  status = fs_readblock("user2", "/diir/dd1/dd2/dd3", 0, readdata);
  status = fs_writeblock("user2", "/diir", 10, writedata1);
  status = fs_writeblock("user2", "/diir/dd1/dd2", 0, writedata1);
  status = fs_delete("user2", "/diir/diir_diir/ddd/");
  status = fs_delete("user2", "/diir/diir_diir/ddd/dd3");
  status = fs_delete("user2", "/diir/diir_diir/ddd/dd.txt");
  status = fs_writeblock("user2", "/diir/diir_diir/ddd/dd.txt", 1, writedata1);
  status = fs_writeblock("user2", "/diir/dd1/dd2/dd3", 0, writedata1);
  status = fs_readblock("user2", "/diir/dd1/dd2/dd3", 0, readdata);
  status = fs_writeblock("user2", "/diir", 10, writedata1);



}
