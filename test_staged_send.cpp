#include <iostream>
#include <string>
#include<sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>

static const size_t MAX_MESSAGE_SIZE = 256;

int main(int argc, const char **argv) {
    // Parse command line arguments
    const char* hostname = argv[1];
    const char* port = argv[2];
    std::string message("FS_CREATEx ");
  std::stringstream large_msg;
  for(int i=0; i < 1001;i++){
    large_msg << i;
  }
    /*std::string message1("user1 ");*/
    /*std::string message2("/dir d");*/

    std::cout << "Sending " << message << " to " << hostname << ":"
              << port << std::endl;

    // Create a socket.  Use socket().


  struct addrinfo* res;
  struct addrinfo hints;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  // Create a sockaddr to specify remote host and port.  Use getaddrinfo().
  getaddrinfo(NULL, port, &hints, &res);//DNS look up

  // a socket can serve multiple purpose
  int fd = socket(res->ai_family, res->ai_flags, res->ai_protocol);
    // Connect to remote server.  Use connect().
  connect(fd, res->ai_addr, res->ai_addrlen);

    // Send message to remote server.  Use send().
  send(fd, large_msg.str().c_str(), large_msg.str().length(), 0);
  /*send(fd, message1.c_str(), message1.length(), 0);*/
  /*send(fd, message2.c_str(), message2.length(), 0);*/
    // Close connection.  Use close().
  close(fd);
    return(0);
}
