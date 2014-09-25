/* A simple TCP client */ 
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>

using namespace std;

#define BUFLEN 512

//message header structure

struct msg_hdr{
    unsigned short int ver : 9;
    unsigned short int type : 7;
    unsigned short int length;
    void clear() {
        ver = 0;
        type = 0;
        length = 0;
    }
    
};

//message sbcp attribute structure

struct msg{
    unsigned short int type;
    unsigned short int length;
    char payload[4];
    void clear() {
        type = 0;
        length = 0;
        for (int i = 0; i < 4; i++) {
            payload[i] = ' ';
        }
    }
    
};

//whole message structure

struct sbcp{
    struct msg_hdr header;
    struct msg msgs[128];
    void clear() {
        header.clear();
        for(int i = 0; i < 128; i++) 
            msgs[i].clear();
    }
};

//get in address

void *get_in_addr(struct sockaddr *sa)
{
 if (sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }
 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv) {
    int maxfd,sockfd, rv, n, m, nbytes;
    fd_set master;
    FD_ZERO(&master);
    struct addrinfo hints, *serverinfo, *p;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (argc < 4) {
        cout<<"./client username server_ip server_port\n";
        return 2;
    }
    if ((rv = getaddrinfo(argv[2], argv[3], &hints, &serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
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
    
    FD_SET(sockfd, &master);
    FD_SET(0, &master);
    maxfd = sockfd;
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(serverinfo);
    //send join
    n = strlen(argv[1])/4;
    m = strlen(argv[1]);
    if (strlen(argv[1])%4 != 0) {
        n++;
    }
    //send join
    
    struct sbcp buf;
    buf.header.ver = 3;
    buf.header.type = 2;
    buf.header.length = 4;
    for(int i = 0; i < n; i++) {
        if (m < 4) {
            buf.msgs[i].length = m + 4;
        }
        else {
            buf.msgs[i].length = 8;
        }
        buf.header.length+=buf.msgs[i].length;
        buf.msgs[i].type = 2;
        for (int j = 0; j < 4; j++) {
            buf.msgs[i].payload[j] = argv[1][i*4 + j];
        }
        m = m-4;
    }
    if (send(sockfd, &buf, sizeof(buf), 0) == -1) {
        perror("send");
    }
   
    //main for loop
    
    for(;;) {
        if (select(maxfd+1,&master,NULL,NULL,NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        //get input from keyboard
        
        if(FD_ISSET(0,&master)) {
            char message[BUFLEN]={};
            nbytes = read(0, message, sizeof(message));
            //cout<<message;
            struct sbcp *newbuf = new sbcp();
            newbuf->clear();
            n = strlen(message)/4;
            m = strlen(message);
            if (strlen(message)%4 != 0) {
                n++;
            }
            newbuf->header.ver = 3;
            newbuf->header.type = 4;
            newbuf->header.length = 4;
            for(int i = 0; i < n; i++) {
                if (m < 4) {
                    newbuf->msgs[i].length = m + 4;
                }
                else {
                    newbuf->msgs[i].length = 8;
                }
                newbuf->header.length+=newbuf->msgs[i].length;
                newbuf->msgs[i].type = 4;
                for (int j = 0; j < 4; j++) {
                    newbuf->msgs[i].payload[j] = message[i*4 + j];
                }
                m = m-4;
            }
            if (send(sockfd, newbuf, sizeof(struct sbcp), 0) == -1) {
                perror("send");
            }
            newbuf->clear();
            delete newbuf;
        }
        
        //get data from server
        
        if (FD_ISSET(sockfd,&master)) {
            //cout << "something has come from server"<<endl;
            //struct sbcp buf;
            
            struct sbcp *newbuf = new sbcp();
            newbuf->clear();
            if ((nbytes = recv(sockfd, newbuf, sizeof(struct sbcp), 0)) <= 0){
                if (nbytes == 0) {
                    cout<<"server down"<<endl;
                    return 2;
                }
                else
                    perror("recv from server");
            }
            else {
                if(newbuf->header.type == 7) {
                    //Handle ACK;
                    cout<<endl;
                    cout<<"Client Count: ";
                    cout<<newbuf->msgs[0].payload<<endl;
                    cout<<"Connected Clients: ";
                    n = ((newbuf->header.length) - 4)/8;
                    if (((newbuf->header.length) - 4)%8 != 0) 
                        n++;
                    for (int i = 1; i < n; i++) {
                        cout<<newbuf->msgs[i].payload;
                    }
                    cout<<endl;
                    newbuf->clear();
                    delete newbuf;
                }
                if(newbuf->header.type == 5) {
                    //Handle NACK;
                    cout<<endl;
                    n = ((newbuf->header.length) - 4)/8;
                    if (((newbuf->header.length) - 4)%8 != 0) 
                        n++;
                    for (int i = 0; i < n; i++) {
                        cout<<newbuf->msgs[i].payload;
                    }
                    cout<<endl;
                    newbuf->clear();
                    delete newbuf;
                    return 2;
                }
                if(newbuf->header.type == 6) {
                    //Handle OFFLINE;
                    n = ((newbuf->header.length) - 4)/8;
                    if (((newbuf->header.length) - 4)%8 != 0) 
                        n++;
                    for (int i = 0; i < n; i++) {
                        cout<<newbuf->msgs[i].payload;
                    }
                    cout<<" went offline"<<endl;
                    newbuf->clear();
                    delete newbuf;
                }
                if(newbuf->header.type == 3) {
                    //Handle FWD
                    int n = 0;
                    for(int i = 0; newbuf->msgs[i].type != 2; i++)
                        n++;
                    for(int i = n; i < n+4; i++){
                        if (newbuf->msgs[i].type == 2)
                            cout<<newbuf->msgs[i].payload;
                    }
                    cout<<" : ";
                    for(int i = 0; i < n; i++) {
                        cout<<newbuf->msgs[i].payload;
                    }
                    cout<<endl;
                    newbuf->clear();
                    delete newbuf;
                }
                if(newbuf->header.type == 8) {
                    //Handle ONLINE
                    n = ((newbuf->header.length) - 4)/8;
                    if (((newbuf->header.length) - 4)%8 != 0) 
                        n++;
                    for (int i = 0; i < n; i++) {
                        cout<<newbuf->msgs[i].payload;
                    }
                    cout<<" came online"<<endl;
                    newbuf->clear();
                    delete newbuf;
                }
            }
        }
        //set it again
        
        FD_ZERO(&master);
        FD_SET(sockfd, &master);
        FD_SET(0, &master);        
    }
    return(0); 
}