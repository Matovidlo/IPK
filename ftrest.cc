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
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <regex>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

void printHelp(){
  std::cerr << "del to delete file/directory req. parameter REMOTE-PATH \
         \nget to copy file from REMOTE-PATH to local directory with LOCAL-PATH\
         \nput to copy file from LOCAL-PATH to directory REMOTE-PATH (required\both parameters 2nd argument as REMOTE-PATH and 3th as LOCAL-PATH)\
         \nlst to list actual directory on server(same as ls on unix-based OS)\
         \nmkd to create directory specified in REMOTE-PATH on server\
         \nrmd to delete directory specified in REMOTE-PATH from server\n";
  exit(1);
}

int parseInput(const char *argument){
  if (!strncmp (argument, "del", 4)){
    return 0;
  }
  else if (!strncmp (argument, "get", 4)){
    return 1;
  }
  else if (!strncmp (argument, "put", 4)){
    return 2;
  }
  else if (!strncmp (argument, "lst", 4)){
    return 3;
  }
  else if (!strncmp (argument, "mkd", 4)){
    return 4;
  }
  else if (!strncmp (argument, "rmd", 4)){
    return 5;
  }
  else
    printHelp();
  return -1;
}

/*  parse regex with http request, localhost or ip address to resolve with
    gethostname method by DNS, returns reference to portNumber and ip address or name which should go to DNS gethostname method  */
std::string getServerName(char (*string)[256]){
  std::string str=*string;
  std::size_t found = str.find_last_of(":");

  if (found != std::string::npos){
    std::string strTmp = str;
    strTmp = strTmp.erase(found,str.length());

    str = str.substr(++found,str.length());

    found = strTmp.find(":");
    if (found != std::string::npos){
      /* not a http protocol */
      if(strTmp.substr(0,found) != "http"){
        std::cerr << "Unknown error.\n";
        exit(1);
      }
      strTmp = strTmp.substr(found+3,str.length());
    }
    else{
      ;
    }
    strcpy(*string,strTmp.c_str());
  }
  else{
    std::cerr << "Error: port not specified!\n";
    exit(1);
  }
  return str;
}

/* GET reply from server, got content lets get it to shape of file */
void getFolder(std::string filename, std::string output){
  std::size_t pos;

  pos = output.find("\r\n\r\n");
  if (pos != std::string::npos)
    output.erase(0, pos+4);
  else{
    return;
  }

  pos = output.find_last_of("\r\n\r\n");
  //remove body
  if (pos != std::string::npos){
    output.erase(pos-3,output.length());
  }
  else
    return;
  std::ofstream ofs (filename, std::ofstream::binary);
  ofs << output;
  ofs.close();
}

/*  Get content-length parameter of HTTP header(filesize in bytes of file or
    folder). Returns buffer of readed bytes and filesize as pointer to int */
std::string getContentLength(std::string filename, unsigned long *filesize){
  FILE *doc;
    /* filename is type of file */
  /* path is probably valid let's open file */
  doc=fopen(filename.c_str(),"rb");
  if(doc == NULL){
    perror("ERROR: could not open file.");
    exit(EXIT_FAILURE);
  }
  fseek(doc,0,SEEK_END);
  *filesize =ftell(doc);
  fclose(doc);

  std::ifstream f1 (filename, std::ifstream::binary);
  std::string content( (std::istreambuf_iterator<char>(f1) ),
                       (std::istreambuf_iterator<char>()   ) );
  f1.close();

  return content;
}

std::string parsePath(std::string filename){
  if (chdir (filename.c_str()) == -1){
    std::size_t found = filename.find_last_of("/");
    if(found != std::string::npos){
      std::string path = filename;
      path.erase(found,filename.length());
      if (chdir(path.c_str()) == -1){
        perror("ERROR: bad path given.");
        exit(EXIT_FAILURE);
      }
      filename=filename.substr(++found);
    }
  }

  return filename;
}

