 /* File:   tftpserver.cpp
 * Author: deep
 *
 * Created on October 1, 2014, 10:39 PM
 */

#include <stdio.h>		
#include <string.h>		
#include <stdlib.h> 		
#include <unistd.h> 		
#include <sys/types.h>		
#include <sys/socket.h>		
#include <netdb.h>		
#include <netinet/in.h>		
#include <arpa/inet.h> 		
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <iostream>

using namespace std;

#define RRQ 1
#define DATA 3
#define TIMEOUT 1 //amount of time to wait for an ACK/Data Packet in 100microseconds
#define RETRIES 50 // Number of times to resend a data OR ack packet beforing giving up 
#define MAXACKFREQ 1 // Maximum number of packets before ack 
#define MAXDATASIZE 512 // Maximum data size allowed 
#define GetCurrentDir getcwd // Get Current Directory 
static char buf[512];	//a buffer for the messages passed between the client and the server 

void *get_in_addr(struct sockaddr *sa)
{
 if(sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }
 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sendit(char *, struct sockaddr_in, char *, int, struct addrinfo *);

char path[512] = "/tmp/";
unsigned short int ackfreq = 1;
int datasize = 512;
 
int main(int argc, char** argv) {
    int listenerfd, i, j, yes=1, rv, pid, sock, n, client_len, status, opt, tid;
    char opcode, *bufindex, filename[196], mode[12], remoteIP[INET6_ADDRSTRLEN];
    struct sockaddr_storage remote_addr;
    struct sockaddr_in client;
    socklen_t addrlen;
    struct addrinfo hints;
    struct addrinfo *internet, *pointinternet;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if(argc < 3) {
        cout<<"./server server_ip server_port \n";
        return 2;
    }
    if((rv = getaddrinfo(argv[1], argv[2], &hints, &internet)) != 0) {
        fprintf(stderr, "Server: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(pointinternet= internet; pointinternet!= NULL; pointinternet= pointinternet->ai_next) {
        listenerfd = socket(pointinternet->ai_family, pointinternet->ai_socktype, pointinternet->ai_protocol);
        if(listenerfd < 0) { 
            continue;
        }
        setsockopt(listenerfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(bind(listenerfd, pointinternet->ai_addr, pointinternet->ai_addrlen) < 0) {
            close(listenerfd);
            continue;
        }
        break;
    }
    if(pointinternet == NULL) {
        fprintf(stderr, "Server: failed to bind\n");
        exit(2);
    }
    freeaddrinfo(internet);
    
    pid = fork();

    if(pid != 0)		// if pid != 0 then we are the parent 
    {
	if(pid == -1)
	{
	    cout<<"Error: Fork failed!"<<endl;
	    return 0;
	}
	else
	{
	    cout<<"Daemon Successfully forked(pid: "<<pid<<")"<<endl;
	    return 1;
	}
    }
    GetCurrentDir(path, sizeof(path));
    //cout<<"Current Dir: "<<path<<endl;
    while(1) {
        client_len = sizeof(client);
        memset(buf, 0, 512);
        n = 0;
        errno = EAGAIN; // there is no data available right now, try again later
        while(errno == EAGAIN || n == 0) {
            waitpid(-1, &status, WNOHANG); 
            n = recvfrom(listenerfd, buf, BUFSIZ, 0, (struct sockaddr *) &client, (socklen_t *) & client_len);
            if(n < 0 && errno != EAGAIN)
	    {
                cout<<"The server could not receive from the client\n";
                    return 0;
	    }
            if(n > 0) {
                //cout<<"n from client : "<<n<<endl;
                cout<<"Got connection\n";
            }
            usleep(1000);
	}
        if(n > 0) {
            bufindex = buf;
            bufindex++;
            tid = ntohs(client.sin_port);	// record the tid 
            opcode = *bufindex++;	//opcode is in the second byte.
            if(opcode == 1) {
                //cout<<"inside if\n";
                strncpy(filename, bufindex, sizeof(filename) - 1);	
                bufindex += strlen(filename) + 1;	// move the pointer to after the filename + null byte in the string 
                strncpy(mode, bufindex, sizeof(mode) - 1);	
                bufindex += strlen(mode) + 1;	// move pointer 
                printf("opcode: %x filename: %s packet size: %d mode: %s\n", opcode, filename, n, mode);
            }
        
        
            pid = fork();

            if(pid == 0) {		// if we are pid == 0 then we are a child 
                //cout<<"inside child"<<endl;
                //char str[12];
                //int num = 20000 + (random() % (21000 - 20000 + 1));
                //sprintf(str, "%d", num);
                struct addrinfo *internet2;
                if((rv = getaddrinfo(argv[1], "0" , &hints, &internet2)) != 0) {
                    fprintf(stderr, "Server: %s\n", gai_strerror(rv));
                    exit(1);
                }
                sendit(filename, client, mode, tid, internet2);
                exit(1);
            }
        }
        
    }
    
    return 0;
}

void sendit(char *pFilename, struct sockaddr_in client, char *pMode, int tid, struct addrinfo *internet2) {
    //cout<<"inside sendit"<<endl;
    int bytessent, sock, len, client_len, opcode, ssize = 0, n, i, j, blockcount = 0, yes=1;
    unsigned short int count = 0, rcount = 0, acked = 0;
    unsigned char filebuf[MAXDATASIZE + 1];
    unsigned char packet[MAXACKFREQ][MAXDATASIZE + 12], recvbuf[MAXDATASIZE + 12];
    char filename[128], mode[12], fullpath[196], *bufindex;
    struct sockaddr_in ack;

    FILE *fp;			// pointer to the file we will be sending 

    strcpy(filename, pFilename);	//copy the pointer to the filename into a real array
    strcpy(mode, pMode);		//same as above
    struct addrinfo *pointinternet2;
    for(pointinternet2= internet2; pointinternet2!= NULL; pointinternet2= pointinternet2->ai_next) {
        sock = socket(pointinternet2->ai_family, pointinternet2->ai_socktype, pointinternet2->ai_protocol);
        if(sock < 0) { 
            continue;
        }
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(bind(sock, pointinternet2->ai_addr, pointinternet2->ai_addrlen) < 0) {
            close(sock);
            continue;
        }
        break;
    }
    if(pointinternet2 == NULL) {
        fprintf(stderr, "Server: failed to bind in child\n");
        exit(2);
    }
    
    //cout<<"New Socket = "<<sock<<endl;
    strcpy(fullpath, path);
    strncat(fullpath, "/", sizeof(fullpath) - 1);
    strncat(fullpath, filename, sizeof(fullpath) - 1);
    //cout<<"Fullpath: "<<fullpath<<endl;
    fp = fopen(fullpath, "r");
    memset(filebuf, 0, sizeof(filebuf));
    while(1) {
        //cout<<"In while in sendit"<<endl;
        acked = 0;
        ssize = fread(filebuf, 1, datasize, fp);
        count++;			// count number of datasize byte portions we read from the file 
        if(count == 1)		// we always look for an ack on the FIRST packet 
            blockcount = 0;
        else if(count == 2)	// The second packet will always start our counter at zero.  
            blockcount = 0;
        else
            blockcount =(count - 2) % ackfreq;
        //cout<<"blockcount : "<<blockcount<<endl;
        sprintf((char *) packet[blockcount], "%c%c%c%c", 0x00, 0x03, 0x00, 0x00);	// build data packet but write out the count as zero 
        memcpy((char *) packet[blockcount] + 4, filebuf, ssize);
        //cout<<ssize<<endl;
        len = 4 + ssize;
        packet[blockcount][2] =(count & 0xFF00) >> 8;	//fill in the count(top number first)
        packet[blockcount][3] =(count & 0x00FF);	//fill in the lower part of the count
        n = sendto(sock, packet[blockcount], len, 0,(struct sockaddr *) &client, sizeof(client));
        //cout<<"n in sendto to rrq : "<<n<<endl;
           // cout<<"error in first sendto\n";
        //else 
          //  cout<<"sent first\n";
        
        //n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *) &ack,(socklen_t *) & client_len);
        
        //cout<<"trying n : "<<n<<endl;
        if((count - 1) == 0 ||((count - 1) % ackfreq) == 0 || ssize != datasize) {
            // The following 'for' loop is used to recieve/timeout ACKs 
            for(j = 0; j < RETRIES; j++) {
                //cout<<"Try "<<j<<endl;
                client_len = sizeof(ack);
                errno = EAGAIN;
                n = -1;
                for(i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++) {
                    n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *) &ack,(socklen_t *) & client_len);
                    //cout<<"n = "<<n<<endl;
                    usleep(100); // for ack
                }
                if(n > 0) {
                    //cout<<"Inside n > 0 if clause\n";
                    if(client.sin_addr.s_addr != ack.sin_addr.s_addr) {	// checks to ensure send to ip is same from ACK IP 
                        j--;
                        continue;
                    }
                    if(tid != ntohs(client.sin_port)) {
                        j--;
                        continue;
                    }
                    bufindex =(char *) recvbuf;	//start our pointer going
                    if(bufindex++[0] != 0x00)
                        cout<<"bad first nullbyte!\n";
                    opcode = *bufindex++;

                    rcount = *bufindex++ << 8;
                    rcount &= 0xff00;
                    rcount +=(*bufindex++ & 0x00ff);
                    if(opcode == 4 || rcount == count) {
                        break;
                    }
                }	    
            }
        }
        else {
            n = recvfrom(sock, recvbuf, sizeof(recvbuf), MSG_DONTWAIT,(struct sockaddr *) &ack,(socklen_t *) & client_len);
        }
        if(j == RETRIES)
	{
            fclose(fp);
            close(sock);
            cout<<"Done sending"<<endl;
            return;
	}
        if(ssize != datasize)
            break;
        memset(filebuf, 0, sizeof(filebuf));
    }
    fclose(fp);
    close(sock);
    cout<<"Done sending"<<endl;
    return;
}