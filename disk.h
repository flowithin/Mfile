#include <asm-generic/socket.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/mutex.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include "server.h"
#include "fs_server.h"
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>

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


struct Lock{
  Lock();
  boost::mutex mem_mt;
  std::unordered_map<std::string, boost::shared_mutex> file_locks;
  boost::shared_mutex& find_lock(std::string str);
};

class Disk_Server : public Server{
  static Lock lock;
  static std::vector<int>* free_list;
  lock_var _access(shared_lock curr_lk, int i, int& block);
  void _write();
  void _read();
  void _create();
  void _delete();
  public:
  void print_req();
  Disk_Server(int newfd);
  void handle();
};
