#include "disk.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <fs_server.h>
#include <iterator>
Lock Disk_Server::lock;
Disk_Server::Disk_Server(int fd):Server(fd){}
/*
 * @brief recursively access the path
 * @requirement size of the vector > 2
 * @return a lock to final entry
 * @return also the block will be inode block number of the last entry on the path
 * @note hand in hand lock might fail due to misunderstanding of compiler
 * */


lock_var Disk_Server::_access(shared_lock curr_lk, int i, int& block){
  //base case
  const std::string curr_dir = request.path[i];//safe
  const std::string next_dir = request.path[i + 1];//safe
  //base case
  fs_inode curr_node;
  //read inode
  disk_readblock(block, &curr_node);
  //traverse direntry array
  for(auto b : curr_node.blocks){
    fs_direntry inv[8];
    disk_readblock(b, &inv);
    for(auto _i : inv){
      if(_i.name == next_dir){
        //access next inode block
        block = _i.inode_block;
      }
    }
  }
  int remnant = 2;
  if(request.rtype == Rtype::DELETE || request.rtype == Rtype::CREATE)
    remnant = 3;
  if(i == request.path.size() - remnant){
    if(request.rtype == Rtype::WRITE)
    {
      return unique_lock(lock.find_lock(next_dir));
    } else {
      return shared_lock(lock.find_lock(next_dir));
    }
  }
  return _access(shared_lock(lock.find_lock(next_dir)), i+1, block);
  // NOTE: will the fs always well formed?
  //assume well formed now
  
  
}
void Disk_Server::_read(){
}
void Disk_Server::_write(){

}
void Disk_Server::_delete(){

}
void Disk_Server::_create(){

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
  try{
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
  }
  //send the response
  /**/
  catch(const NofileErr& e){
    std::cerr << e.msg << '\n';
    _close();
    delete this;//delete the handler when spawning thread
    return;//NOTE: not sure if work
  }
  _send();
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
