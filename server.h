#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include "fs_param.h"
#include "fs_server.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
/*#pragma */

enum class Rtype {READ, WRITE, CREATE, DELETE};
enum class Ftype {FILE, DIR};
struct Request{
  Rtype rtype;
  Ftype ftype;
  std::string usr;
  std::vector<std::string> path;
  std::string path_str;
  /*std::string content;*/
  char content[FS_BLOCKSIZE];
  uint32_t tar_block;
};
class Server{
  struct addrinfo* res;
  struct sockaddr_storage their_addr;
  char* port;
  int fd;
  public:
  Server(char* port);
  Server(int newfd);
  void run_server();
  protected:
  void init();
  void _accept();
  void _close();
  void _recv();
  void _send();
  std::string str_in;
  std::vector<std::string> parse_del(std::string& str, char del);
  void to_req(std::vector<std::string>&& vec);
  Request request;
};
