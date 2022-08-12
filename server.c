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


void child(int);
void disconnectClients(int);

// Add a method for response with message var maybe. Has to be child fork only
void cmd_user(char*, int, int);
void cmd_cwd(char*, char*, int, int);
int cmd_port(char*, char*, int, int, int, char**);
void cmd_list(char*, char*, int, int, int, char**);
void cmd_cdup(char*, char*, int, int); // noargs like cwd but changes to parent dir of current dir. sort of like cd ..


void cmd_rein(char*, char*, int, int); // noargs This command terminates a USER, flushing all I/O and account information, except to allow any transfer in progress to be completed.
void cmd_quit(char*, char*, int, int); // no args This command terminates a USER and if file transfer is not
                  //in progress, the server closes the control connection.  If
            //file transfer is in progress, the connection will remain
           // open for result response and the server will then close it.
            //If the user-process is transferring files for several USERs

void cmd_retr(char*, char*, int, int, int, char**); // args: retr <filename> (OPEN NEW FIFO HERE - not same as PORT)
//This command causes the server-DTP to transfer a copy of the
            //file, specified in the pathname, to the server- or user-DTP
           // at the other end of the data connection. 


void cmd_stor(char*, char*, int, int, int, char**); // args: stor <filename> client sends file to server at server site. Overwrites if same file present


void cmd_appe(char*, char*, int, int, int, char**); // same as stor but except appends to file in server side if filename same

void cmd_rest(); // REST <SP> <marker> <CRLF> 
// his command does not cause file transfer but skips over the file to the specified data checkpoint
//This command shall be immediately followed
//by the appropriate FTP service command which shall cause
  //          file transfer to resume.

void cmd_rnfr(char*, char*, int, int, char**); /*  RNFR <SP> <pathname> <CRLF>
 This command specifies the old pathname of the file which is
            to be renamed.  This command must be immediately followed by
            a "rename to" command specifying the new file pathname.*/


void cmd_rnto(char*, char*, int, int, char**); //RNTO <SP> <pathname> <CRLF> RENAMETO
// First do rnfr then rnto
/*This command specifies the new pathname of the file
            specified in the immediately preceding "rename from"
            command.*/

void cmd_abor(char*, char*, int, int, int, char**); // no args: This command tells the server to abort the previous FTP service command and any associated transfer of data
void cmd_stat(); // stat [pathName]

void cmd_dele(char *, char *, int, int); // DELE <pathname>
//This command causes the file specified in the pathname to be
 // deleted at the server site

void cmd_rmd(char *, char *, int, int); // rmd <pathname> REMOVE DIRECTORY
/*
This command causes the directory specified in the pathname
            to be removed as a directory (if the pathname is absolute)
            or as a subdirectory of the current working directory (if
            the pathname is relative)
*/

void cmd_mkd(char *, char *, int, int); // rmd <pathname>
/*
This command causes the directory specified in the pathname
            to be created as a directory (if the pathname is absolute)
            or as a subdirectory of the current working directory (if
            the pathname is relative).*/

void cmd_pwd(char*, char*, int, int); // no args
/*
his command causes the name of the current working
            directory to be returned in the reply
*/


void cmd_noop(char*, int, int); // no args 
// It specifies no action other than that the server send an OK reply


int main(int argc, char *argv[]){  
  int sd, client, portNumber, status;
  struct sockaddr_in servAdd;      // client socket address


  
 if(argc != 2){
    printf("Call model: %s <Port Number>\n", argv[0]);
    exit(0);
  }
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    fprintf(stderr, "Cannot create socket\n");
    exit(1);
  }
  sd = socket(AF_INET, SOCK_STREAM, 0);
  servAdd.sin_family = AF_INET;
  servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
  sscanf(argv[1], "%d", &portNumber);
  servAdd.sin_port = htons((uint16_t)portNumber);
  
  bind(sd, (struct sockaddr *) &servAdd, sizeof(servAdd));
  listen(sd, 5);

  signal(SIGINT, disconnectClients);
  signal(SIGTSTP, disconnectClients);

  while(1){
    printf("Waiting for a client to connect\n");
    client = accept(sd, NULL, NULL);
    printf("Client connected\n");

    // Save PID of all childs maybe for sighandlers to disconnect all clients (forked child)
    
    if(!fork())
      child(client);

    close(client);
    waitpid(0, &status, WNOHANG);
  }
}

