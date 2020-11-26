#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <errno.h>  
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#define MAX 80 
#define PORT 8084 
#define SA struct sockaddr 
#define MAX_CLIENT 30
char currentUser[100] = "$";
char broadPortStr[4];
int broadcastPermission = 1;
struct sockaddr_in broadcasttwoaddr, broadcasttwoaddrSend;
int broadSock;
struct sockaddr_in servaddr;
int serverSock;
  
void clientFunc(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    while(1) { 
        bzero(buff, MAX);  
        read(sockfd, buff, sizeof(buff)); 
        printf("From Client 1 : %s\t Enter Your Message -> ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n');
        write(sockfd, buff, sizeof(buff)); 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Client 2 Exit\n"); 
            break; 
        } 
    } 
} 

void serverFunc(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    while(1) { 
        bzero(buff, sizeof(buff)); 
        printf("Enter Your Message -> "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n'); 
        write(sockfd, buff, sizeof(buff)); 
        bzero(buff, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff)); 
        printf("\t From Client 2 : %s", buff); 
        if ((strncmp(buff, "exit", 4)) == 0) { 
            printf("Client 1 Exit\n"); 
            break; 
        } 
    } 
} 

int makeItClient(int po) 
{
    printf("privateChat on Port %d\n", po);
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
    // po = 8087;
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(po); 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
    clientFunc(sockfd); 
    close(sockfd); 
    return 0;
}

void makeItServer(int po) 
{ 
    printf("privateChat on Port %d\n", po);
    int sockfd, connfd;
    socklen_t len; 
    struct sockaddr_in servaddr, cli; 
    //int port = 8087;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(po); 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }
    else
        printf("Socket successfully binded..\n"); 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("listening..\n"); 
    len = sizeof(cli); 
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("acccepted...\n"); 
    serverFunc(connfd); 
    close(sockfd); 
}

