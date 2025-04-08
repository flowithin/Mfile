#include "disk.h"
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
 * */

void Server::_recv(){
      // Receive message from client.  Use recv().
      int numbytes = 0;
  int MAX_MESSAGE_SIZE = 256;
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
      std::cerr << "space shouldn't appear!!\n";
    path.push_back(file_str);
    /*std::cout << path[i++] << '\n';*/
  }
  return path;
}
/*
 * @brief fill in the request structure
 * */
void Server::to_req(std::vector<std::string>&& vec){
  if(vec[0] == "FS_CREATE"){
    // TODO: check if the size and format are correct
    Ftype ft;
    if(vec[3] == "d")
      ft = Ftype::DIR;
    else ft = Ftype::FILE;
    request = Request{Rtype::CREATE, ft, vec[1], parse_del(vec[2], '/'), "", 0 };
  } else if(vec[0] == "FS_READ"){
    request.rtype = Rtype::READ;
  } else if(vec[0] == "FS_WRITE"){
    request.rtype = Rtype::WRITE;
  } else if(vec[0] == "FS_DELETE"){
    request.rtype = Rtype::DELETE;
  }
}
void Server::_send(){
  send(fd, request.content.c_str(), request.content.length(), 0);
}
void Server::_close(){
  close(fd);
}
void Server::run_server(){
  init();
  _accept();
}

