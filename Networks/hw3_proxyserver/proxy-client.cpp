/* 
 * File:   proxy-client.cpp
 * Author: deep
 *
 * Created on October 17, 2014, 1:42 PM
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <ctype.h>

using namespace std;

//encode url for '/'

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

//get in address

void *get_in_addr(struct sockaddr *sa)
{
 if (sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }
 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int sockfd, rv, n, m, nbytes;
    struct addrinfo hints, *serverinfo, *p;
    char s[INET6_ADDRSTRLEN];
    string url = argv[3], host, suburl = "/";
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (argc < 4) {
        cout<<"./client proxy_ip proxy_port url\n";
        return 2;
    }
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    cout<<url<<endl;
    std::size_t found = url.find("http://");
    if (found!=std::string::npos) {
        url = url.substr(url.find("http://")+7);
    }
    found = url.find("/");
    if (found!=std::string::npos) {
        host = url.substr(0, found);
        suburl = url.substr(found,url.length());
    }
    else {
        host = url;
    }
    
    string get = "GET ";
    string http = " HTTP/1.0\r\n";
    string hostc = "Host: ";
    string conn = "\r\nConnection: close";
    string end = "\r\n\r\n";
    string q = get + suburl + http + hostc + host  + end;
    
    cout<<"string q:"<<q<<endl;
   
    
    for(p = serverinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(serverinfo);
    
    if(send(sockfd, q.c_str(), q.length(), 0) == -1) {
        perror("send");
    }
    
    char newbuf[4096];
    nbytes = 0;
    
    char url_encoded[1024];
    char urlnew[1024];
    memcpy(urlnew,suburl.c_str(),suburl.size());
    url_encode(urlnew, url_encoded);
    string filename = "./clientdir/"+host+url_encoded;
    
    cout<<"filename: "<<filename<<endl;
    struct stat st = {0};
    if(stat("./clientdir/",&st) == -1) {
        mkdir("./clientdir/", 0700);
    }
    int cfd;
    if((cfd = open(filename.c_str(), O_RDWR|O_TRUNC|O_CREAT, S_IRWXU)) < 0) {
	perror("failed to open client file");
        close(sockfd);
        return (EXIT_FAILURE);
    }
    int c = -1;
    do{
        bzero((char*)newbuf, 4096);
        
        nbytes = recv(sockfd, newbuf, 4096, 0);
        
        //remove header
        
        if (c == -1) {
            char *start = strstr(newbuf, "\r\n\r\n");
            c = 0;
            write(cfd,start+4,nbytes - (start + 4 - newbuf));
            continue;
        }
        write(cfd,newbuf,nbytes);
    }while(nbytes>0);
      
    return 0;
}