void heartbeat(int sig){
    int s = sendto(serverSock, broadPortStr, 4, 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (s != 4)
    {
        write(STDOUT_FILENO,"couldn't send !\n", sizeof("couldn't send !\n")-1);
        exit(1);
    }
    alarm(5);
	signal(SIGALRM,heartbeat);
	return;
}

void recvhandler(int sig){
    int readfromylen;
    socklen_t len = sizeof(broadcasttwoaddr);
    char readfromy[1025];
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(broadSock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    if((readfromylen = recvfrom(broadSock, readfromy, 1025, 0, (struct sockaddr *) &broadcasttwoaddr, &len)) < 0) {
    }
    else {
        write(STDOUT_FILENO, readfromy, readfromylen);
    }
    alarm(1);
    signal(SIGALRM,recvhandler);
	return;
}

void createBroadcastSock(int porty){                  
    if ((broadSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		write(STDOUT_FILENO, "socket() failed !\n", sizeof("socket() failed !\n") - 1);
		exit(1);	
	}	

    if(setsockopt(broadSock, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0){
		write(STDOUT_FILENO, "error in setsockopt()1 !\n", sizeof("error in setsockopt()1 !\n") - 1);
		exit(1);
	}

    if(setsockopt(broadSock, SOL_SOCKET, SO_REUSEPORT , &broadcastPermission, sizeof(broadcastPermission)) < 0){
		write(STDOUT_FILENO, "error in setsockopt()2 !\n", sizeof("error in setsockopt()2 !\n") - 1);
		exit(1);
	}

    memset(&broadcasttwoaddr, 0, sizeof(broadcasttwoaddr));   
    broadcasttwoaddr.sin_family = AF_INET;                
    broadcasttwoaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    broadcasttwoaddr.sin_port = htons(porty);

    if(bind(broadSock, (struct sockaddr*)&broadcasttwoaddr, sizeof(broadcasttwoaddr)) < 0){
        write(STDOUT_FILENO, "error in binding !\n", sizeof("error in binding !\n") - 1);
	 exit(1);
	}
    write(STDOUT_FILENO, "Binded to port y for broadcast !\n", sizeof("Binded to port y for broadcast !\n") - 1);

	return;
}


int main(int argc, char const *argv[]) 
{ 
    if(argc < 2){
		write(1, "ERROR...more inputs are needed!\n", 32);
		exit(1);
	}

	if(argc > 2){
		write(1, "ERROR...too much input, only 2 inputs are needed!\n", 50);
		exit(1);
	}

    int activity, FD, newfd, n, cliSock,
    newClinetSock[MAX_CLIENT] = {0}, max_fd; 
	struct sockaddr_in cli; 
    fd_set fileDescriptors;

    int portX = atoi(argv[1]);

	serverSock = socket(AF_INET, SOCK_STREAM, 0); 
	if (serverSock == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 50000;
    int p = 1;
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(atoi(argv[1])); 

    newClinetSock[0] = STDIN_FILENO;	
	newClinetSock[1] = serverSock;
    newClinetSock[2] = cliSock;

    if (connect(serverSock, (SA*)&servaddr, sizeof(servaddr)) < 0) { 
        printf("connection with the server failed :(\n"); 
        exit(0); 
    }

    cliSock = socket(AF_INET, SOCK_STREAM, 0);
    cli.sin_family = AF_INET; 
    cli.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    cli.sin_port = htons(portX); 

    
    max_fd = serverSock;
    while(1){
        FD_ZERO(&fileDescriptors);
        for(int i = 0; i < MAX_CLIENT; i++){
            if(newClinetSock[i] >= 0) {
                FD_SET(newClinetSock[i],&fileDescriptors);
            }
            if(newClinetSock[i] > max_fd) {
                max_fd = newClinetSock[i];
            }
	    }
	    activity = select(FD_SETSIZE, &fileDescriptors, NULL, NULL, &t);

        if(FD_ISSET(newClinetSock[0], &fileDescriptors)){
            char buff[MAX]; 
            int n = 0; 
            bzero(buff, sizeof(buff)); 
            if((strncmp(currentUser, "$", 1)) == 0) {
                buff[0] = '#';  //so the server will know this is user's username
                n++;
            }
            while ((buff[n++] = getchar()) != '\n') {}
            if((strncmp(currentUser, "$", 1)) == 0) {
                for(int i = 1; i < strlen(buff); i++) {
                    currentUser[i - 1] = buff[i];
                }
            }
            if((strncmp(buff, "privateChat", 11)) == 0) {
                buff[0] = '_';  //So the server will know this is a private chat
            }
            if((strncmp(buff, "leave", 5)) == 0) {
                alarm(0);
                bzero(buff, sizeof(buff));
                char message[80] = "You Left This Group\n";
                for(int j = 0; j < strlen(message); j++) {
                    buff[j] = message[j];
                }
            }
            if(strncmp(buff, "send", 4) == 0) {
                memset(&broadcasttwoaddrSend, 0, sizeof(broadcasttwoaddrSend));   
                broadcasttwoaddrSend.sin_family = AF_INET; 
                broadcasttwoaddrSend.sin_addr.s_addr = inet_addr("192.168.1.255"); 
                broadcasttwoaddrSend.sin_port = htons(atoi(broadPortStr)); 
                memmove(buff, buff+5, strlen(buff));
                int s = sendto(broadSock, buff, strlen(buff), 0, (struct sockaddr *) &broadcasttwoaddrSend, sizeof(broadcasttwoaddrSend));
                if (s != strlen(buff))
                {
                    write(STDOUT_FILENO,"couldn't send !\n", sizeof("couldn't send !\n")-1);
                    exit(1);
                }
                bzero(buff, sizeof(buff)); 
            }
            if ((strncmp(buff, "exit", 4)) == 0) { 
                printf("Client Exit...\n"); 
                // alarm(0);
                break; 
            } 
            send(serverSock, buff, 80, 0);
        }
        else if((FD_ISSET(newClinetSock[1], &fileDescriptors)) && (newClinetSock[1] != -1)) {
            char buff[1025];
            bzero(buff, sizeof(buff));
            n = recv(newClinetSock[1], buff, 1025, 0);
            if(strncmp(buff, "_", 1) == 0) {
                char newPort[4];
                for(int i = 0; i< 4; i++) {
                    newPort[i] = buff[i+1];
                }
                memmove(buff, buff+5, strlen(buff));
                if(strncmp(buff, currentUser, strlen(currentUser)) == 0) {
                    makeItClient(atoi(newPort));
                    bzero(buff, sizeof(buff));
                }
            }
            if(strncmp(buff, "&", 1) == 0) {
                memmove(buff, buff+1, strlen(buff));
                for(int k = 0; k < 4; k++) {
                    broadPortStr[k] = buff[k];
                }
                bzero(buff, sizeof(buff));
                createBroadcastSock(atoi(broadPortStr));
                signal(SIGALRM, recvhandler);
                alarm(1);
                // signal(SIGALRM, heartbeat);
                // alarm(5);
            }
            if(strncmp(buff, "ONLINEEE", 8) == 0) {
                memmove(buff, buff+8, strlen(buff));
                makeItServer(atoi(buff));
                bzero(buff, sizeof(buff));
            }
            if(strncmp(buff, "OFFLINEE", 8) == 0) {
                bzero(buff, sizeof(buff));
                char message2[80] = "The User Is Offline!\n";
                for(int j = 0; j < strlen(message2); j++) {
                    buff[j] = message2[j];
                } 
            }
            if(strncmp(buff, "NOGROUP", 7) == 0) {
                bzero(buff, sizeof(buff));
                char message2[80] = "Group Doesn't Exist!\n";
                for(int j = 0; j < strlen(message2); j++) {
                    buff[j] = message2[j];
                } 
            }
            printf("%s", buff);
        }
        else if((FD_ISSET(newClinetSock[2], &fileDescriptors)) && (newClinetSock[2] != -1)) {
            int addrlen = sizeof(servaddr);  
            newfd = accept(newClinetSock[2],(struct sockaddr *)&cli, (socklen_t*)&cli);

			for (int i = 0; i < MAX_CLIENT; i++)   
            {   
                if(newClinetSock[i] == 0 )   
                {   
                    newClinetSock[i] = newfd;   
                    printf("adding to list of sockets !\n");  
                    break;   
                } 
                else
                    continue;  
            }
			if(newfd > max_fd)
		        max_fd = newfd;
        }
    }
    // printf("FOUNDIT!!!!\n");
	close(serverSock); 
} 