void listFolder(std::string input){
  std::size_t pos;
  int code, conLength = 0;
  pos = input.find("Content-Length: ");
  if (pos != std::string::npos){
    input = input.substr(pos);
    code = sscanf(input.data(), "Content-Length: %d", &conLength);
    if (code != 1){
      return;
    }
  }
  // no content given
  if(conLength == 0){
    return;
  }

  pos = input.find("\r\n\r\n");

  if (pos != std::string::npos)
    input.erase(0,pos+4);
  else{
    return;
  }

  pos = input.find_last_of("\r\n\r\n");
  //remove body
  if (pos != std::string::npos){
    input.erase(pos-3,input.length());
  }
  else
    return;
  std::cout << input;
}

int parsePacket(int flag, std::string input, std::string path){
  int reply,code;
  code = sscanf(input.c_str(), "HTTP/1.1 %d",&reply);
  if (code != 1){
    std::cerr << "Unknown error.\n";
    return 1;
  }

  if (reply == 200){
    if (flag == 1)
      getFolder(path, input);
    else if (flag == 3)
      listFolder(input);
    return 0;
  }
  else if (reply == 400){
    // Parse content of packet
    std::size_t found = input.find("Not a directory");
    if (found != std::string::npos){
      std::cerr << "Not a directory.\n";
      return 1;
    }
    found = input.find("Directory not empty");
    if (found != std::string::npos){
      std::cerr << "Directory not empty.\n";
      return 1;
    }

    switch(flag){
      case 0:
      case 1:
      // del and get
        std::cerr << "Not a file.\n";
        break;
      // put and mkd
      case 2:
      case 4:
        std::cerr << "Already exists.\n";
        break;
      case 3:
      //lst
        std::cerr << "Not a directory.\n";
        break;
      case 5:
      //rmd
        break;
    }
    return 1;
  }
  else if (reply == 404){
    std::size_t found = input.find("fail account");
    if (found != std::string::npos){
      std::cerr << "User Account Not Found.\n";
      return 1;
    }
    switch(flag){
      case 0:
      case 1:
      case 2:
        std::cerr << "File not found.\n";
        break;
      case 3:
      case 5:
      case 4:
        std::cerr << "Directory not found.\n";
        break;

    }
    return 1;
  }
  return 1;
}


