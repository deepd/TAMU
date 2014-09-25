/* 
 * File:   server.cpp
 * Author: deep
 *
 * Created on September 16, 2014, 11:33 PM
 */

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
#include <map>

using namespace std;

/*
 * 
 */

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


int main(int argc, char** argv) {
    fd_set write_fds, read_fds;
    int maxfd, listenerfd, newfd, i, j, yes=1, rv, nbytes, n, N, m, flag, totalclients = 0, tempclients;
    char remoteIP[INET6_ADDRSTRLEN];
    char numberofclients[4] = {' ',' ',' ',' '};
    string join_ack_string = "";
    char joinusernameerror[] = "This username is already taken or it is bigger than 16 characters";
    char usersexceeded[] = "Server has reached its total users limit";
    multimap<int,string> usernames;
    multimap<int,string>::iterator it = usernames.begin();
    string s = "";  
    struct sockaddr_storage remote_addr;
    socklen_t addrlen;
    struct addrinfo hints;
    struct addrinfo *internet, *pointinternet;
    FD_ZERO(&write_fds);
    FD_ZERO(&read_fds);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (argc < 4) {
        cout<<"./server server_ip server_port maximum_clients\n";
        return 2;
    }
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &internet)) != 0) {
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
    FD_SET(listenerfd, &write_fds);
    //Keep Track of the Biggest FileDescriptor
    maxfd = listenerfd; 
    
    //Main For Loop
    
    for(;;) {        
        read_fds = write_fds;
        if (select(maxfd+1,&read_fds,NULL,NULL,NULL) == -1) {
            perror("select");
            exit(4);
        }
        for(i = 0; i <= maxfd; i++){
            if (FD_ISSET(i, &read_fds)) {
                
                // Handle new clients
                
                if (i == listenerfd && totalclients >= atoi(argv[3])) {
                    cout<<"Client not accepted, limit exceeded"<<endl;
                    addrlen = sizeof remote_addr;
                    newfd = accept(listenerfd, (struct sockaddr *)&remote_addr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    }
                    else {
                        n = strlen(usersexceeded)/4;
                        m = strlen(usersexceeded);
                        if (strlen(usersexceeded)%4 != 0) {
                            n++;
                        }
                        //send nack user exceeded
                        struct sbcp *newbuf = new sbcp();
                        newbuf->header.ver = 3;
                        newbuf->header.type = 5;
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
                                newbuf->msgs[i].payload[j] = usersexceeded[i*4 + j];
                            }
                            m = m-4;
                        }
                        if (send(newfd, newbuf, sizeof(struct sbcp), 0) == -1) {
                            perror("send in nack");
                        }
                        newbuf->clear();
                        delete newbuf;
                    }
                }
                else if (i == listenerfd && totalclients < atoi(argv[3])){
                    struct sbcp *firstbuf = new sbcp();
                    flag = 0;
                    addrlen = sizeof remote_addr;
                    newfd = accept(listenerfd, (struct sockaddr *)&remote_addr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    }
                    else {
                        if ((nbytes = recv(newfd, firstbuf, sizeof(struct sbcp), 0)) <= 0){
                            perror("recv in join 2");
                        }
                        else {
                            //Check join type and version
                            s = "";
                            if (firstbuf->header.ver == 3){
                                if (firstbuf->header.type == 2) {
                                    n = ((firstbuf->header.length) - 4)/8;
                                    if (((firstbuf->header.length) - 4)%8 != 0) 
                                        n++;
                                    for (int i = 0; i < n; i++) {
                                        s = s + firstbuf->msgs[i].payload;
                                    }
                                    
                                    tempclients = totalclients;
                                    
                                    printf("Server: New Connection From %s on Socket %d with username: ",inet_ntop(remote_addr.ss_family, 
                                                get_in_addr((struct sockaddr*)&remote_addr), remoteIP, INET6_ADDRSTRLEN), newfd);
                                    for (int i = 0; i < n; i++) {
                                        cout<<firstbuf->msgs[i].payload;
                                    }
                                    cout<<endl;
                                    
                                    //check username
                                    
                                    it = usernames.begin();
                                    for (int i =0; i < usernames.size(); ++it, ++i) {
                                        if (it->second == s) {
                                            flag = 1;
                                        }
                                    }
                                    
                                    if (n > 4)
                                        flag = 1;
                                    
                                    //flag == 0 means username allowed
                                    
                                    if (flag == 0) {
                                        cout<<"Accepted Client"<<endl;
                                        usernames.insert(pair<int,string> (newfd,s));
                                        //cout<<"username "<<usernames.find(newfd)->second<<" added\n";
                                        totalclients+=1;
                                        join_ack_string = "";
                                        FD_SET(newfd, &write_fds);
                                        if (newfd > maxfd) {
                                            maxfd = newfd;
                                        }
                                        
                                        // Get connected clients
                                        
                                        struct sbcp *newbuf = new sbcp();
                                        if (!usernames.empty()) {
                                            for (it = usernames.begin(); it != usernames.end(); ++it) {
                                                if ((*it).second != usernames.find(newfd)->second)
                                                    join_ack_string = join_ack_string + (*it).second + " ";
                                            }
                                        }
                                        cout << join_ack_string<< endl;
                                        
                                        int temp = join_ack_string.size();
                                        char join_ack[200] = {0};
                                        if (temp != 0) {
                                            for (int i = 0; i < temp; i++) {
                                                join_ack[i] = join_ack_string[i];
                                            }
                                        }
                                        
                                        //number of clients connected
                                        
                                        for (int i = 3 ; i >= 0 && tempclients != 0; i--) {
                                            numberofclients[i] = (tempclients%10) + '0';
                                            tempclients = tempclients/10;
                                        }
                                         
                                        newbuf->msgs[0].type = 3;
                                        newbuf->msgs[0].length = 8;
                                        for (int i = 0; i < 4; i++)
                                            newbuf->msgs[0].payload[i] = numberofclients[i];
                                        
                                        n = strlen(join_ack)/4;
                                        m = strlen(join_ack);
                                        if (strlen(join_ack)%4 != 0) {
                                            n++;
                                        }
                                        n++;
                                        newbuf->header.ver = 3;
                                        newbuf->header.type = 7;
                                        newbuf->header.length = 12;
                                        for(int i = 1; i < n; i++) {
                                            if (m < 4) {
                                                newbuf->msgs[i].length = m + 4;
                                            }
                                            else {
                                                newbuf->msgs[i].length = 8;
                                            }
                                            newbuf->header.length+=newbuf->msgs[i].length;
                                            newbuf->msgs[i].type = 2;
                                            for (int j = 0; j < 4; j++) {
                                                newbuf->msgs[i].payload[j] = join_ack[(i-1)*4 + j];
                                            }
                                            m = m-4;
                                        }
                                        
                                        if (send(newfd, newbuf, sizeof(struct sbcp), 0) == -1) {
                                            perror("send in ack");
                                        }
                                        newbuf->clear();
                                        delete newbuf;
                                        
                                        //send online to others
                                        
                                        struct sbcp *onlinebuf = new sbcp();
                            
                                        onlinebuf->header.type = 8;
                                        onlinebuf->header.ver = 3;
                                        onlinebuf->header.length = 4;
                                        string user = usernames.find(newfd)->second;
                                        temp = user.size();
                                        char user_online[16] = {};
                                        
                                        for (int i = 0; i < temp; i++) {
                                            user_online[i] = user[i];
                                        }
                                        
                                        n = strlen(user_online)/4;
                                        m = strlen(user_online);
                                        if (strlen(user_online)%4 != 0) {
                                            n++;
                                        }
                                        for(int i = 0; i < n; i++) {
                                            if (m < 4) {
                                                onlinebuf->msgs[i].length = m + 4;
                                            }
                                            else {
                                                onlinebuf->msgs[i].length = 8;
                                            }
                                            onlinebuf->header.length+=onlinebuf->msgs[i].length;
                                            onlinebuf->msgs[i].type = 4;
                                            for (int j = 0; j < 4; j++) {
                                                onlinebuf->msgs[i].payload[j] = user_online[i*4 + j];
                                            }
                                            m = m-4;
                                        }   
                                       
                                        
                                        for (j = 0; j <= maxfd; j++) {
                                            if (j != listenerfd && j != newfd && FD_ISSET(j,&write_fds)) {
                                                cout<<"sending online to : "<<j<<endl;
                                                if (send(j, onlinebuf, sizeof(struct sbcp), 0) == -1) {
                                                    perror("send in online");
                                                }
                                            }
                                        }
                                        onlinebuf->clear();
                                        delete onlinebuf;
                                    }
                                    else {
                                        // send nack for username
                                        
                                        cout<<"Client not accepted"<<endl;
                                        
                                        n = strlen(joinusernameerror)/4;
                                        m = strlen(joinusernameerror);
                                        if (strlen(joinusernameerror)%4 != 0) {
                                            n++;
                                        }
                                        struct sbcp *newbuf = new sbcp();
                                        newbuf->header.ver = 3;
                                        newbuf->header.type = 5;
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
                                                newbuf->msgs[i].payload[j] = joinusernameerror[i*4 + j];
                                            }
                                            m = m-4;
                                        }
                                        if (send(newfd, newbuf, sizeof(struct sbcp), 0) == -1) {
                                            perror("send in nack");
                                        }
                                        newbuf->clear();
                                        delete newbuf;
                                    }
                                }
                            }
                        }
                    }
                    firstbuf->clear();
                    delete firstbuf;
                }
                else {
                    // for clients sending SEND
                    
                    struct sbcp *newbuf = new sbcp();
                    newbuf->clear();
                    if ((nbytes = recv(i, newbuf, sizeof(struct sbcp), 0)) <= 0){
                        // Clients disconnecting
                        if (nbytes == 0) {
                            printf("Server: socket %d hung up\n", i);
                            totalclients--;
                            
                            // send offline
                            
                            newbuf->clear();
                            delete newbuf;
                            struct sbcp *newbuf = new sbcp();
                            
                                    newbuf->header.type = 6;
                                    newbuf->header.ver = 3;
                                    newbuf->header.length = 4;
                                    string user = usernames.find(i)->second;
                                   // cout<<"user: "<<user<<endl;
                                    int temp = user.size();
                                    char user_online[16] = {};
                                    for (int i = 0; i < temp; i++) {
                                        user_online[i] = user[i];
                                    }
                                    n = strlen(user_online)/4;
                                    m = strlen(user_online);
                                    if (strlen(user_online)%4 != 0) {
                                        n++;
                                    }
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
                                            newbuf->msgs[i].payload[j] = user_online[i*4 + j];
                                        }
                                        m = m-4;
                                    }   
                            for (j = 0; j <= maxfd; j++) {
                                if (j != listenerfd && j != i && FD_ISSET(j,&write_fds)) {
                                    cout<<"sending offline to : "<<j<<endl;
                                    if (send(j, newbuf, sizeof(struct sbcp), 0) == -1) {
                                        perror("send in offline");
                                    }
                                }
                            }
                            close(i);
                            FD_CLR(i,&write_fds);
                            usernames.erase(i); //removing the username
                            for (it = usernames.begin(); it != usernames.end(); it++) {
                                cout<<(*it).second;
                                cout<<endl;
                            }
                            newbuf->clear();
                            delete newbuf;
                        }
                        else {
                            perror("recv");
                        }
                    }
                    else {
                        // send part from client and then forward
                            if (FD_ISSET(i, &write_fds)){ 
                                if (newbuf->header.ver == 3) {
                                    if (newbuf->header.type == 4) {
                                        n = ((newbuf->header.length) - 4)/8;
                                        if (((newbuf->header.length) - 4)%8 != 0) 
                                            n++;
                                        cout<<"client "<<usernames.find(i)->second<<" : ";
                                        for (int i = 0; i < n; i++) {
                                            cout<<newbuf->msgs[i].payload;
                                        }
                                        cout<<endl;
                                    }
                                    else {
                                        cout<<"header type mismatch\n";
                                    }
                                }
                                else {
                                    cout<<"header version mismatch\n";
                                }
                                struct sbcp *fwdbuf = new sbcp();
                                fwdbuf->clear();
                                fwdbuf->header.type = 3;
                                N = ((newbuf->header.length) - 4)/8;
                                if (((newbuf->header.length) - 4)%8 != 0) 
                                    N++;
                                for (int i = 0; i < N; i++) {
                                    for (int j = 0; j < 4; j++) 
                                        fwdbuf->msgs[i].payload[j] = newbuf->msgs[i].payload[j];
                                    fwdbuf->msgs[i].length = newbuf->msgs[i].length;
                                    fwdbuf->msgs[i].type = newbuf->msgs[i].type;
                                }
                                fwdbuf->header.length = newbuf->header.length;
                                fwdbuf->header.ver = newbuf->header.ver;
                                
                                string user = usernames.find(i)->second;
                                int temp = user.size();
                                char user_online[16] = {};
                                for (int i = 0; i < temp; i++) {
                                    user_online[i] = user[i];
                                }
                                n = strlen(user_online)/4;
                                m = strlen(user_online);
                                if (strlen(user_online)%4 != 0) {
                                    n++;
                                }
                                for(int i = N; i < N + n; i++) {
                                    if (m < 4) {
                                        fwdbuf->msgs[i].length = m + 4;
                                    }
                                    else {
                                        fwdbuf->msgs[i].length = 8;
                                    }
                                    fwdbuf->header.length+=fwdbuf->msgs[i].length;
                                    fwdbuf->msgs[i].type = 2;
                                    for (int j = 0; j < 4; j++) {
                                        fwdbuf->msgs[i].payload[j] = user_online[(i-N)*4 + j];
                                    }
                                    m = m-4;
                                }
                                for (int i = N + n; i < 128; i++) {
                                    fwdbuf->msgs[i].length = 0;
                                    fwdbuf->msgs[i].type = 0;
                                    for (int j = 0; j < 4; j++)
                                        fwdbuf->msgs[i].payload[j] = '\0';
                                }
                                cout<<endl;
                                for (j = 0; j <= maxfd; j++) {
                                    if (j != listenerfd && j != i && FD_ISSET(j,&write_fds)) {
                                        cout<<"sending fwd to : "<<j<<endl;
                                        if (send(j, fwdbuf, sizeof(struct sbcp), 0) == -1) {
                                            perror("send in fwd");
                                        }
                                    }
                                }
                                fwdbuf->clear();
                                delete fwdbuf;
                            }
                            newbuf->clear();
                            delete newbuf;
                    }
                }
            }
        }    
    }
    
    return 0;

}
    

