#include "server.h"
int main(int argc, char** argv){
  char* port;
  if(argc == 1)
    port = "0";
  else port = argv[1];
  Server svr(port);
  svr.run_server();
}
