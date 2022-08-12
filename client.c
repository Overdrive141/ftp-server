#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>


void cmd_port(char*, char*, int, int, int, char**);
void cmd_list(char*, char*, int, int, int, char**);
void cmd_retr(char*, char*, int, int, int, char**);
void cmd_stor(char*, char*, int, int, int, char**);

int main(int argc, char *argv[]){
  char message[255];
  int server, portNumber, pid, n;
  struct sockaddr_in servAdd;     // server socket address

    // For FIFO Data Connection
  int fd;
  // FIFO file path
  char * myfifo=malloc(255);

  
 if(argc != 3){
    printf("Call model: %s <IP Address> <Port Number>\n", argv[0]);
    exit(0);
  }

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0){
     fprintf(stderr, "Cannot create socket\n");
     exit(1);
  }

  servAdd.sin_family = AF_INET;
  sscanf(argv[2], "%d", &portNumber);
  servAdd.sin_port = htons((uint16_t)portNumber);

  if(inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0){
  fprintf(stderr, " inet_pton() has failed\n");
  exit(2);
}

 if(connect(server, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){
  fprintf(stderr, "connect() has failed, exiting\n");
  exit(3);
 }

  read(server, message, 255);
  fprintf(stderr, "message received: %s\n", message);

  

  pid=fork();

  if(pid)
     while(1)                         /* reading server's messages */
       if(n=read(server, message, 255)){
          message[n]='\0';
          fprintf(stderr,"%s\n", message);
          if(!strcasecmp(message, "Bye\n")){
	    //  kill(pid, SIGTERM);
             exit(0);
           }
           if(!strcasecmp(message, "550 Failed to create pipe.")){
                  myfifo=NULL;
                  myfifo=realloc(myfifo, 255);
           }
         }

  if(!pid)                           /* sending messages to server */
     while(1)
      if(n=read(0, message, 255)){
         char command[5]; // Decoded Command
         char arg[50]; // Decoded args

         message[n]='\0';
         write(server, message, strlen(message)+1);
         sscanf(message,"%s %s",command, arg);
         if(!strcasecmp(message, "Bye\n")){  // QUIT
           // kill(getppid(), SIGTERM);
            exit(0);
          }

          else if(!strcasecmp(command, "PORT")){
            cmd_port(command, arg, server, n, fd, &myfifo);
          }
          else if(!strcasecmp(command, "LIST")){
            cmd_list(command, arg, server, n, fd, &myfifo);
          }
          else if(!strcasecmp(command, "RETR")){
            cmd_retr(command, arg, server, n, fd, &myfifo);
          }
          else if(!strcasecmp(command, "STOR")){
            cmd_stor(command, arg, server, n, fd, &myfifo);
          }

      }
}

void cmd_port(char command[], char arg[], int sd, int n, int fd, char **fifoname){
  // Open a new pipe
  char tmpFifoName[255];
  char response[255];

  // unlink(arg);

  char cwd[1024], cwd_orig[1024];
  getcwd(cwd_orig,1024);

  // if(strlen(arg)>0&&arg[0]!='-'){
  //   chdir("..");
  // }
  // else {
  //   // No args passed
  // }
  fprintf(stderr, "Client: Arg %s", arg);

  sprintf(tmpFifoName,"/home/hammad6/Desktop/project/tmp/%s", arg);

  // if(mkfifo(tmpFifoName, 0777) != 0) {
  //   sprintf(response, "550 Failed to create pipe.");
  // } else {
    strcpy(*fifoname,tmpFifoName);
    fprintf(stderr, "Client: Currently %s", *fifoname);
    // sprintf(response, "200 Command Okay.");
  // }

  // if mkfifo is successful, return 200 Command Okay
  // if fail, return Failed Pipe


  // if((write(sd, response, strlen(response)+1)) < 0){
  //       fprintf(stderr,"Failed to execute PORT command", response);
  // }

  // chdir(cwd_orig);


}

void cmd_list(char command[], char arg[], int sd, int n, int fd, char **fifoname){
  char ch;
  pid_t pid;

  if(**fifoname == '\0'){
    fprintf(stderr, "Client: No Connection"); // When not set
  } else {
    fprintf(stderr, "Client: FifoName %s", *fifoname); // When set

    while((fd=open(*fifoname, O_RDONLY))==-1){
      fprintf(stderr, "trying to connect to %s\n", *fifoname);
      sleep(1);
    }
    close(fd);

    // sleep(1);

    fd = open(*fifoname, O_RDONLY);

    if(fd < 0){
        perror("ERROR"); // When set
    }

    // //     fprintf(stderr, "Client: FD %d", fd);

    while(read(fd, &ch, 1) == 1)
      fprintf(stderr, "%c", ch);
    close(fd);
    *fifoname=NULL;
    *fifoname=realloc(*fifoname, 255);
  }

}

