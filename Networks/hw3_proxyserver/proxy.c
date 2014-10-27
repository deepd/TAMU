/* 
 * File:   proxy-server.c
 * Author: deep
 *
 * Created on October 25, 2014, 11:20 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
/*
 * 
 */

int create_server_socket(char *ip, char *port) {
    int listenerfd, rv, yes;
    struct addrinfo hints, *internet, *pointinternet;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(ip, port, &hints, &internet)) != 0) {
        fprintf(stderr, "Server: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(pointinternet= internet; pointinternet!= NULL; pointinternet= pointinternet->ai_next) {
        listenerfd = socket(pointinternet->ai_family, pointinternet->ai_socktype, pointinternet->ai_protocol);
        if (listenerfd < 0) { 
            continue;
        }
        setsockopt(listenerfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listenerfd, pointinternet->ai_addr, pointinternet->ai_addrlen) < 0) {
            close(listenerfd);
            continue;
        }
        break;
    }
    if (pointinternet == NULL) {
        fprintf(stderr, "Server: failed to bind\n");
        exit(2);
    }
    freeaddrinfo(internet);
    if (listen(listenerfd, 10) == -1) {
        perror("listen");
        exit(3);
    }
    return listenerfd;
}

void print_usage() {
    printf("Usage: ./proxyserver <IP> <PORT>\n");
}

//url encode for '/'

void url_encode(char *url, char *url_encode) {
    char rfc[256] = {0};
    int i = 0;
    for (; i < 256; i++) {
        rfc[i] = isalnum(i)||i == '~'||i == '-'||i == '.'||i == '_' 
            ? i : 0;
    }   
    
    char *tb = rfc;
    
    for (; *url; url++) {
        if (tb[*url]) sprintf(url_encode, "%c", tb[*url]);
        else        sprintf(url_encode, "%%%02X", *url);
        while (*++url_encode);
    }   
}

