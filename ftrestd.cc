/**********************************/
/***** Martin Vaško xvasko12  *****/
/****  FIT VUTBR 2.BIT->2.BIB  ****/
/*** IPK -> 1.project, RESTapi  ***/
/** Date of creation = 24.2.2016 **/
/* Makefile usage = make,make all */
/**********           *************/
/* project- RESTapi client which  */
/* sends http requests to server  */
/**********           *************/
/*                                */
/**   Using BSD sockets, regex,  **/
/***    strftime, timestamps    ***/
/****      and redirection     ****/
/*****                        *****/
/**********************************/

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
const char *WELCOME_MESSAGE="";

void printHelp(){
  std::cerr << "-r option with ROOT-FOLDER(directory). ROOT-FOLDER is used as"
  <<           "main folder,where are saved files for each user, default value"
  <<           "is actual folder.\n"
  <<           "-p option with PORT(number). Specifies port, on which server"
  <<           "will be listening, default(implicit) value is \'6677\'\n";
  exit(1);
}

int checkType(char cmd[], char protocol[]){
  if(strncmp(protocol,"HTTP/1.1",9)){
    std::cerr << "Unknown error." << "\n";
    return 400;
  }
  if(strncmp(cmd,"GET",3) && strncmp(cmd,"PUT",3) &&strncmp(cmd,"DELETE",7)){
    std::cerr << "Error: invalid command given.";
    return 400;
  }

  if(!strncmp(cmd,"GET",3))
     return 10;
  if(!strncmp(cmd,"PUT",3))
     return 15;
  if(!strncmp(cmd,"DELETE",7))
     return 20;
  //fail
  return 400;
}
/* Parse path with given command path and command code, 1-GET file, 2-PUT file
 * 3-DELETE file, 4-GET folder, 5-PUT folder, 6-DELETE folder */
int parsePath(char command[], char path[],int cmdCode, std::string *err){
  char tmp[256];
  std::string str = path;
  std::string change;
  char *filename;

  str.erase(0,1);
  std::size_t found=str.find("/");
  // found second /, parse username
  if (found == std::string::npos){
    found=str.find("?");
    str.erase(found,str.length());
  }
  else{
    change=str;
    str.erase(found,str.length());
    found=change.find_last_of("/");
    change.erase(++found,change.length());
  }

  strcpy(tmp,path);
  tmp[0]=' ';
  //FIXME kinda pointless condition
  if(strchr(tmp,'/')){
    filename=strrchr(tmp,'?');
  }
  else
    filename=strrchr(path,'?');

  if(strcmp(filename, "?type=file") == 0){
    if(cmdCode == 10)
      cmdCode = 1;
    if(cmdCode == 15)
      cmdCode = 2;
    if(cmdCode == 20)
      cmdCode = 3;
  }
  else if(strcmp(filename, "?type=folder") == 0){
    if(cmdCode == 10)
      cmdCode = 4;
    if(cmdCode == 15)
      cmdCode = 5;
    if(cmdCode == 20)
      cmdCode = 6;
  }

  if (chdir(str.c_str()) == -1){
    std::cerr << "User Account Not Found" << "\n";
    *err = "fail account";
    return 404;
  }
  //go back
  chdir("../");

  if (change.empty()){
    std::cerr << "Unknown error." << "\n";
    return 400;
  }

  if (chdir(change.c_str()) == -1){
    std::cerr << "Directory not found." << "\n";
    *err = "fail";
    return 404;
  }

  return cmdCode;
}
/* Read file because of GET command sent, read and send filesize by reference
 * as return value buffer of readed folder */
std::string getContent(std::string filename, unsigned long *filesize){

  std::ifstream f1 (filename, std::ifstream::binary);
  std::string content( (std::istreambuf_iterator<char>(f1) ),
                       (std::istreambuf_iterator<char>()   ) );

  f1.close();
  *filesize = content.length();
  return content;
}
/* Execute command which was given by client specified by HTTP header.
 * In put command is also body of http request, this should be read and stored
 * on server root folder site. */
