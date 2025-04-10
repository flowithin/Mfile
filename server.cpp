#include "disk.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fs_param.h>
#include <string>
/*Server::Server():port{}*/
Server::Server(char* port):port{port}{}
Server::Server(int newfd):fd{newfd}{}
/*
 * @brief initialize socket structures and start listening
 * */
void Server::init(){
  struct addrinfo* res;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  std::cout << "port: " << port <<'\n';
  if(getaddrinfo(NULL, port, &hints, &res) == -1)
  {
    perror("getaddrinfo");//DNS look up
    exit(1);
  }
  // Create socket for accepting connections.  Use socket().
  fd = socket(res->ai_family, res->ai_flags, res->ai_protocol);
  if(fd == -1){
    perror("socket");
    exit(1);
  }
  std::cout << "fd: " << fd << '\n';
  // Set the "reuse port" socket option.  Use setsockopt().
  int yes = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt\n");
    exit(1);
  }
  // Configure a sockaddr_in for the accepting socket.
  // Bind to the port.  Use bind().
  bind(fd, res->ai_addr, res->ai_addrlen);
  // Begin listening for incoming connections.  Use listen().
  listen(fd, 30);
  // Serve incoming connections one by one forever.
}

/*
 * @brief looping to accept and spawning thread when succeed
 * */
void Server::_accept(){
while (1) {
    // Accept connection from client.  Use accept().
    int newfd;
    socklen_t peer_size = sizeof(their_addr);
    if((newfd = accept(fd, (struct sockaddr *)&their_addr, &peer_size)) == -1){
      perror("accept");
      continue;
    }
    Disk_Server* ds = new Disk_Server(newfd);
    //if accepted create a new thread
    boost::thread t{&Disk_Server::handle, ds};
    t.detach();
  }
}
/*
 * @brief recv() in socket.h wrapped up
 * @note it modifies request here before to_req is called
 * */

void Server::_recv(){
      // Receive message from client.  Use recv().
  int numbytes = 0;
  int MAX_MESSAGE_SIZE = 1000;
  char buf[MAX_MESSAGE_SIZE];
  if((numbytes = recv(fd, buf, MAX_MESSAGE_SIZE-1, 0)) == -1){
        perror("recv");
        exit(1);
      } else if (numbytes == 0)
        {
          //closed from client side
          close(fd);
          return;
        }
      buf[numbytes] = '\0';
      // Print message from client.
      str_in = buf;
      /*std::cout << "str_in.len = " << str_in.length() << '\n';*/
      if(numbytes > str_in.length()){
    //write
      memcpy(request.content, buf+str_in.length()+1, FS_BLOCKSIZE);
  }
  buf[str_in.length()] = '#'; 
      printf("server received %s\n size: %d\n", buf, numbytes);
      // Close connection.  Use close().
}

/*
 * @brief parse a string with deliminator del and check white space
 * */
std::vector<std::string> Server::parse_del(std::string& str, char del){
  std::stringstream ss(str);
  std::vector<std::string> path;
  int i=0;
  char file[FS_MAXFILENAME];
  while(ss.getline(file, FS_MAXFILENAME, del)){
    std::string file_str(file);
    file_str += file_str == "" ? "@ROOT" : "";
    int n = file_str.find(' ');
    /*std::cout << "n= " << n << '\n';*/
    if (n != -1)
      throw NofileErr("space shouldn't appear!!\n");
    path.push_back(file_str);
    /*std::cout << path[i++] << '\n';*/
  }
  return path;
}
/*
 * @brief fill in the request structure
 * */
void Server::to_req(std::vector<std::string>&& vec){
  if(vec[1].size() > FS_MAXUSERNAME)
    throw NofileErr("user name too long");
  if(vec[2].size() > FS_MAXPATHNAME)
    throw NofileErr("path name too long");
  std::vector<std::string> p = parse_del(vec[2], '/');
  for(auto e : p){
    if(e.size() > FS_MAXFILENAME)
      throw NofileErr("file name too long");
  }
  if(p[0] != "@ROOT")
    throw NofileErr("mal formed path");
  if(vec[0] == "FS_CREATE"){
    // TODO: check if the size and format are correct
    Ftype ft;
    if(vec[3] == "d")
      ft = Ftype::DIR;
    else ft = Ftype::FILE;
    request = Request{Rtype::CREATE, ft, vec[1], p, "", 0 };
  } else if(vec[0] == "FS_READBLOCK"){
    request = Request{Rtype::READ, Ftype::FILE, vec[1], p, "", stoi(vec[3])};
  } else if(vec[0] == "FS_WRITEBLOCK"){
    if(vec[1] == "")
      throw NofileErr("user name can't be empty");
    request.rtype = Rtype::WRITE;
    request.ftype = Ftype::FILE;
    request.usr = vec[1];
    request.path = p;
    request.tar_block = stoi(vec[3]);
  } else if(vec[0] == "FS_DELETE"){
    request = Request{Rtype::DELETE, Ftype::FILE, vec[1], p, "", 0};
  }
}
void Server::_send(){
  /*std::string out = str_in + std::string(request.content);*/
  uint32_t size = str_in.length() + 1;
  if(request.rtype == Rtype::READ)
  {
    size = FS_BLOCKSIZE + str_in.length() + 1; 
    char out[size]={};
    strcpy(out, str_in.c_str()); 
    memcpy(out + str_in.length()+1, request.content, FS_BLOCKSIZE);
    send(fd, out, size, 0);
  } else{
    char out[size]={};
    strcpy(out, str_in.c_str()); 
    send(fd, out, size, 0);
  }
}
void Server::_close(){
  close(fd);
}
void Server::run_server(){

  init();
  _accept();
}