void child_process(int newsockfd) {
    char query[4096] = {0}, buffer[4096] = {0};
    int nbytes = recv(newsockfd,query,4096, 0);
    if(nbytes > 0) {
        printf("*Query: %s\n",query);
        char *h, *http, *rn, *s;
        char host[4096]={0}, suburl[4096]={0}, url_encoded[4096]={0};

        int n;

        s = strstr(query, "/");
        http = strstr(query, "HTTP/1.0");
        h = strstr(query, "Host:");
        rn = strstr(query, "\r\n\r\n");
        n = http - s - 1;
        for(int i = 0; i < n; i++) {
            suburl[i] = *(s+i);
        }
        n = rn - h - 6;
        for(int i = 0; i < n; i++) {
            host[i] = *(h+i+6);
        }
        
        printf("*proxy client receive data from %s%s\n",host,suburl);
        printf("*\t->Host: %s*\n", host);
        printf("*\t->Suburl: %s*\n", suburl);
        url_encode(suburl, url_encoded);
        
        //find ip for host
        struct hostent *hostent;
	if((hostent = gethostbyname(host)) == NULL)
	{
            fprintf(stderr, "failed to resolve %s: %s\n", host, strerror(errno));
        }
        
        // create socket
	int rsockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(rsockfd < 0)
	{
            perror("failed to create remote socket");
        }
        int portn = 80;
        struct sockaddr_in host_addr;
        if((hostent = gethostbyname(host)) == NULL)
	{
            fprintf(stderr, "failed to resolve %s: %s\n", host, strerror(errno));
            close(rsockfd);
            return;
        }
	bzero((char*)&host_addr, sizeof(host_addr));
	host_addr.sin_port = htons(portn);
	host_addr.sin_family = AF_INET;
	bcopy((char*)hostent->h_addr, (char*)&host_addr.sin_addr.s_addr, hostent->h_length);

	// try connecting to the remote host
	if(connect(rsockfd, (struct sockaddr*)&host_addr, sizeof(struct sockaddr)) < 0)
	{
            perror("failed to connect to remote server");
        }
        
        printf("*\t-> connected to host: %s w/ ip: %s\n", host, inet_ntoa(host_addr.sin_addr));
        
        char file[1024]; //filepath for cached file
        sprintf(file, "./cache/%s%s", host, url_encoded);
        
        //send conditional get to server if cached file
        if(access(file, F_OK) != -1) {
            // GET CACHE DATA
            int cfd = open (file, O_RDWR);
            bzero((char*)buffer, 4096);
            // reading the first chunk is enough
            read(cfd, buffer, 4096);
            close(cfd);

            // find the first occurunce of "Date:" in response -- NULL if none.
            // ex. Date: Fri, 18 Apr 2014 02:57:20 GMT
            char* dateptr = strstr(buffer, "Date:");
            // response has a Date field, like Date: Fri, 18 Apr 2014 02:57:20 GMT

            char datetime[256];	// the date-time when we last cached a url
            bzero((char*)datetime, 256);
            // skip 6 characters, namely "Date: "
            // and copy 29 characters, like "Fri, 18 Apr 2014 02:57:20 GMT"
            strncpy(datetime, &dateptr[6], 29);

            // send CONDITIONAL GET
            // If-Modified-Since the date that we cached it
            
            sprintf(query, "GET %s HTTP/1.0\r\nHost: %s\r\nIf-Modified-Since: %s\r\n\r\n", suburl, host, datetime);
            
            printf("*\t-> conditional GET...\n");
            printf("*\t-> If-Modified-Since: %s\n", datetime);
        }
        
        n = send(rsockfd, query, strlen(query), 0);
        if (n < 0) {
            perror("failed to write to remote socket");
            close(rsockfd);
            return;
        }
        
        int cfd = -1; // cache file descriptor

	// since the response can be huge, we might need to iterate to
	// read all of it
	do
	{
            bzero((char*)buffer, 4096);

            // recieve from remote host
            n = recv(rsockfd, buffer, 4096, 0);
            // if we have read anything - otherwise END-OF-FILE
            if(n > 0)
            {
		// if this is the first time we are here
		// meaning: we are reading the http response header
		if(cfd == -1)
		{
                    float ver;
                    // read the first line to discover the response code
                    // ex. HTTP/1.0 200 OK
                    // ex. HTTP/1.0 304 Not Modified
                    // we only care about these two!
                    
                    int response_code;
                    sscanf(buffer, "HTTP/%f %d", &ver, &response_code);

                    printf("*\t-> response_code: %d\n", response_code);

                    // if it is not 304 -- anything other than sub-CASE32
                    if(response_code != 304)
                    {
			// create the cache-file to save the content
			sprintf(file, "./cache/%s%s", host, url_encoded);
			if((cfd = open(file, O_RDWR|O_TRUNC|O_CREAT, S_IRWXU)) < 0)
			{
				perror("failed to create cache file");
				close(rsockfd);
                                return;
			}

			printf("*\t-> from remote host...\n");

                        } else {
                            // if it is 304 -- sub-CASE32
                            // our content is already up-to-date
                            printf("***\t-> not modified\n");
                            printf("***\t-> from local cache...\n");

                            // send the response to client from local cache
                            goto FROM_CACHE;
			}
                }

		// save to cache
		write(cfd, buffer, n);
            }
	} while(n > 0);
	close(cfd);
        
        FROM_CACHE:

	// read from cache file
	sprintf(file, "./cache/%s%s", host, url_encoded);
	if((cfd = open(file, O_RDONLY)) < 0)
	{
            perror("failed to open cache file");
            close(rsockfd);
            return;
	}
	do
	{
            bzero((char*)buffer, 4096);
            n = read(cfd, buffer, 4096);
            if(n > 0)
            {
                // send it to the client
                send(newsockfd, buffer, n, 0);
            }
	} while(n > 0);
	close(cfd);
        close(rsockfd);
    }
    
    
}


void main_loop(int sockfd) {
    struct sockaddr_in cli_addr;
    bzero((char*)&cli_addr, sizeof(cli_addr));
    unsigned int clilen = sizeof(cli_addr);

    while(1)
    {
	int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

	pid_t chpid = fork();
	if(chpid == 0) // if child process
	{
            child_process(newsockfd);

            printf("*\t-> child process exiting...\n");
            close(newsockfd);
            close(sockfd);
            exit(0);
	}

        // parent process
	close(newsockfd);
    }
    
}

int main(int argc, char** argv) {
    if(argc != 3){
        print_usage();
        return (EXIT_FAILURE);
    }
    char* ip = argv[1];
    char *port = argv[2];
    printf("proxy server (%s:%s) starting..\n", ip, port);
    
    //check cache directory exists
    
    struct stat st = {0};
    if(stat("./cache/",&st) == -1) {
        mkdir("./cache/", 0700);
    }
    
    //create socket 
    int socket = create_server_socket(ip, port);
    main_loop(socket);
    
    close(socket);
    
    return (EXIT_SUCCESS);
}

