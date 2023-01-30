#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#define MAXLENGTH 1000
#define DOMAIN AF_INET 
#define PORT 8080
#define FILE_PATH "/.config/mmblog.txt"

long get_file_size(FILE* fp)
{
   long size = 0;
   fpos_t pos;
   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   return size;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
	    puts("Not enough arguments");
	    return 1;
	}

	char *home = getenv("HOME");
	char file[1024];

	strcat(file, home);
	strcat(file, FILE_PATH);

	if (strcmp(argv[1], "add") == 0 || strcmp(argv[1], "-a") == 0) 
	{
	    if (argc < 2)
	    {
		puts("Not enough arguments for add");
		return 1;
	    }

	    if (access(file, F_OK) != 0)
	    {
		perror("access");
		exit(EXIT_FAILURE);
	    }

	    int i;
	    FILE *fp;
	    char final[10000];
	    time_t raw_time;

	    fp = fopen(file, "a");
	    if (fp == NULL)
	    {
		perror("fopen");
		exit(EXIT_FAILURE);
	    }

	    time(&raw_time);
	    strcat(final, asctime(localtime(&raw_time)));

	    for (i = 2; i < argc; i++){
		    strcat(final, argv[i]);
		    strcat(final, " ");
		    if (i == argc - 1)
			strcat(final, "\n\n");
	    }

	    fputs(final, fp);
	    fclose(fp);
	    exit(EXIT_SUCCESS);

	} 
	else if (strcmp(argv[1], "serve") == 0 || strcmp(argv[1], "-s") == 0)
	{
	    int sock, new_sock, address_length;
	    long file_size;
	    char hello[30000] = "HTTP/1.1 200 OK\nContent-Type: text\nConnection: Closed\r\n\r\n";
	    char res[30000];
	    char buff[30000];
	    char *file_contents;
	    FILE *fp;
	    struct sockaddr_in address;

	    sock = socket(DOMAIN, SOCK_STREAM, 0);
	    if (sock == 0)
	    {
		perror("socket");
		exit(EXIT_FAILURE);
	    }

	    address.sin_family = DOMAIN;
	    address.sin_port = htons(PORT);
	    address.sin_addr.s_addr = htonl(INADDR_ANY);
	    address_length = sizeof(address);

	    if ((bind(sock, (struct sockaddr *)&address, address_length)) < 0)
	    {
		perror("bind");
		exit(EXIT_FAILURE);
	    }

	    if (listen(sock, 10) < 0)
	    {
		perror("listen");
		exit(EXIT_FAILURE);
	    }

	    puts("Listening for connections...");
	    while(1)
	    {
		// check for errors
		fp = fopen(file, "r");
		if (fp == NULL)
		{
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		file_size = get_file_size(fp);
		file_contents = malloc(file_size);
		fread(file_contents, 1, file_size, fp);
		strcpy(res, hello);
		strcat(res, file_contents);
		free(file_contents);
		fclose(fp);

		new_sock = accept(sock, (struct sockaddr *)&address
			, (socklen_t *)&address_length);
		if (new_sock < 0)
		{
		    perror("accept");
			exit(EXIT_FAILURE);
		}
		if (read(new_sock, buff, 1024) < 0)
		{
		    perror("read");
			exit(EXIT_FAILURE);
		}
		if (write(new_sock, res, strlen(res)) < 0)
		{
		    perror("write");
			exit(EXIT_FAILURE);
		}
		if (close(new_sock) < 0)
		{
		    perror("close");
			exit(EXIT_FAILURE);
		}
	    }
	    exit(EXIT_SUCCESS);
	}
	else if (strcmp(argv[1], "create") == 0 || strcmp(argv[1], "-c") == 0)
	{
	    char name[1024];
	    char title[2048] = "'s Blog\n\n";
	    int c, i;
	    FILE *fp;

	    if (access(file, F_OK) == 0)
	    {
		puts("Filed already exists");
		exit(EXIT_FAILURE);
	    }

	    printf("Name: ");
	    while((c = getchar()) != EOF && c != '\n')
	    {
		if (i < 1024)
		    name[i++] = c;
		else 
		    puts("Maximum character amount reached");
	    }

	    strcat(name, title);
	    strcpy(title, name);

	    fp = fopen(file, "w");
	    if (fp == NULL)
	    {
		perror("fopen");
		exit(EXIT_FAILURE);
	    }
	    fputs(title, fp);
	    fclose(fp);
	    exit(EXIT_SUCCESS);
	}
	else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0)
	{
	    puts("Welcome to mmblog\nThese are the possible commands:");
	    puts("\ncreate\nThis command creates a new text file where your blog will be saved. The file \nwill be created according to the variable FILE_PATH, which is \n~/.config/mmblog.txt, if you want to change this, just edit main.c and \nrecompile. create will prompt you to write your user name and then the file \nwill be created. If the file already exists, the command does not overwrite it.");
	    puts("\nadd <new entry>\nThis command creates a new entry in your blog file. It will take all of the \nwords after add as the text to be inputted into the file.");
	    puts("\nserve\nThis command serves a web server in port 8080 that will host your blog. If \nyou want to change the port, change the PORT variable in main.c and recompile.");
	    exit(EXIT_SUCCESS);
	}

	puts("Not a valid argument, try mmblog help.");
	exit(EXIT_FAILURE);
}