void child(int sd){
	char message[255];
  int n, pid;
  
  // For FIFO Data Connection
  int fd;
  // FIFO file path
  char * myfifo=malloc(255);
  char * fileName=malloc(255);

  write(sd, "Welcome to FTP Server! Start typing a command", 35);

	// pid=fork();

	// if(pid)                         /* reading client messages */
    while(1)
	   if(n=read(sd, message, 255)){
      char command[5]; // Decoded Command
       char arg[50]; // Decoded args

       memset(arg, 0, 50);
       memset(command, 0, 5);

	     message[n]='\0';

       sscanf(message,"%s %s",command, arg);

              // if(strlen(arg)>0)

       fprintf(stderr,"Client entered the command: %s with args: %s\n", command, arg);

       if(!strcasecmp(message, "cd\n")){
         //traverseDirs
         fprintf(stderr,"I am %s", message);
       }

       

       // Commands
       else if(!strcasecmp(command, "USER")){
         cmd_user(message, sd, n);
       }
       else if(!strcasecmp(command, "CWD")){
         cmd_cwd(command, arg, sd, n);
       }
      else if(!strcasecmp(command, "PORT")){
         cmd_port(command, arg, sd, n, fd, &myfifo);
       }
      else if(!strcasecmp(command, "CDUP")){
        cmd_cdup(command, arg, sd, n);
      }
       else if(!strcasecmp(command, "REIN")){
         exit(0);
       }
       else if(!strcasecmp(command, "QUIT")){
         cmd_quit(command, arg, sd, n);
       }

       else if(!strcasecmp(command, "RETR")){
        cmd_retr(command, arg, sd, n, fd, &myfifo);
       }
       else if(!strcasecmp(command, "STOR")){
        cmd_stor(command, arg, sd, n, fd, &myfifo);
       }
       else if(!strcasecmp(command, "APPE")){
        cmd_appe(command, arg, sd, n, fd, &myfifo);
       }
       else if(!strcasecmp(command, "REST")){}
       else if(!strcasecmp(command, "RNFR")){
         cmd_rnfr(command, arg, sd, n, &fileName);
       }
       else if(!strcasecmp(command, "RNTO")){
         cmd_rnto(command, arg, sd, n, &fileName);
       }
       else if(!strcasecmp(command, "ABOR")){
         cmd_abor(command, arg, sd, n, fd, &myfifo);
       }
       else if(!strcasecmp(command, "DELE")){
         cmd_dele(command, arg, sd, n);
       }
       else if(!strcasecmp(command, "RMD")){
         cmd_rmd(command, arg, sd, n);
       }
       else if(!strcasecmp(command, "MKD")){
         cmd_mkd(command, arg, sd, n);
       }
       else if(!strcasecmp(command, "PWD")){
         cmd_pwd(command, arg, sd, n);
       }
       else if(!strcasecmp(command, "LIST")){
         cmd_list(command, arg, sd, n, fd, &myfifo);
       }
      //  else if(!strcasecmp(command, "STAT")){}
       else if(!strcasecmp(command, "NOOP")){
         cmd_noop(arg, sd, n);
       }
	   }

}

void disconnectClients(int sigNumber){

  // while(1){
  //   // send close command to all forked childs? Through kill in same pgid
  //   close(client);
  //   waitpid(0, &status, WNOHANG);
  // }

  // kill(getpgrp(), SIGTERM);
  exit(0);

}

// void traverseDirectories()

void cmd_user(char message[], int sd, int n){

      message[n]='\0';
      int FTP_LOGIN_CODE = 230;

      sprintf(message, "%d User logged in", FTP_LOGIN_CODE);

	    if((write(sd, message, strlen(message)+1)) < 0){
         fprintf(stderr,"Failed to execute USER command", message);
      }

}

