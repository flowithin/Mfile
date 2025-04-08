#include "disk.h"
Disk_Server::Disk_Server(int fd):Server(fd){}
void Disk::_read(){

}
void Disk::_write(){

}
void Disk::_delete(){

}
void Disk::_create(){

}



/*
 * @brief debug function
 * */
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
  switch (request.rtype) {
    case Rtype::READ:{
      _read();
      break;
    }
    case Rtype::WRITE:{
      _write();
      break;
    }
    case Rtype::CREATE:{
      _create();
      break;
    }
    case Rtype::DELETE:{
      _delete();
      break;
    }
  }
  /**/
  _close();
  //NOTE: This may lead to Segmentation fualt
  delete this;//delete the handler when spawning thread
}

boost::shared_mutex& Lock::find_lock(std::string str){
    //first aquiring memory lock
    boost::lock_guard<boost::mutex> __lock(mem_mt);
    //aquire shared mutex for read
    /*boost::shared_lock<boost::shared_mutex> _lock(lock.sd_mt);*/
    //check if the file exists
    auto it = file_locks.find(str);
    if(it != file_locks.end()){
      return it->second;
    }
    throw NofileErr("cannot find file");
  }
