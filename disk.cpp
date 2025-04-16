#include "disk.h"
#include <boost/function/function_base.hpp>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unistd.h>
/*#define LOG_FL*/
#define LOG
/*#define LOCK_DETECT*/
template <typename T, typename...>
void myPrint(std::string words, const T& t){
#ifdef LOG
  std::cout << words;
  std::cout << t << std::endl;
#endif
}
template <typename k, typename t>
void print_lock_map(const std::unordered_map<k, t>& map){
  #ifdef LOG
  std::cout << "********LOCK************: \n";
  auto it = map.begin();
  while(it != map.end()){
    std::cout << it->first <<"\n";
    it++;
  }
  std::cout << "********LOCK************: \n";
#endif
}
Lock::Lock(){}
Lock Disk_Server::lock;
/*
 * @brief fill in the non free blocks
 * */
void free_list_init(int block, std::vector<bool>& v){
  fs_direntry inv[8];
  fs_inode in;
  v[block] = 1;
  disk_readblock(block, &in);
  for(size_t i = 0; i < in.size; i++){
    v[in.blocks[i]] = 1;
    if(in.type == 'f') continue;
    disk_readblock(in.blocks[i], inv);
    for(int j = 0; j < 8; j++){
      if(inv[j].inode_block != 0)
        free_list_init(inv[j].inode_block, v);
    }
  }
}
std::vector<bool> Disk_Server::free_list = [](){
  std::vector<bool> v(4096);
  //free list traverse
  free_list_init(0, v);
  /*for(auto v_ : v)*/
  /*std::cout << v_ << ' ';*/
  return v;
}();
Disk_Server::Disk_Server(int fd):Server(fd){}

/*
 * @brief access the free_list with lock
 * @param idx: the index you watn to change
 * @param state: the state you watn it to be
 * @return true if the state toggles
 * */
bool free_list_access(uint32_t idx, bool state){
  boost::lock_guard<boost::mutex> _lock(Disk_Server::lock.mem_mt);
  bool is_free = Disk_Server::free_list[idx] != state;
  Disk_Server::free_list[idx] = state;
  return is_free;
}

/*
 * @brief get a free block from the free list and make it occupied
 * @param free: free block
 * */
bool get_free_block(uint32_t& free, bool toggle = true){
  boost::lock_guard<boost::mutex> _lock(Disk_Server::lock.mem_mt);
  for(size_t i = 0; i < Disk_Server::free_list.size(); i++){
    if(Disk_Server::free_list[i] == FREE){
      //free block
      free = i;
      if(toggle)
        Disk_Server::free_list[i] = OCCUPIED;
      return true;
    }
  }
  return false;
}
/*
 * @brief recursively access the path
 * @requirement size of the vector > 2
 * @return a lock to final entry
 * @return also the block will be inode block number of the last entry on the path
 * @note hand in hand lock might fail due to misunderstanding of compiler
 * */

/*
 *
 * @brief access inode with checks, in will contain the read info
 * */
void Disk_Server::access_inode(int block, fs_inode& in, char type){
  disk_readblock(block, &in);
  std::stringstream ss;
  ss << "expect " << type << " found " << in.type << '\n';
  if(type != 'a' && in.type != type)
    throw NofileErr(ss.str());
  if(strcmp(in.owner, "") != 0 && in.owner != request.usr)
    throw NofileErr("access denied");
}



/*
 * @brief find a file in directory 
 * @param in: inode of the directory
 * @param entry: entry of the found entry (if not found, this is the first not occupied entry in current dir block)
 * @param inv: fs_direntry inv used to type cast read value
 * @return true if found false otw
 * */
bool dir_find(const fs_inode in, uint32_t& entry, std::string target, fs_direntry* inv){
  for(uint32_t i = 0; i < in.size; i++){
    disk_readblock(in.blocks[i], inv + i * 8);
    uint32_t j = 0;
    for(; j < 8; j++){
      if(inv[i * 8 + j].inode_block == 0)
      {
        //update free_block
        entry = std::min(i * 8 + j, entry);
      } else if(inv[i * 8 + j].name == target){
        entry = i * 8 + j;
        return true;
      }
    }
  }
  return false;
}




