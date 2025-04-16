#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <string>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    char readdata[FS_BLOCKSIZE];
    strcpy(readdata, "hellow");
    int status;

    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    if (argc != 3) {
        std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    status = fs_create("user1", "/dir", 'd');
    assert(!status);
  /**/
    /*status = fs_create("user1", "/dir/file.txt", 'f');*/
    /*assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2", 'd');*/
  /*  assert(!status);*/
  /*//NOTE:same file name in different dir*/
  /*  status = fs_create("user1", "/dir/dir2/file", 'f');*/
  /*  assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2/dir3", 'd');*/
  /*  assert(!status);*/
  /*  status = fs_create("user1", "/dir/dir2/dir3/file2", 'f');*/
  /*  assert(!status);*/
  /*  status = fs_create("user2", "/dir22", 'd');*/
  /*  assert(!status);*/
  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i;
    std::cout << ss.str() << "\n";
    status = fs_create("user2", ss.str().c_str(), 'd');
    assert(!status);

    ss <<  "/file.txt";
    status = fs_create("user2", ss.str().c_str() , 'f');
    assert(!status);
  }
  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i << "11";
    std::cout << ss.str() << "\n";
    status = fs_create("user2", ss.str().c_str(), 'd');
    assert(!status);

    ss <<  "/file.txt";
    status = fs_create("user2", ss.str().c_str() , 'f');
    assert(!status);

    status = fs_writeblock("user2", ss.str().c_str() , 0, writedata);
    assert(!status);

  }

  /*print_fl();*/
  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i  << "11";
    std::stringstream ss2;
    ss2  << ss.str() <<  "/file.txt";
    std::cout << ss.str() << "\n";
    status = fs_delete("user2", ss2.str().c_str());
    assert(!status);
    status = fs_create("user2", ss2.str().c_str(), 'f');
    assert(!status);
    status = fs_delete("user2", ss2.str().c_str());
    assert(!status);
    status = fs_delete("user2", ss.str().c_str());
    assert(!status);
  }

  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i << "11";
    std::cout << ss.str() << "\n";
    status = fs_create("user2", ss.str().c_str(), 'd');
    assert(!status);

    ss <<  "/file.txt";
    status = fs_create("user2", ss.str().c_str() , 'f');
    assert(!status);
    status = fs_writeblock("user2", ss.str().c_str() , 0, writedata);
    assert(!status);
    status = fs_readblock("user2", ss.str().c_str(), 0, readdata);

    status = fs_writeblock("user2", ss.str().c_str() , 1, writedata);
    assert(!status);

  }

  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i  << "11";
    std::stringstream ss2;
    ss2  << ss.str() <<  "/file.txt";
    std::cout << ss.str() << "\n";
    status = fs_readblock("user2", ss2.str().c_str(), 0, readdata);
    assert(!status);
    status = fs_delete("user2", ss2.str().c_str());
    assert(!status);
    status = fs_readblock("user2", ss2.str().c_str(), 0, readdata);
    status = fs_create("user2", ss2.str().c_str(), 'f');
    assert(!status);
    status = fs_readblock("user2", ss2.str().c_str(), 0, readdata);
    status = fs_delete("user2", ss2.str().c_str());
    assert(!status);
    status = fs_delete("user2", ss.str().c_str());
    assert(!status);
  }

  for(int i = 0; i < 24; i++){
    std::stringstream ss;
    ss << "/dir" << i;
    std::cout << ss.str() << "\n";

    std::stringstream ss2;
    ss2 <<ss.str()<<  "/file.txt";
    status = fs_delete("user2", ss2.str().c_str());
    assert(!status);

    status = fs_delete("user2", ss.str().c_str());
    assert(!status);
  }

    status = fs_delete("user2", "/");
    status = fs_delete("user2", "/dir");
    status = fs_delete("user1", "/dir");
}