int executeHttp(int code, unsigned long *filesize, std::string *output, int *response){
  char str[80];
  char str1[3];
  std::string tmp;
  std::string root;
  *response=code;

  sscanf((*output).c_str(), "%s %s",str1,str);
  root = str;
  tmp = str;
  // should be or trying to make new account (username)
  std::size_t found = tmp.find_last_of("?");
  tmp.erase(found,tmp.length());
  found=tmp.find_last_of("/");
  //off by one +-1
  tmp.erase(0,++found);

  //probably root folder
  if (tmp.empty()){
    chdir("../");
    tmp = root;
    tmp.erase(0,1);
    found=tmp.find("/");
    tmp.erase(found,tmp.length());
  }

  int type=100;
  struct stat s;
  DIR *dir;
  struct dirent *ent;

  if(stat(tmp.c_str(),&s) == 0){
    if(s.st_mode & S_IFDIR)
      type=1;
    else if(s.st_mode & S_IFREG)
      type=0;
  }

  //GET file | get
  if (code == 1){
    if(type == 1){
      std::cerr << "Not a file.\n";
      return 400;
    }
    else if (type != 0){
      std::cerr << "File not found.\n";
      return 404;
    }
    *output=getContent(tmp, filesize);

    return 200;
  }
  //PUT file | put
  else if(code == 2){

    if(type == 0){
      std::cerr << "Already exists.\n";
      return 400;
    }
    else if (type == 1){
      std::cerr << "Unknown error.\n";
      return 400;
    }
    root = *output;

    std::size_t pos = root.find("\r\n\r\n");
    //remove body
    if (pos != std::string::npos){
      root.erase(0,pos+4);
    }
    else
      return 400;

    pos = root.find_last_of("\r\n\r\n");
    //remove body
    if (pos != std::string::npos){
      root.erase(pos-3,root.length());
    }
    else
      return 400;

    std::ofstream outStream(tmp.c_str(), std::ofstream::binary);
    outStream << root;
    outStream.close();
    (*output)= "";
    return 200;
  }

  //DELETE file | del
  else if(code == 3){
    if (type == 1){
      std::cerr << "Not a file.\n";
      return 400;
    }
    if (remove(tmp.c_str()) != 0){
      std::cerr << "File not found.\n";
      return 404;
    }
    return 200;
  }

  //GET folder | lst
  else if(code == 4){
    //already in least folder
    if(type == 0){
      std::cerr << "Not a directory.\n";
      return 400;
    }
    else if (type != 1){
      std::cerr << "Directory not found.\n";
      return 404;
    }
    dir = opendir(tmp.c_str());
    root = "";
    if(dir != NULL){
      while((ent = readdir(dir))){
        if(ent->d_type != DT_DIR){
          root += ent->d_name;
          root += "\n";
        }
      }
    }
    closedir(dir);
    *output = root;
    *filesize = root.length();
    return 200;
  }

  //PUT folder | mkd
  else if(code == 5){
    if(mkdir(tmp.c_str(), S_IRWXU | S_IRWXG | S_IROTH ) == -1){
      std::cerr << "Already exists.\n";
      return 400;
    }
    return 200;
  }

  //DELETE folder | rmd
  else if(code == 6){
    //DIR *dir;
    if(type ==1){
      if(rmdir(tmp.c_str()) != 0){
        std::cerr << "Directory not empty.\n";
        *output = "Directory not empty.\n";
        *filesize=22;
        return 400;
      }
      return 200;
    }
    else if(type == 0){
      std::cerr << "Not a directory.\n";
      *output = "Not a directory.\n";
      *filesize=18;
      return 400;
    }
    else{
      std::cerr << "Directory not found.\n";
      return 404;
    }
  }
  else{
    std::cerr << "Unknown error." << "\n";
    return 400;
  }
  return 400;
}

/* Parsing packet: filesize and code is given by pointer to store
 * success/failure of operation in code and filesize when get response is given.
 */

std::string parsePacket(std::string msg, const char *buffer, int *code, unsigned long *filesize){

  char command[20];
  char path[256];
  char protocol[20];

  *code=sscanf(buffer,"%s %s %s %*s",command, path, protocol);

  if( *code != 3 ){
    std::cerr << "Unknown error.\n";
    return "fail";
  }
  *code=checkType(command, protocol);

  if (*code == 400){
    std::cerr << "Unknown error." << "\n";
    return "fail";
  }
  //GET, PUT, DELETE recipe
  *code = parsePath(command, path, *code, &msg);

  return msg;
}

int packetLength(std::string input){
  std::size_t found = input.find("Content-Length:");
  int code,conLength = 0;
  if (found != std::string::npos){
    input = input.substr(found);
    code = sscanf(input.data(), "Content-Length:%d", &conLength);
    if (code != 1){
      return -1;
    }
  }

  return conLength;
}

/* Filling output packet with JSON response, with content type,
 * length and encoding. Sending HTTP /1.1 response with appriotiate return code
 */
std::string fillPacket(int code, unsigned long filesize, int type){
  static_cast<void>(filesize);
  std::time_t time = std::time(nullptr);
  struct tm *timeinfo = localtime(&time);
  char timeBuffer[32];
  ::strftime(timeBuffer, sizeof(timeBuffer), "%a, %d %b %G %H:%M:%S %Z", timeinfo);


  /* redirect cout to buffer to send message trough TCP packets */
  std::stringstream buffer;
  auto old = std::cout.rdbuf(buffer.rdbuf());

  std::cout << "HTTP/1.1 ";
  if(code == 200){
    std::cout << code << " OK\r\n";
  }
  else if(code == 404)
    std::cout << code << " Not Found\r\n";
  else if(code ==400)
    std::cout << code << " Bad Request\r\n";
  std::cout << "Date: " << timeBuffer << "\r\n";
  if(type == 1)
    //text/plain
    std::cout << "Content-Type: text/plain\r\n";
  else
    std::cout << "Content-Type: application/octet-stream\r\n";
  std::cout << "Content-Length: " << filesize << "\r\n";
  std::cout << "Content-Encoding: identity\r\n\r\n";

  /* now output contains whole cout above */
  std::string output = buffer.str();
  /* print again to console */
  std::cout.rdbuf(old);

  return output;
}

