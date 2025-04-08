#include "disk.h"
Disk_Server::Disk_Server(int fd):Server(fd){}
void Disk_Server::print_req(){
  switch (request.rtype) {
    case Rtype::DELETE:std::cout << "DELETE\n";
    case Rtype::READ:std::cout << "READ\n";
    case Rtype::WRITE:std::cout << "WRITE\n";
    case Rtype::CREATE:std::cout << "CREATE\n";
  }
  switch (request.ftype) {
    case Ftype::FILE:std::cout << "file\n";
    case Ftype::DIR:std::cout << "dir\n";
  }
  std::cout <<"usr: "<< request.usr << '\n';
  std::cout << "path: ";
  for(auto v : request.path)
    std::cout << v << " ";
}
/*
 * @brief This function handles the requests
 * */
void Disk_Server::handle(){
  _recv();
  to_req(parse_del(str_in, ' '));
  print_req();

  //handle the request
  /*switch (request.rtype) {*/
  /*  case Rtype::READ:*/
  /*  case Rtype::READ:*/
  /*}*/
  /**/
  _close();
  //NOTE: This may lead to Segmentation fualt
  delete this;//delete the handler when spawning thread
}
