#include "disk.h"
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
  // Set the "reuse port" socket option.  Use setsockopt().
  int yes = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt\n");
    exit(1);
  }
  // Configure a sockaddr_in for the accepting socket.
  bind(fd, res->ai_addr, res->ai_addrlen);
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) == -1) {
    perror("getsockname");
    exit(1);
  }
  //print port to let client know
  print_port(ntohs(addr.sin_port));
  // Begin listening for incoming connections.  Use listen().
  listen(fd, 30);
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
  int numbytes = 0;
  const int MAX_MESSAGE_SIZE = 20 + 4 + FS_MAXUSERNAME + FS_MAXPATHNAME + 100;
  int idx = 0;
  char buf[MAX_MESSAGE_SIZE + FS_BLOCKSIZE];
  char ch;
  do {
    numbytes = recv(fd, &ch, 1, MSG_WAITALL);
    if(numbytes <= 0 || idx >= MAX_MESSAGE_SIZE)
      throw NofileErr("invalid request");
    buf[idx++] = ch;
  } while (ch!='\0');
  // automatically detect the portion up to <NULL>
  str_in = buf;
  std::string req = str_in.substr(0, str_in.find(' '));
  if(req == "FS_WRITEBLOCK"){
    if((numbytes = recv(fd, buf + idx, FS_BLOCKSIZE, MSG_WAITALL)) <= 0)
      throw NofileErr("connection break");
    //FS_WRITEBLOCK
    memcpy(request.content, buf+idx, FS_BLOCKSIZE);
  }
}

/*
 * @brief parse a string with deliminator del and check white space
 * */
std::vector<std::string> Server::parse_del(std::string& str, char del){
  std::stringstream ss(str);
  std::vector<std::string> path = {};
  int i=0;
  const int MAX_MESSAGE_SIZE = 20 + 4 + FS_MAXUSERNAME + FS_MAXPATHNAME + 100;
  char file[MAX_MESSAGE_SIZE];
  bool flag=false;
  if(*str.rbegin() == '/')
    throw NofileErr("cannot end with /");
  while(ss.getline(file, MAX_MESSAGE_SIZE, del)){
    std::string file_str(file);
    if(file_str == "" && del == '/')//only if parsing pathname TODO: test the functionality
    {
      //only one root allowed
      if(flag)
        throw NofileErr("mal formed pathname, you have ..//..");
      file_str = "@ROOT"; 
      flag = true;
    }
    path.push_back(file_str);
  }
  return path;
}
/*
 * @brief fill in the request structure
 * */
void Server::to_req(std::vector<std::string>&& vec){
  if(vec.size() < 3)
    throw NofileErr("size should be at least 3");
  if(vec[1].size() > FS_MAXUSERNAME)
    throw NofileErr("user name too long");
  if(vec[1] == "")
    throw NofileErr("user name can not be empty");
  if(vec[2].size() > FS_MAXPATHNAME)
    throw NofileErr("path name too long");
  std::vector<std::string> p = parse_del(vec[2], '/');
  //path name too short
  if(p.size() < 2)
    throw NofileErr("must have more than two pathname");
  for(auto e : p){
    if(e.size() > FS_MAXFILENAME)
      throw NofileErr("file name too long");
  }
  if(p[0] != "@ROOT")
    throw NofileErr("mal formed path");
  if(vec[0] == "FS_CREATE"){
    uint32_t _f_;
    if(vec.size() != 4)
      throw NofileErr("should be size of 4");
    Ftype ft;
    if(vec[3] == "d")
      ft = Ftype::DIR;
    else if(vec[3] == "f") ft = Ftype::FILE;
    else throw NofileErr("invalid type");
    request = Request{.rtype = Rtype::CREATE, .ftype = ft, .usr = vec[1], .path = p, .content = "", .tar_block = 0 };

  } else if(vec[0] == "FS_READBLOCK"){
    if(vec.size() != 4)
      throw NofileErr("should be size of 4");
    boost::regex num("0|[1-9][0-9]*");
    if(!boost::regex_match(vec[3], num))
      throw NofileErr("invalid block number");
    uint32_t block=0;
    block = std::stoul(vec[3]);
    request = Request{.rtype = Rtype::READ, .ftype = Ftype::FILE, .usr = vec[1], .path = p, .content = "", .tar_block =  block};

  } else if(vec[0] == "FS_WRITEBLOCK"){
    if(vec.size() != 4)
      throw NofileErr("should be size of 4");
    if(vec[1] == "")
      throw NofileErr("user name can't be empty");
    request.rtype = Rtype::WRITE;
    request.ftype = Ftype::FILE;
    request.usr = vec[1];
    request.path = p;
    boost::regex num("0|[1-9][0-9]*");
    if(!boost::regex_match(vec[3], num))
      throw NofileErr("invalid block number");
    uint32_t block=0;
    block = std::stoul(vec[3]);
    request.tar_block = block;
  } else if(vec[0] == "FS_DELETE"){
    if(vec.size() != 3)
      throw NofileErr("should be size of 3");
    request = Request{.rtype = Rtype::DELETE, .ftype = Ftype::FILE, .usr = vec[1], .path = p, .content = "", .tar_block =  0};
  } else 
    throw NofileErr("not correct request");
  request.path_str = "@ROOT" + vec[2] + '/';
  if(request.tar_block >= FS_MAXFILEBLOCKS)
    throw NofileErr("exceed file size");
}

/*
 * @brief This function send the response (if successful) to the client
 * */
void Server::_send(){
  uint32_t size = str_in.length() + 1;
  if(request.rtype == Rtype::READ)
  {
    size = FS_BLOCKSIZE + str_in.length() + 1; 
    char out[size];
    strcpy(out, str_in.c_str()); 
    memcpy(out + str_in.length()+1, request.content, FS_BLOCKSIZE);
    send(fd, out, size, MSG_NOSIGNAL);
  } else{
    char out[size];
    strcpy(out, str_in.c_str()); 
    send(fd, out, size, MSG_NOSIGNAL);
  }
}
void Server::_close(){
  close(fd);
}
void Server::run_server(){

  init();
  _accept();
}

