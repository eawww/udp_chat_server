//
//  main.cpp
//  udp_chat_server
//
//  Created by Eric Wilson on 4/2/16.
//  Copyright © 2016 Eric Wilson. All rights reserved.
//

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

#define MYPORT "9111" //the port users will be connecting to
#define MAXBUFLEN 100

//prototypes
void *get_in_addr(struct sockaddr *sa);

int main(int argc, const char * argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    int yes =1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //set AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; //use my IP
    
    if ((rv = getaddrinfo(NULL,MYPORT,&hints,&servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }
    
    //loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p=p->ai_next){
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("UDP_Server: socket");
            continue;
        }
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("UDP_Server: bind");
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "UDP_Server: failed to bind socket\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);
    
    while(1){
        printf("\n");
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*)&their_addr,&addr_len)) == -1){
            perror("\trecvfrom");
            exit(1);
        }
        char buffer[INET_ADDRSTRLEN];
        printf("\tUDP_Server: got packet from &s\n",getnameinfo((struct sockaddr*)&their_addr,addr_len,buffer,sizeof(buffer),0,0,NI_NUMERICHOST));
        
        printf("\tUDP_Server: packet is %d bytes long\n", numbytes);
        
        buf[numbytes] = '\0';
        
        printf("\tUDP_Server: packet contains \"%s\"\n", buf);
        
        if ((numbytes = sendto(sockfd, "Server", 6, 0, (struct sockaddr *)&their_addr, addr_len )) == -1) {
            perror("\tUDP_Server: sendto error");
            exit(1);
        }
    }
    close(sockfd);
    return 0;
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}