/** CWD command */
void cmd_cwd(char command[], char arg[], int sd, int n)
{

  char message[255];

  fprintf(stderr, "Sample %s %s\n", command, arg);

  // if(state->logged_in){
    if(chdir(arg)==0){
      // printf("%s\n", getcwd(s, 100));
      sprintf(message, "250 Directory successfully changed to %s", arg);
    }else{
      sprintf(message, "550 Failed to change directory.");
    }
  // }else{
  //   state->message = "500 Login with USER and PASS.\n";
  // }
  // write_state(state);
  	    if((write(sd, message, strlen(message)+1)) < 0){
         fprintf(stderr,"Failed to execute USER command", message);
      }

}

int cmd_port(char command[], char arg[], int sd, int n, int fd, char **fifoname){
  // Open a new pipe
    char tmpFifoName[255];
    char response[255];

  // unlink(arg);

  // if(strlen(arg)>0&&arg[0]!='-'){
  //   
  // }
  // fprintf(stderr, "1tmpFifoName %s\n", tmpFifoName);
  sprintf(tmpFifoName,"/home/hammad6/Desktop/project/tmp/%s", arg);

  fprintf(stderr, "2tmpFifoName %s\n", tmpFifoName);

  if(mkfifo(tmpFifoName, 0666) != 0) {
    sprintf(response, "500 Failed to create pipe.");
  } else {
    // sprintf(*fifoname, "%s", tmpFifoName);
    chmod(tmpFifoName, 0666);
    strcpy(*fifoname,tmpFifoName);

    // *fifoname = tmpFifoName;
    fprintf(stderr, "Currently %s\n", *fifoname);
    sprintf(response, "200 Command Okay.");
  }

  // if mkfifo is successful, return 200 Command Okay
  // if fail, return Failed Pipe


  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute PORT command", response);
  }

  return 0;


}