int main(int argc, char **argv){
  int status;
  int portNumber=6677;
  if (argc == 3){
    if(strncmp(argv[1], "-r", 3) == 0){
      status = mkdir(argv[2],S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
      if (chdir(argv[2]) == -1){
          std::cerr << "Error: Could not access ROOT-FOLDER." << "\n";
          return 1;
        }
    }
    else if(strncmp(argv[1], "-p", 3) == 0){
      if(argc >= 3){
        portNumber=std::stoi(argv[2]);
      }
    }
    else{
      std::cerr << "Error: No valid option found." << "\n";
      printHelp();
    }
  }
  /* implicit port number */
  else if (argc == 5){
     if (strncmp(argv[3], "-p", 3) == 0){
       portNumber = std::stoi(argv[4]);
     }
     if (strncmp (argv[1], "-r", 3) == 0){
        status = mkdir(argv[2],S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        if (chdir(argv[2]) == -1){
          std::cerr << "Error: Could not access ROOT-FOLDER." << "\n";
          return 1;
        }
    }
    else{
      std::cerr << "Error: no valid option found." << "\n";
      printHelp();
    }
  }
  else if (argc != 1){
    std::cerr << "Error: Not enough or too many parameters." << "\n";
    return 1;
  }
  static_cast<void>(status);
    /*  Arguments already set, lets start with TCP connection parse HTML request.
}       Start communication conncurent. Be aware of soc errors and bad requests.
      Common codes to send - 200 OK, 404 No object found 400 Bad request. */
  int rc;
  int welcomeSocket;
  struct sockaddr_in6 serverAddress;
  struct sockaddr_in6 clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);

  if ((welcomeSocket = socket(PF_INET6, SOCK_STREAM, 0)) < 0){
    perror("Error: socket() failed.");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddress, 0 , sizeof(serverAddress));
  serverAddress.sin6_family= AF_INET6;
  serverAddress.sin6_addr = in6addr_any;
  serverAddress.sin6_port = htons(portNumber);

  if ((rc = bind (welcomeSocket, (struct sockaddr*) &serverAddress,sizeof(serverAddress))) < 0){
    perror("bind() error.");
    exit(EXIT_FAILURE);
  }
  if ((rc = listen (welcomeSocket, 10)) < 0){
    perror("listen() error.");
    exit(EXIT_FAILURE);
  }

  //be carefull ZOMBIES !!! TODO
  //signal(SIGCHLD, SigCatcher);

  while(true){
    int comm_socket = accept(welcomeSocket, (struct sockaddr*) &clientAddress,
                             &clientAddressLength);
    if (comm_socket <= 0)
      continue;

    int pid = fork();
    if(pid < 0){
      perror("fork() error.");
      exit(EXIT_FAILURE);
    }
    /* new process created */
    if (pid == 0){
      //int child_pid = getpid();
      close(welcomeSocket);
      send (comm_socket, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE), 0);

      char buff[4097];
      int res,code,total,contentLength = 0;
      unsigned long filesize = 0;
      std::string str;

      res = recv (comm_socket, buff, 4096, 0);
      if(res < 0){
        perror("Error in receive TCP packet.");
        break;
      }
      str = std::string(buff,res);
      contentLength = packetLength(buff);
      std::string tail = parsePacket(str,str.data(), &code ,&filesize);

      if (tail == "fail" || tail == "fail account" || contentLength == -1){
        str = fillPacket(code, filesize , 0);
        str += tail + "\r\n\r\n";
        int suc=send(comm_socket, str.data(), str.size(), 0);
        if (suc < 0){
           perror("ERROR: send packet.");
        }
        close(comm_socket);
        exit(0);
      }
      str.clear();
      total = 0;
      while(total < contentLength){
        memset(buff, 0 , 4097);
        res = recv (comm_socket, buff, 4096, 0);
        if (!res)
          break;
        if(res < 0){
          perror("Error in receive TCP packet.");
          break;
        }
        total += res;
        str += std::string(buff,res);

        if (str.find("\r\n\r\n") != std::string::npos)
          break;
        /* received message */
      }
      tail += str;
      int typeHttp = 0;
      code = executeHttp(code, &filesize, &tail, &typeHttp);
      std::string tmp = "";
      tmp = fillPacket(code, filesize, typeHttp);

      int suc = send(comm_socket, tmp.data(), tmp.size(), 0);
      if (suc < 0)
        perror("ERROR: send packet.");

      if (filesize != 0)
        tmp = tail + "\r\n\r\n";
      suc = send(comm_socket, tmp.data(), tmp.size(), 0);
      if (suc < 0){
         perror("ERROR: send packet.");
      }

      close(comm_socket);
      exit(0);
    }
    /* welcome process */
    else{
      close(comm_socket);
    }
  }
  return 0;
}