/******************* MAIN FUNCTION ********************/
/******************************************************/
int main (int argc, char **argv){
  int clientSocket, portNumber, byteStx, byteSrx;
  char serverHostname[256];
  struct hostent *server;
  struct sockaddr_in serverAddress;

  int currentFlag;
  if (argc > 1){
    currentFlag = parseInput(argv[1]);
    if (argc > 2 && argc <= 4){
      /* check for http:// protocol specified in second parameter */
      const char *remotePath(argv[2]);
      char realPath[256];

      sscanf(remotePath,"%256s", serverHostname);
      std::string tmp;

      tmp = getServerName(&serverHostname);
      sscanf(tmp.c_str(), "%d%256s", &portNumber,realPath);
      /* Get address server with help of DNS service */
      if ((server = gethostbyname(serverHostname)) == NULL){
        fprintf(stderr,"ERROR: no such host as %s\n", serverHostname);
        exit(EXIT_FAILURE);
      }

      /* Initialization of structure serverAddress */
      bzero((char *) &serverAddress, sizeof(serverAddress));
      serverAddress.sin_family = AF_INET;
      bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr,
             server->h_length);
      serverAddress.sin_port = htons(portNumber);

      /* Create socket */
      if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        perror("Unknown error.");
        exit(EXIT_FAILURE);
      }

      /* create HTTP header which should be sended by TCP channel */
      /* timezone */
      time_t time = std::time(nullptr);
      struct tm *currentTime = localtime(&time);
      char dateBuffer[256];
      ::strftime(dateBuffer, sizeof(dateBuffer), "%a, %d %b %G %H:%M:%S %Z", currentTime);

      std::string localPath;
      /* redirect cout to buffer to send message trough TCP packets */
      std::stringstream buffer;
      auto old = std::cout.rdbuf(buffer.rdbuf());

      switch(currentFlag){
      //del argument
      case 0:
        std::cout << "DELETE " << realPath << "?type=file HTTP/1.1" << "\r\n";
        break;
      //get argument
      case 1:
        std::cout << "GET " << realPath << "?type=file HTTP/1.1" << "\r\n";
        break;
      //put argument
      case 2:
        if (argc != 4){
        /* required parameter. Raise error */
          std::cerr <<"ERROR: not enough parameters.\n";
          exit(1);
        }
        std::cout << "PUT " << realPath << "?type=file HTTP/1.1" << "\r\n";
        break;
      //lst argument
      case 3:
        std::cout << "GET " << realPath << "?type=folder HTTP/1.1" << "\r\n";
        break;
      //mkd argument
      case 4:
        std::cout << "PUT " << realPath << "?type=folder HTTP/1.1" << "\r\n";
        break;
      //rmd argument
      case 5:
        std::cout << "DELETE " << realPath << "?type=folder HTTP/1.1" << "\r\n";
        break;
      //failure of parsing arguments
      default:
        printHelp();
      }
      if (argc == 4)
        localPath=argv[3];
      /* function to std::cout */
      std::cout << "Host: " << serverHostname << "\r\n";
      std::cout << "Date: " << dateBuffer << "\r\n";
      std::cout << "Accept: application/json\r\n";
      std::cout << "Accept-Encoding: identity\r\n";

      std::string fileContent;
      unsigned long filesize = 0;

      // flag is set to PUT, file should be read, else it should be written to it
      if (currentFlag == 2){
        /* FIXME content type magic function http://stackoverflow.com/questions/423626/get-mime-type-from-filename-in-c */
        // getContentType(filePut);
        localPath=parsePath(localPath);
        fileContent=getContentLength(localPath,&filesize);
      }
      std::cout << "Content-Type: " << "application/octet-stream" << "\r\n";
      std::cout << "Content-Length: " << filesize <<"\r\n\r\n";
      /* now output contains whole cout above */
      std::string output = buffer.str();
      /* print again to console */
      std::cout.rdbuf(old);

      /* refuse connection if not valid server hostname */
      if(connect (clientSocket,
         (const struct sockaddr *) &serverAddress,
         sizeof(serverAddress)) != 0){
        perror("Unknown error.");
        close(clientSocket);
        exit(EXIT_FAILURE);
      }

      /* send message to server */
      char packetContent [4097];

      byteStx = send (clientSocket, output.data(), output.size(), 0);
      if (byteStx < 0)
        perror("Error in send TCP packet.");

      if (!fileContent.empty()){
        output = fileContent + "\r\n\r\n";
        byteStx = send (clientSocket, output.data(), output.size(), 0);
        if (byteStx < 0)
          perror("Error in send TCP packet.");
      }


      /* incomming message from server */
      //static_cast<const char*>(const char*)(&hdr)where &hdr=long data_length;
      output="";

      std::memset(packetContent, 0, 4097);
      byteSrx = recv (clientSocket, packetContent, 4096,0);
      if(byteSrx < 0){
        perror("Error in receive TCP packet.");
      }
      output += std::string(packetContent,byteSrx);
      std::string tail = "";

      while(1){
        std::memset(packetContent, 0 , 4097);
        byteSrx = recv (clientSocket, packetContent, 4096, 0);
        if (!byteSrx)
          break;
        if(byteSrx < 0){
          perror("Error in receive TCP packet.");
          break;
        }
        tail += std::string(packetContent,byteSrx);

        // end of header
        if (tail.find("\r\n\r\n") != std::string::npos)
          break;
        /* received message */
      }
      /* parse localPath if exists, when not get the same filename as found on server side */
      output += tail;
      if (currentFlag == 1 && localPath.size()){
        localPath = parsePath(localPath);
      }
      else if (currentFlag == 1 && !localPath.size()){
        localPath = realPath;
        std::size_t found = localPath.find_last_of("/");
        localPath = localPath.substr(++found);
      }
      int code = 0;
      code = parsePacket(currentFlag, output, localPath);
      close(clientSocket);
      return code;

    }
    printHelp();
  }
  printHelp();
  return 1;
}