void cmd_list(char command[], char arg[], int sd, int n, int fd, char **fifoname){

  char errRes[255];

  fprintf(stderr, "Server: ListCommand %s", *fifoname); // When set
  

  if(**fifoname == '\0'){
    fprintf(stderr, "Server: No Connection\n"); // When not set
    sprintf(errRes, "425 Can't open data connection.\n");
    if((write(sd, errRes, strlen(errRes)+1)) < 0){
        fprintf(stderr,"Failed to execute LIST command", errRes);
    }
  } else {
    fprintf(stderr, "Server: Else %s\n", *fifoname); // When set


  

    // loop within directory from arg and write to client until all files are passed (!=null)
    // On client side open fifo too and keep reading until server sends a null/eof signal
    // Client stops reading and unlink on client side.


     struct dirent *entry;
     struct stat statbuf;
     struct tm *time;
     char timebuff[80], current_dir[1024];
     int connection;
     time_t rawtime;
     char response[1024];

     memset(response, 0, 1024);

     char cwd[1024], cwd_orig[1024];
      memset(cwd,0,1024);
      memset(cwd_orig,0,1024);
    
//     /* Later we want to go to the original path */
     getcwd(cwd_orig,1024);
    
//     /* Just chdir to specified path */
     if(strlen(arg)>0&&arg[0]!='-'){
       chdir(arg);
     }
    
     getcwd(cwd,1024); // dir passed in arg
     DIR *dp = opendir(cwd);

     sprintf(response, "150 File status okay.\n125 Data connection already open; transfer starting.\n");
     if((write(sd, response, strlen(response)+1)) < 0){
          fprintf(stderr,"Failed to execute LIST command", response);
     }

     sleep(1);

     if(!dp){
       sprintf(response, "550 Failed to open directory.");
        if((write(sd, response, strlen(response)+1)) < 0){
            fprintf(stderr,"Failed to execute LIST command", response);
        }
     }else{
         sprintf(response, "150 The directory listing is listed below: ");
         puts(response);

         char myFifo[255];
         memset(myFifo, 0, 255);
         sprintf(myFifo,"%s", *fifoname);
         fprintf(stderr, "Server: MyFifo Name %s\n", myFifo);

        // int fd2 = open("new.txt",  O_CREAT|O_WRONLY, 0777);
        fd = open(myFifo, O_WRONLY);

        if(fd < 0){
            fprintf(stderr, "Server: ERROR in FD"); // When set
        }

//         fprintf(stderr, "Server: FD %d\n", fd1);

         while(entry=readdir(dp)){


           
          if(stat(entry->d_name,&statbuf)==-1){
            fprintf(stderr, "FTP: Error reading file in dir...\n");
          }else{
            char *perms = malloc(9);
            memset(perms,0,9);

// //             /* Convert time_t to tm struct */
            rawtime = statbuf.st_mtime;
            time = localtime(&rawtime);
            strftime(timebuff,80,"%b %d %H:%M",time);
            // str_perm((statbuf.st_mode & ALLPERMS), perms);
            dprintf(fd,
                "%c%s %5d %4d %4d %8d %s %s\r\n", 
                (entry->d_type==DT_DIR)?'d':'-',
                perms,statbuf.st_nlink,
                statbuf.st_uid, 
                statbuf.st_gid,
                statbuf.st_size,
                timebuff,
                entry->d_name);
          }

                      // sprintf(response, "%s/%s", argv[1], entry->d_name);
//                   // stat(response, &statbuf);
//         // printf("%zu",mystat.st_size);
//         // printf(" %s\n", entry->d_name);
              // fprintf(stderr, "Testing Result %s", entry->d_name); // When set


              char dirName[255];
              strcpy(dirName, entry->d_name);
              // puts(dirName);
              // fprintf(stderr, "Testing Result %s", entry->d_name); // When set
              dirName[254] = '\0';
              // sprintf(dirName, "%s\n", entry->d_name);

              // write(fd1, fifoname, strlen(*fifoname));
              // write(fd, dirName, strlen(dirName));
              // write(fd2, dirName, strlen(dirName));
         }

            close(fd);
            memset(*fifoname, 0, 255);
            // *fifoname=NULL;
            // *fifoname=realloc(*fifoname, 255);
            // close(fd2);
            unlink(myFifo);

           sprintf(response, "250 Requested file action okay, completed.\n");

     }
     closedir(dp);
     chdir(cwd_orig);


     if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute LIST command", response);
     }





  }

}

/** CDUP command */
void cmd_cdup(char command[], char arg[], int sd, int n)
{
  char response[255];
  if(chdir("..")==0){
      // printf("%s\n", getcwd(s, 100));
      sprintf(response, "200 Directory successfully changed to parent directory");
    }else{
      sprintf(response, "550 Failed to change directory.");
    }
  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute CDUP command", response);
  }

}

void cmd_noop(char arg[], int sd, int n){
  char response[255];
  sprintf(response, "200 Status OK");
  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute NOOP command", response);
  }
}

void cmd_pwd(char command[], char arg[], int sd, int n)
{
  char response[255];
  char cwd[1024];
  getcwd(cwd,1024);

  sprintf(response, "257 Directory Changed to %s", cwd);

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute PWD command", response);
  }
}


// mkd <pathname>
/*
This command causes the directory specified in the pathname
            to be created as a directory (if the pathname is absolute)
            or as a subdirectory of the current working directory (if
            the pathname is relative).*/
void cmd_mkd(char command[], char arg[], int sd, int n)
{
  char response[255];
  char cwd[1024];
  getcwd(cwd,1024);

  struct stat st = {0};

  if (stat(arg, &st) == -1) {
      if(mkdir(arg, 0700) == 0){
          sprintf(response, "257 Directory Created: %s", arg);
      }
      else {
        sprintf(response, "550 Failed to Create Directory at: %s", arg);
      }
  }



  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute MKD command", response);
  }
}