void cmd_retr(char command[], char arg[], int sd, int n, int fd, char **fifoname){
  char ch;
  pid_t pid;
  int fd2;
  char buf[1];

  if(**fifoname == '\0'){
    fprintf(stderr, "Client: No Connection"); // When not set
  } 
  else {

    // while((fd=open(*fifoname, O_RDONLY))==-1){
    //   fprintf(stderr, "trying to connect to %s\n", *fifoname);
    //   sleep(1);
    // }
    // close(fd);

    // sleep(1);

    fd = open(*fifoname, O_RDONLY);

    if(fd < 0){
        perror("ERROR"); // When set
    }

    if( (fd2 = open(arg, O_CREAT | O_WRONLY | O_TRUNC, 0777)) < 0)
    {
      fprintf(stderr, "File incorrect or does not exist");
    }

    chmod(arg, 0777);

    int nx;
    int fileSize=0;
    int totalBytesRead=0;

    read(fd, &fileSize, sizeof(int));
    int offset = lseek(fd, 4, SEEK_SET);
    fprintf(stderr, "Client: Size %d", fileSize);
    // close(fd);
    // sleep(2);

    // while((fd=open(*fifoname, O_RDONLY))==-1){
    //   fprintf(stderr, "trying to connect to %s\n", *fifoname);
    //   sleep(1);
    // }
    // close(fd);

    // fd = open(*fifoname, O_RDONLY);

    while(fileSize > 0)
    {
      if(read(fd, &ch, 1) > 0)
        {
          totalBytesRead++;
          if(totalBytesRead > sizeof(int)){
            fileSize = fileSize - 1;
            write(fd2, &ch, 1);
          }

          // fprintf(stderr, "%c", ch);
          // if(write(fd2, &ch, nx) != 1)
          // {
          //     fprintf(stderr, "Client: Problem in Writing");
          //         break;
          // }
          // if(nx == -1){
          //   fprintf(stderr, "Client: Problem in Reading");
          // }
        }
    }


   fprintf(stderr, "Client: Finished");

    close(fd);
    close(fd2);
    *fifoname=NULL;
    *fifoname=realloc(*fifoname, 255);



  }

}

void cmd_stor(char command[], char arg[], int sd, int n, int fd, char **fifoname){
  
   char response[255];
  int fd2;


  if( (fd2 = open(arg, O_RDONLY)) < 0)
  {
    fprintf(stderr, "File incorrect or does not exist");
  }
  else
  {

    if(**fifoname == '\0')
    {
      fprintf(stderr, "Server: No Connection"); // When not set
    } 
    
    else 
    {
          lseek(fd2, 0, SEEK_SET);
          char buf[1];
          char myFifo[255];
          off_t fileSize = lseek(fd2, 0, SEEK_END);
          long int nx;

          sprintf(myFifo,"%s", *fifoname);
          fprintf(stderr, "Client: MyFifo Name %s. File Size: %ld\n", myFifo, fileSize);

          lseek(fd2, 0, SEEK_SET);
          
          fprintf(stderr, "200 Beginning File Transfer");

          fd = open(myFifo, O_WRONLY);

          fprintf(stderr, "Client: File Descriptor %d", fd2); // When set

          if(fd < 0){
              fprintf(stderr, "Client: ERROR in Opening Fifo FD"); // When set
          }

          int convFileSize = (int)fileSize;

          write(fd, &fileSize, sizeof(off_t)); // Send fileSize in FiFO first x byes. Client reads that then runs loop for that amount of bytes
          fprintf(stderr, "Client: Sent File Size %d", convFileSize);

          while(fileSize > 0){
                    
            // fprintf(stderr, "fileSize %ld", fileSize);
            if( (nx=read(fd2, buf, 1)) > 0)
            {
              // fileSize = lseek(fd2, 1, SEEK_CUR);
              fileSize = fileSize - nx;
              if(write(fd, buf, nx) != 1)
              {
                  fprintf(stderr, "Client: Problem in Writing");
                  break;
              }

              if(nx==-1)
              {
                  fprintf(stderr, "Client: Problem in Reading");
                  break;
              }
            }
          }

  


          close(fd);
          *fifoname=NULL;
          *fifoname=realloc(*fifoname, 255);
          unlink(myFifo);
          fprintf(stderr, "File stored at Server");

    }
  }

  close(fd2);


}