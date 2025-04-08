#include "server.h"
#include <cstring>
int main(){
  char port[4];
  strcpy(port, "6969");
  std::cout << port << '\n';
  Server svr(port);
  svr.run_server();
}
