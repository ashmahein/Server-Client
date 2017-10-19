// This is the echo SERVER server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <grp.h>
#include <pwd.h>



#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   //if (hp == 0){
   //   printf("unknown host\n");
   //   exit(1);
   //}
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients)
   printf("5 : server is listening ....\n");
   listen(mysock, 5);
   printf("===================== init done =======================\n");
}


main(int argc, char *argv[])
{
   char *hostname;
   char line[MAX];
   char cmd[MAX], arg[MAX], cwd[MAX], buf[MAX], size[MAX];
   int fd, fileSize;

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];

   server_init(hostname);

   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n");

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
     if (client_sock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     // Processing loop: newsock <----> client
     while(1){
       bzero(line, MAX); bzero(cmd, MAX); bzero(arg, MAX);
       n = read(client_sock, line, MAX);
       if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;

      }
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);
      // parse input, ignoring whitespace
      sscanf(line, "%s %s", cmd, arg);

      if (!strcmp(cmd, "pwd"))
      {
            if(getcwd(cwd, sizeof(cwd)) != NULL)
            {
                write(client_sock, cwd, MAX);
                putc('\n', stdout);
            }
            else
            {
                write(client_sock, "FAILED", MAX);
            }
      }
      // ls
      else if (!strcmp(cmd, "ls"))
      {
            char *pointer=NULL;
            DIR *dp=NULL;
            char result[MAX];
            struct dirent *sd=NULL;
            pointer = getenv("PWD");
            dp=opendir((const char*)pointer);
            while((sd=readdir(dp))!=NULL)
            {
                strcpy(result, sd->d_name);
                write(client_sock, result, MAX);

                printf("%s  \t ",sd->d_name);
            }
            printf("\n");
            /*
            DIR *d;
            struct dirent *e;
            struct stat fileStat;
            struct passwd *p = getpwuid(fileStat.st_uid);
            struct group  *g = getgrgid(fileStat.st_gid);

            d = opendir(".");
            while(e = readdir(d))
            {
                if(stat(e->d_name, &fileStat) < 0)
                {
                    puts("ls FAILED");
                }
                else
                {

                    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

                    printf("\t%d ", fileStat.st_nlink);

                    p = getpwuid(fileStat.st_uid);
                    g = getgrgid(fileStat.st_gid);
                    printf("\t%s %s", p->pw_name, g->gr_name);

                    printf("\t%d ", fileStat.st_size);


                    char date[20];
                    strftime(date, 20, "%d-%m", localtime(&(fileStat.st_mtime)));
                    printf("\t%s ", date);

                    printf("\t%s\n", e->d_name);

                }

            }
            putc('\n', stdout);
            */
      }
      // cd
      else if (!strcmp(cmd, "cd"))
      {
            if(arg == NULL)
            {
                chdir(getenv("HOME"));
                puts("cd to HOME");
            }
            else
            {
                int ret = chdir(arg);
                puts("successful");
            }
      }
      // mkdir
      else if (!strcmp(cmd, "mkdir"))
      {
            int ret = mkdir(arg, 0755);

            if(ret == 0)
            {
                write(client_sock, "mkdir OK", MAX);
            }
            else
            {
                write(client_sock, "mkdir FAILED", MAX);
            }
      }
      // rmdir
      else if (!strcmp(cmd, "rmdir"))
      {
            int ret = rmdir(arg);

            if(ret == 0)
            {
                write(client_sock, "rmdir OK", MAX);
            }
            else
            {
                write(client_sock, "rmdir OK", MAX);
            }
      }
      // rm
      else if (!strcmp(cmd, "rm"))
      {
            int ret = unlink(arg);

            if(ret == 0)
            {
                write(client_sock, "rm OK", MAX);
            }
            else
            {
                write(client_sock, "rm failed", MAX);
            }
      }
      // get
      else if (!strcmp(cmd, "get"))
      {
            //stat filename to get file type AND SIZE
            //file type MUST be a REGular file
            struct stat sb;
            if(stat(arg, &sb) != 0)
            {
                write(client_sock, "BAD", MAX);
                continue;
            }
            sprintf(size, "%d", sb.st_size);
            write(client_sock, size, MAX);
            fd = open(arg, O_RDONLY);
            if(fd < 0)
            {
                perror("server sending error 1\n");
                return 1;
            }
            while(n = read(fd, buf, MAX))
            {
                write(client_sock, buf, n);
            }
            close(fd);
      }
      // put
      else if (!strcmp(cmd, "put"))
      {
            read(client_sock, size, MAX);
            sscanf(size, "%d", &fileSize);
            int count = 0;
            fd = open(arg, O_WRONLY | O_CREAT, 0664);
            if(fd < 0)
            {
                perror("get error 1\n");
                return 1;
            }
            while(count < fileSize)
            {
                n = read(client_sock, buf, MAX);
                count += n;
                write(fd, buf, n);
            }
            close(fd);
      }
      else
      {
            printf("Error: Command %s is not supported", cmd);
      }




      //strcat(line, " ECHO");

      // send the echo line to client
      //n = write(client_sock, line, MAX);

      //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      printf("server: ready for next request\n");
    }
 }
}
