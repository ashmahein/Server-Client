// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int server_sock, r;
int SERVER_IP, SERVER_PORT;


// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n");
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n",
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n;
  char line[MAX], ans[MAX];

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);

    //tokenize the line so that we can use dirname to make the directory.
    char temp[MAX], *dirname, *cmd, *strTemp, cwd[215], temp2[MAX];
    const char *strArr[64];
    int i = 0;
    strcpy(temp, line);


    //grab the command from the line.
    strTemp = strtok(temp, " ");
    cmd = strTemp;

    //grab the dirname and store it in a seperate variable.
    while(strTemp != NULL)
    {
        strTemp = strtok(NULL, " ");
        strArr[i++] = strTemp;
    }


    if((strcmp(cmd, "lmkdir") == 0) || (strcmp(cmd, "lrmdir") == 0) || (strcmp(cmd, "lls") == 0) ||
    (strcmp(cmd, "lrm") == 0) || (strcmp(cmd, "lcd") == 0) || (strcmp(cmd, "lpwd") == 0) || (strcmp(cmd, "lcat") == 0))
    {
        dirname = strArr[0];

        if(strcmp(cmd, "lmkdir") == 0)
        {
            int ret = mkdir(dirname, 0755);

            if(ret == 0)
            {
                puts("lmkdir OK");
            }
            else
            {
                puts("lmkdir FAILED");
            }
        }
        else if(strcmp(cmd, "lrmdir") == 0)
        {
            int ret = rmdir(dirname);

            if(ret == 0)
            {
                puts("lrmdir OK");
            }
            else
            {
                puts("lrmdir FAILED");
            }
        }
        else if(strcmp(cmd, "lls") == 0)
        {
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
                    puts("lls FAILED");
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
        }
        else if(strcmp(cmd, "lcd") == 0)
        {
            if(dirname == NULL)
            {
                chdir(getenv("HOME"));
                puts("cd to HOME");
            }
            else
            {
                int ret = chdir(dirname);
            }
        }
        else if(strcmp(cmd, "lpwd") == 0)
        {
            if(getcwd(cwd, sizeof(cwd)) != NULL)
            {
                fprintf(stdout, cwd);
                putc('\n', stdout);
            }
            else
            {
                puts("lpwd FAILED");
            }
        }
        else if(strcmp(cmd, "lrm") == 0)
        {
            int ret = unlink(dirname);

            if(ret == 0)
            {
                puts("lrm OK");
            }
            else
            {
                puts("lrm FAILED");
            }
        }
        else if(strcmp(cmd, "lcat") == 0)
        {
            if(dirname != NULL)
            {
                char str;
                FILE *infile = fopen(dirname, "r");

                if(infile)
                {
                    while((str = fgetc(infile)) != EOF)
                    {
                        putc(str, stdout);
                    }
                }
                fclose(infile);
            }
        }

    }
    else
    {
        // Send ENTIRE line to server
        n = write(server_sock, line, MAX);
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        if(strcmp(cmd, "get") == 0)
        {
            dirname = strArr[0];
            char buf[MAX], size[MAX];
	    int fileSize;
            //write(server_sock, line, MAX);

	    read(server_sock, size, MAX);
	    sscanf(size, "%d", &fileSize);

	    //int fileSize = atoi(size);

            int count = 0;

            int fd = open(dirname, O_WRONLY | O_CREAT, 0664);
            if (fd < 0)
            {
                perror("Client receiving error");
            }
            while(count < fileSize)
            {
                n = read(server_sock, buf, MAX);
                count += n;
                int ret = write(fd, buf, n);
		if(ret < 0)
		{puts("write error");
		}

            }
            close(fd);
        }
        else if(strcmp(cmd, "ls") == 0)
        {
            while(n)
            {
                n = read(server_sock, ans, MAX);

                puts(ans);
            }
        }
        else if(strcmp(cmd, "put") == 0)
        {
            dirname = strArr[0];
            char size[MAX], buf[MAX];

            struct stat sb;
            if(stat(dirname, &sb) != 0)
            {
                write(server_sock, "BAD", MAX);
                continue;
            }
            //sprintf(size, "%d", sb.st_size);
            write(server_sock, size, MAX);
            int fd = open(dirname, O_RDONLY);
            if(fd < 0)
            {
                perror("server sending error 1\n");
                return 1;
            }
            while(n = read(fd, buf, MAX))
            {
                write(server_sock, buf, n);
            }
            close(fd);
        }
        else
        {
            // Read a line from sock and show it
            n = read(server_sock, ans, MAX);
            printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
        }
    }
  }
}