/*
 * @brief this function access to the point second last element along the path
 * @param block is the block of the next inode
 *
 * */
Acc Disk_Server::_access(lock_var curr_lk, int i, uint32_t& block, fs_inode& curr_node){
  //base case

myPrint("access begin\n","");
  
  const std::string next_dir = request.path[i + 1];//safe
  std::string next_path = "";
  for(int j = 0; j <= i + 1; j++){
    next_path += request.path[j] + '/';
  }
  myPrint("next_dir: ", next_dir );
  myPrint("next_path: ", next_path );
  access_inode(block, curr_node, 'd');
  fs_direntry* inv_r = new fs_direntry[curr_node.size * 8]; 
  uint32_t entry=curr_node.size * 8;
  //traverse direntry array
  bool found = dir_find(curr_node, entry, next_dir, inv_r);
  if(i == request.path.size() - 2) {
    /*std::cout << "sleep on " << next_path << '\n';*/
    /*sleep(1000);*/
    if(request.rtype == Rtype::CREATE && found)
      throw NofileErr(next_dir + " already exist");
    else if(request.rtype != Rtype::CREATE && !found)
      throw NofileErr(next_dir + " not found!");
    return Acc{boost::move(curr_lk), entry, std::unique_ptr<fs_direntry[]>(inv_r)};
  }
  if(!found)
    throw NofileErr(next_dir + " not found!");
  {
    lock_var nl;
    if(i == request.path.size() - 3 && (request.rtype == Rtype::DELETE || request.rtype == Rtype::CREATE))
      nl = unique_lock(lock.find_lock(next_path));
    else 
      nl = shared_lock(lock.find_lock(next_path));
    curr_lk.swap(nl);
  }
  std::string curr_path;
  for(int j = 0; j <= i; j++){
    curr_path += request.path[j] + '/';
  }
  /*std::cout << "_detect_ curr_path lock\n";*/
  /*unique_lock _detect_ = unique_lock(lock.find_lock(curr_path));*/
  /*std::cout << "_detect_ next_path lock\n";*/
  /*_detect_ = unique_lock(lock.find_lock(next_path));*/
 /*std::cout << "next_dir: " << next_dir << '\n';*/
  return _access(boost::move(curr_lk), i+1, block = inv_r[entry].inode_block, curr_node);
  // NOTE: will the fs always well formed?
  //assume well formed now
}




//inode insert block
uint32_t iiblock(fs_inode& inode){
  uint32_t free_block;
  if(inode.size == FS_MAXFILEBLOCKS)
    throw NofileErr("maximum file size");
  if(!get_free_block(free_block))
    throw NofileErr("no free block");
  inode.blocks[inode.size++] = free_block;
  return free_block;
}



void Disk_Server::_read(){
myPrint("_read\n","");
  uint32_t block = 0, file_in;
  fs_inode din, fin;
  lock_var lv;
  {
    Acc acc = _access(boost::move(shared_lock(lock.find_lock("@ROOT/"))), 0, block, din);
    // ^---this WILL unlock going out of scope
    lv = shared_lock(lock.find_lock(request.path_str));
    file_in = acc.inv.get()[acc.entry].inode_block;
  }
  //debug code
#ifdef LOCK_DETECT
  std::stringstream ss;
  for(int i=0; i < request.path.size() -1; i++)
    ss << request.path[i] << "/";
  myPrint("ss.str(): ","ss.str()");
  unique_lock _detect_ = unique_lock(lock.find_lock(ss.str()));
#endif
  //debug code
  access_inode(file_in, fin, 'f');
  if(request.tar_block >= fin.size)
    throw NofileErr("block exceed boundary");
  disk_readblock(fin.blocks[request.tar_block], &request.content);
}




