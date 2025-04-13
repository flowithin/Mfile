#include <asm-generic/socket.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/mutex.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include "server.h"
#include "fs_server.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>
#define OCCUPIED true
#define FREE false
using shared_lock = boost::shared_lock<boost::shared_mutex>;
using unique_lock = boost::unique_lock<boost::shared_mutex>;
using lock_var = std::variant<shared_lock, unique_lock>;
/*struct Read_lock(){*/
/*  ReadLock(boost::shared_mutex& sd_mt)*/
/*        : mem_lock(mem_mt), shared_lock(sd_mt) {}*/
/*}*/
struct NofileErr{
  std::string msg;
  NofileErr(std::string str):msg{str}{}
};

struct Acc{
  lock_var lv;
  uint32_t entry;
  std::unique_ptr<fs_direntry[]> inv;
};

struct Lock{
  Lock();
  boost::mutex mem_mt;
  std::unordered_map<std::string, boost::shared_mutex> file_locks;
  boost::shared_mutex& find_lock(std::string str);
  friend void probe(int block);
};

class Disk_Server : public Server{
  bool status = false;
  void access_inode(int block, fs_inode& in, char type);
  Acc _access(lock_var curr_lk, int i, uint32_t& block, fs_inode& curr_node);
  void _read();
  void _write();
  void _create();
  void _delete();
  public:
  static Lock lock;
  static std::vector<bool> free_list;
  void print_req();
  Disk_Server(int newfd);
  void handle();
};
