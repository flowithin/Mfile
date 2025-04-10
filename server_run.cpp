#include "server.h"
int main(int argc, char** argv){
  char port[10];
  if(argc == 1)
    strcpy(port, "6969");
  else strcpy(port, argv[1]);
  print_port(std::stoi(port));
  /*std::cout << port << '\n';*/
  Server svr(port);
  svr.run_server();
}