void Disk_Server::_write(){
myPrint("_write\n","");
  uint32_t block = 0;
  fs_inode din, fin;
  lock_var lv;
  {
    lock_var lk = request.path.size() <= 2 ? unique_lock(lock.find_lock("@ROOT/")) : 
                                             shared_lock(lock.find_lock("@ROOT/"));
    /*lock_var lk = shared_lock(lock.find_lock("@ROOT/")); */
    Acc acc = _access(boost::move(lk), 0, block, din);
    block = acc.inv.get()[acc.entry].inode_block;
    lv = unique_lock(lock.find_lock(request.path_str));
  }
  //debug code

  #ifdef LOCK_DETECT
  std::stringstream ss;
  for(int i=0; i < request.path.size() -1; i++)
    ss << request.path[i] << "/";
  myPrint("ss.str(): ", ss.str() );
  shared_lock _detect_ = shared_lock(lock.find_lock(ss.str()));
#endif
  //debug code

  access_inode(block, fin, 'f');
  if(request.tar_block > fin.size)
    throw NofileErr("exceed file size");
  else if(request.tar_block < fin.size){//<------scheme tested (test_write_excess.cpp)
    //write the data
    disk_writeblock(fin.blocks[request.tar_block], &request.content);
  }else{
    iiblock(fin);
    disk_writeblock(fin.blocks[request.tar_block], &request.content);
    //then write inode 
    disk_writeblock(block, &fin);
  }
}



void Disk_Server::_delete(){
myPrint("_delete\n","");
  print_lock_map(lock.file_locks);
  uint32_t din_block = 0;
  lock_var ful;
  lock_var dlv;  
  if(request.path.size() <= 2)
    dlv = unique_lock(lock.find_lock("@ROOT/"));
  else
    dlv = shared_lock(lock.find_lock("@ROOT/"));
  fs_inode din, fin;
  Acc acc = _access(boost::move(dlv), 0, din_block, din);
  myPrint("_access returned\n","");
  myPrint("size: ",request.path.size() ); 

  //debug code
  #ifdef LOCK_DETECT
  std::stringstream ss;
  for(int i=0; i < request.path.size() -1; i++)
    ss << request.path[i] << "/";
  myPrint("ss.str(): ",ss.str());
  shared_lock _detect_ = shared_lock(lock.find_lock(ss.str()));
#endif
  //debug code

  //should have unique lock
  myPrint("delete sleep\n","");
  /*sleep(1000);*/
  uint32_t file_entry = acc.entry;//the entry #
  uint32_t file_block = acc.inv.get()[file_entry].inode_block;//the inode block of file
  //acquire the UNIQUE lock on the file(or dir)
  //this is to ensure no read/write is happening on the file
  ful = unique_lock(lock.find_lock(request.path_str));
  /*std::cout << "delete ful before\n";*/
  /*shared_lock _detect_ = shared_lock(lock.find_lock(request.path_str));*/
  /*std::cout << "delete ful after\n";*/
  access_inode(file_block, fin, 'a');
  //check if directory(if it is) is empty
  if(fin.type == 'd' && fin.size != 0)
    throw NofileErr("deleting non-empty dir");
  //freeing block of the deleted
  for(size_t j = 0; j < fin.size; j++){
    //TODO: free_list_access to multiple function
    assert(free_list_access(fin.blocks[j], FREE));
  }
  int size = 0;
  for(int i = file_entry/8 * 8; i < file_entry/8 * 8 + 8; i++){
    if(acc.inv.get()[i].inode_block != 0)
      size++;
  }
  //freeing inode of the file(dir)
  assert(free_list_access(acc.inv.get()[file_entry].inode_block, FREE));
  if(size == 1)
  {
    //shrink the dir
    //free the block of the inventory
    assert(free_list_access(din.blocks[file_entry/8], FREE));
    for(int j = file_entry/8; j < din.size-1; j++){
      din.blocks[j] = din.blocks[j+1];
    }
    din.size--;
    //write the inode block for the dir
    //we don't need to clear the content
    disk_writeblock(din_block, &din);
  } else {
    acc.inv.get()[file_entry].inode_block = 0;
    disk_writeblock(din.blocks[file_entry/8], acc.inv.get() + file_entry/8*8);//delete the entry in the inv
  }
  //clear its space in lock
  myPrint("path_str = ", request.path_str );
  
}



