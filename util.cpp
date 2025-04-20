/*#define LOG_FL*/
/*#define LOG*/
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



void print_fl(){
  #ifdef LOG_FL
  std::cout << "---------FREE LIST-------------\n";
  for(int i=0; i < 106;i++){
    std::cout << Disk_Server::free_list[i] << " ";
  }
  std::cout << '\n';
#endif
}
