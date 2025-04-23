#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"


int main(int argc, char* argv[]) {
    char* server;
    int server_port;

    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    const char* writedata2 = "We xxhold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
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

  fs_create("user1", "/dir", 'd');
  fs_create("user1", "/file", 'f');
  fs_create("user1", "/dir/file.txt", 'f');
  for(int i=0; i < 125; i++){
    fs_writeblock("user1", "/dir/file.txt", i, writedata);
  }
  //should be allowed to do so up to i = 123
  for(int i=0; i < 125; i++){
    fs_writeblock("user1", "/file", i, writedata);
  }
  status = fs_writeblock("user1", "/dir/file.txt", 0, writedata);
  for(int i=0; i < 150; i+=10){
    status = fs_readblock("user1", "/file", i, readdata);
  }
  for(int i=0; i < 125; i+=10){
    fs_writeblock("user1", "/file", i, writedata2);
  }
  for(int i=0; i < 150; i+=10){
    status = fs_readblock("user1", "/file", i, readdata);
  }
  fs_delete("user1", "/dir/file.txt");
  fs_delete("user1", "/file");
  fs_delete("user1", "/dir");
 

}