//rmd <pathname>
            /*
This command causes the directory specified in the pathname
            to be removed as a directory (if the pathname is absolute)
            or as a subdirectory of the current working directory (if
            the pathname is relative)
*/
void cmd_rmd(char command[], char arg[], int sd, int n)
{
  char response[255];
  char cwd[1024];
  getcwd(cwd,1024);

  if(rmdir(arg) == 0){
      sprintf(response, "250 Directory Removed: %s", arg);
  }
  else {
      sprintf(response, "550 Failed to Remove Directory at: %s", arg);
  }

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute RMD command", response);
  }

}



void cmd_retr(char command[], char arg[], int sd, int n, int fd, char **fifoname)
{

  char response[255];
  int fd2;

  if(strlen(arg) < 0){
    sprintf(response, "No Filename Provided");
  }
  else if( (fd2 = open(arg, O_RDONLY)) < 0)
  {
    sprintf(response, "File incorrect or does not exist");
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
          fprintf(stderr, "Server: MyFifo Name %s. File Size: %ld\n", myFifo, fileSize);

          lseek(fd2, 0, SEEK_SET);
          
          sprintf(response, "200 Beginning File Transfer\n");
          puts(response);

          fd = open(myFifo, O_WRONLY);

          fprintf(stderr, "Server: File Descriptor %d", fd2); // When set

          if(fd < 0){
              fprintf(stderr, "Server: ERROR in Opening Fifo FD"); // When set
          }

          int convFileSize = (int)fileSize;

          write(fd, &fileSize, sizeof(off_t)); // Send fileSize in FiFO first x byes. Client reads that then runs loop for that amount of bytes
          fprintf(stderr, "Server: Sent File Size %d", convFileSize);

          while(fileSize > 0){
                    
            // fprintf(stderr, "fileSize %ld", fileSize);
            if( (nx=read(fd2, buf, 1)) > 0)
            {
              // fileSize = lseek(fd2, 1, SEEK_CUR);
              fileSize = fileSize - nx;
              if(write(fd, buf, nx) != 1)
              {
                  fprintf(stderr, "Server: Problem in Writing");
                  break;
              }

              if(nx==-1)
              {
                  fprintf(stderr, "Server: Problem in Reading");
                  break;
              }
            }
          }

  


          close(fd);
          *fifoname=NULL;
          *fifoname=realloc(*fifoname, 255);
          unlink(myFifo);
          sprintf(response, "226 File send OK");

    }
  }

  close(fd2);

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute RMD command", response);
  }
}


void cmd_stor(char command[], char arg[], int sd, int n, int fd, char **fifoname)
{
  char response[255];
  char ch;
  pid_t pid;
  int fd2;
  char buf[1];

  if(strlen(arg) < 0){
    sprintf(response, "No Filename Provided");
  }

  if(**fifoname == '\0'){
    fprintf(stderr, "Server: No Connection"); // When not set
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
      fprintf(stderr, "File incorrect or does not exist\n");
    }

    chmod(arg, 0777);

    int nx;
    int fileSize=0;
    int totalBytesRead=0;

    read(fd, &fileSize, sizeof(int));
    int offset = lseek(fd, 4, SEEK_SET);
    fprintf(stderr, "Server: File Size %d", fileSize);
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


    sprintf(response, "226 File Received OK\n");

    close(fd);
    close(fd2);
    // *fifoname=NULL;
    // *fifoname=realloc(*fifoname, 255);
    memset(*fifoname, 0, 255);


  }

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute STOR command", response);
  }
}



