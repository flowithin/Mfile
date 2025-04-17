#include <fs_param.h>
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
    std::string message("FS_WRITEBLOCK ");

    const char* writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
  std::stringstream large_msg;
  for(int i=0; i < 1001;i++){
    large_msg << i;
  }
    std::string message1("user1 ");
    std::string message2("/dir d");
  std::string message_create("FS_CREATE user1 /dir x"); 
  std::string message_write("FS_WRITEBLOCK user1 /dir -1A"); 
    /*std::cout << "Sending " << large_msg.str() << " to " << hostname << ":"*/
              /*<< port << std::endl;*/

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
  /*send(fd, large_msg.str().c_str(), large_msg.str().length(), 0);*/
  /*send(fd, message.c_str(), message.length(), 0);*/
  /*send(fd, message1.c_str(), message1.length(), 0);*/
  /*send(fd, message2.c_str(), message2.length() + 1, 0);*/
  send(fd, message_write.c_str(), message_write.length() + 1, 0);
  send(fd, writedata, FS_BLOCKSIZE , 0);
    // Close connection.  Use close().
  close(fd);
    return(0);
}