void Disk_Server::_create(){
  myPrint("_create\n","");
  print_lock_map(lock.file_locks);
  std::string name = *(request.path.end()-1);
  uint32_t file_inode_block;
  if(request.usr == "")
    throw NofileErr("invalid user name");
  if(!get_free_block(file_inode_block))
    throw NofileErr("no free space on disk!");
  assert(file_inode_block!=0);
  fs_inode din;
  uint32_t dir_block = 0; 
  lock_var dlv; 
  if(request.path.size() <= 2) 
    dlv = unique_lock(lock.find_lock("@ROOT/"));
  else 
    dlv = shared_lock(lock.find_lock("@ROOT/"));
  Acc acc = _access(boost::move(dlv), 0, dir_block, din);

  uint32_t free_block, dir_b_w;//free block for new dir entry
  bool need_expand = false;
  fs_direntry _inv[8]={{"",0}};//the directory block of change, intialized to all "",0
  if(acc.entry == din.size * 8){
    need_expand = true;
    dir_b_w = iiblock(din);
    //prepare _inv
   _inv[0] = {"", file_inode_block};
    strcpy(_inv[0].name, name.c_str());
  } else {
    for(int i = 0; i < 8; i++){
      _inv[i] = acc.inv.get()[acc.entry/8 * 8 + i];
      /*std::cout << acc.inv.get()[acc.entry/8 + i].name << ' ' << acc.inv.get()[acc.entry/8 + i].inode_block << '\n';*/
    }
    //_inv block modified by adding to it the new entry
    strcpy(_inv[acc.entry%8].name, name.c_str());
    _inv[acc.entry%8].inode_block = file_inode_block;
    dir_b_w = din.blocks[acc.entry/8];
  }
  fs_inode fin = fs_inode{request.ftype == Ftype::FILE ? 'f' : 'd', "", 0};
  strcpy(fin.owner, request.usr.c_str());
  disk_writeblock(file_inode_block, &fin);//inode of the new file(or dir)
  disk_writeblock(dir_b_w, _inv);//block of inv where new entry is mapped
  if(need_expand)
    disk_writeblock(dir_block, &din);//dir inode, only if new block added
}



void print_fl(){
  #ifdef LOG_FL
  std::cout << "---------FREE LIST-----------------\n";
  for(int i=0; i < 106;i++){
    std::cout << Disk_Server::free_list[i] << " ";
  }
  std::cout << '\n';
#endif
}
/*
 * @brief debug function
 * */
void Disk_Server::print_req(){
  #ifdef LOG
  switch (request.rtype) {
    case Rtype::DELETE:{
      std::cout << "DELETE\n";
      break;
    }
    case Rtype::READ:{std::cout << "READ\n";break;}
    case Rtype::WRITE:{std::cout << "WRITE\n";break;}
    case Rtype::CREATE:{std::cout << "CREATE\n";break;}
  }
  switch (request.ftype) {
    case Ftype::FILE:{std::cout << "file\n";break;}
    case Ftype::DIR:{std::cout << "dir\n";break;}
  }
  std::cout <<"usr: "<< request.usr << '\n';
  std::cout << "path: ";
  for(auto v : request.path)
    std::cout << v << " ";
  std::cout << "\n";
  std::cout << "block: " << request.tar_block << '\n';
  std::cout << "content: " << request.content << '\n';
#endif
}

/*
 * @brief This function handles the requests
 * */
void Disk_Server::handle(){
  try{
    _recv();
    to_req(parse_del(str_in, ' '));
    print_req();
    print_fl();
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
        lock.remove_lock(request.path_str);
        break;
      }
    }
    print_fl();
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
  delete this;//delete the handler when spawning thread
}

/*
 * @brief return mutex of the lock of the file
 * @note that passing in an invalid file screw things up
 *
 * */
boost::shared_mutex& Lock::find_lock(std::string str){
  //first aquiring memory lock
  boost::lock_guard<boost::mutex> __lock(mem_mt);
  return file_locks[str];
}
void Lock::remove_lock(std::string str){
  //first aquiring memory lock
  myPrint("remove_lock, str = ", str );
  boost::lock_guard<boost::mutex> __lock(mem_mt);
  if(file_locks.find(str) != file_locks.end())
    file_locks.erase(str);
}