void cmd_appe(char command[], char arg[], int sd, int n, int fd, char **fifoname)
{
  char response[255];
  char ch;
  pid_t pid;
  int fd2;
  char buf[1];

  if(strlen(arg) < 0){
    sprintf(response, "No Filename Provided");
  }

  if(**fifoname == '\0'){
    fprintf(stderr, "Server: No Connection"); // When not set
  } 
  else {

    int attemptMode=0;
    int fileSize=0;

    if( (fd2 = open(arg, O_CREAT | O_EXCL | O_WRONLY, 0777)) < 0)
    {
      fprintf(stderr, "File exists. Attempting Appending Mode\n");
      if( (fd2 = open(arg, O_APPEND | O_WRONLY, 0777)) < 0)
      {
        fprintf(stderr, "Failed to open file in Append Mode");
        perror("ERROR");
      }
      else attemptMode = 1;
    }
    
    else if( (fd2 = open(arg, O_CREAT | O_WRONLY | O_TRUNC, 0777)) < 0)
    {
      fprintf(stderr, "File incorrect or does not exist\n");
    }

    
    fprintf(stderr, "Server: File Size %d. Attempt Mode %d", fileSize, attemptMode);

    fd = open(*fifoname, O_RDONLY);


    if(fd < 0){
        perror("ERROR"); // When set
    }

    chmod(arg, 0777);

    int nx;
    int totalBytesRead=0;

    read(fd, &fileSize, sizeof(int));
    int offset = lseek(fd, 4, SEEK_SET);


    if(attemptMode == 1) fileSize=fileSize*2;

    while(fileSize > 0)
    {
      if(read(fd, &ch, 1) > 0)
        {
          totalBytesRead++;
          if(totalBytesRead > sizeof(int)){
            fileSize = fileSize - 1;
            write(fd2, &ch, 1);
          }
        }
    }


    sprintf(response, "226 File Received OK\n");

    close(fd);
    close(fd2);

    memset(*fifoname, 0, 255);
  }

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute APPE command", response);
  }
}

void cmd_rnfr(char command[], char arg[], int sd, int n, char **fileName){
  // Open a new pipe
    char tmpFifoName[255];
    char response[255];

  // unlink(arg);

  // if(strlen(arg)>0&&arg[0]!='-'){
  //   
  // }
  // fprintf(stderr, "1tmpFifoName %s\n", tmpFifoName);
  if( strlen(arg)>0 ){
      sprintf(*fileName, "%s", arg);
      sprintf(response, "450 Command Okay. Now use RNTO");
      fprintf(stderr, "File Name Selected: %s \n", *fileName);
  }
  else {
    sprintf(response, "Command Error. No file selected to rename");
  }
  

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute RNFR command", response);
  }

}
void cmd_rnto(char command[], char arg[], int sd, int n, char **fileName){

  char response[255];
  int status;

  fprintf(stderr, "%s", *fileName);

  if(strlen(arg)>0){
      status = rename(*fileName, arg);
      if(status < 0){
          sprintf(response, "532 Error Renaming\n");
      }
      else sprintf(response, "250 Command Okay\n");
  }
  else {
    sprintf(response, "Command Error. No file selected to rename");
  }
  

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute RNFR command", response);
  }


}


void cmd_dele(char command[], char arg[], int sd, int n){
    char response[255];
  int status;

  if(strlen(arg)>0){
      status = remove(arg);
      if(status < 0){
          sprintf(response, "450 Error Deletion");
      }
      else sprintf(response, "250 Command Okay. File Deleted");
  }
  else {
    sprintf(response, "Command Error. No file selected to delete");
  }
  

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute DELE command", response);
  }

}


void cmd_abor(char command[], char arg[], int sd, int n, int fd, char **fifoname){

  char response[255];
  int status;

  if(**fifoname == '\0'){
    sprintf(response, "350 No FTP in progress"); // When not set
  }
  else
     {
                  unlink(*fifoname);
                memset(*fifoname, 0, 255);
                sprintf(response, "225 Command Okay. Aborted");

    }




  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute ABOR command", response);
  }


}


void cmd_quit(char command[], char arg[], int sd, int n){

  char response[255];
  int status;
  
  sprintf(response, "221 Service closing control connection. Logged out if appropriate.\n");

  if((write(sd, response, strlen(response)+1)) < 0){
        fprintf(stderr,"Failed to execute ABOR command", response);
  }

  sleep(1);
   kill(getpid(), SIGTERM);

  // exit(0);


}