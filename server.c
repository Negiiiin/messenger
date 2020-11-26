#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <errno.h>  
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/socket.h>  
#include <sys/time.h> 
#define MAX 80 
#define MAX_CLIENTS 30
#define SA struct sockaddr 
int lastGroup = 0;
char listOfGroups[1000][100] = {};
char workingGroups[1000][100] = {};
char ports[1000][100] = {};
struct sockaddr_in;
int bdsock;
int broadcastPermission = 1;
char onlineUsers[MAX_CLIENTS][50];
int lastUser = 0;
int client_socket[MAX_CLIENTS];
struct sockaddr_in address;
int lastWorkingGroup = 0;
int addToThisNumer;

void recvhandler(int sig){
    int readfromlen;
    socklen_t len = sizeof(address);
    char readfrom[MAX];
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    free(workingGroups);
    lastWorkingGroup = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_socket[i];
        setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
        if((readfromlen = recvfrom(sd, readfrom, MAX, 0, (struct sockaddr *) &address, &len)) < 0) {
        }
        else {
            for(int i = 0; i < 4; i++) {
                workingGroups[lastWorkingGroup][i] = readfrom[i];
            }
            lastGroup++;
        }
    }
    for(int i = 0; i < lastGroup; i++) {
        int flag = 0;
        for(int j = 0; j < lastWorkingGroup; j++) {
            if((strncmp(listOfGroups[i], workingGroups[j], 4)) == 0) {
                flag = 1;
                break;
            }
        }
        if(flag == 0) {
            strcpy(listOfGroups[i], "");
        }
    }
    alarm(5);
    signal(SIGALRM,recvhandler);
	return;
}

int func(int sockfd, struct sockaddr_in address) 
{
	char buff[80]; 
	int n; 
    int addrlen = sizeof(address);
	int valread = read(sockfd, buff, sizeof(buff)); 
    //Somebody disconnected
    if(valread == 0)
    {
        getpeername(sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        printf("Host disconnected , ip %s , port %d \n" ,  
        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
        close( sockfd );
        return 1;
    }
    if(strncmp(buff, "# ", 1) == 0) {
        getpeername(sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        printf("Entered Username , ip %s , port %d \n" ,  
        inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 

        for(int i = 1; i < strlen(buff); i++) {
            onlineUsers[addToThisNumer][i - 1] = buff[i];
        }
        lastUser ++;
        bzero(buff, MAX);
        char message[80] = "Welcome\n";
        printf("Welcome Message Sent , ip %s , port %d \n" ,  
        inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
        for(int j = 0; j < strlen(message); j++) {
            buff[j] = message[j];
        }
    }
    if ((strncmp(buff, "makeGroup", 9)) == 0) {
        getpeername(sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   

        memmove(buff, buff+10, strlen(buff));
        char addThis[100];
        char newPort[4];
        while(1) {
            for(int i = 0; i< 4; i++) {
                newPort[i] = (((rand() % 9)+1) + '0');
            }
            int flag = 0;
            for(int i = 0; i < lastGroup; i++) {
                if(strncmp(newPort, ports[i], 4) == 0) {
                    flag = 1;
                    break;
                }
            }
            if(flag == 0) {
                strcpy(ports[lastGroup], newPort);
                break;
            }
        }
        printf("Broadcast Port %s\n", newPort);
        printf("Group Created, ip %s , port %d \n" ,  
        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        for(int i = 0; i < 4; i++) {
            addThis[i] = newPort[i];
        }
        for(int i = 0; i < strlen(buff); i++) {
            addThis[i + 4] = buff[i];
        }
        strcpy(listOfGroups[lastGroup], addThis);
        bzero(buff, MAX);
        char message[20] = "Group Created!\n";
        for(int j = 0; j < strlen(message); j++) {
            buff[j] = message[j];
        } 
        lastGroup ++;
    }
    if ((strncmp(buff, "joinGroup", 9)) == 0) {
        getpeername(sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen); 

        memmove(buff, buff+10, strlen(buff));
        int foundIt = 0;
        int isEqual = 1;
        for(int i = 0; i < lastGroup; i++) {
            for(int j = 4; j < strlen(buff) + 3; j++) {
                if((strlen(listOfGroups[i]) - 4) != strlen(buff)) {
                    isEqual = 0;
                    break;
                }
                if(buff[j - 4] != listOfGroups[i][j]) {
                    isEqual = 0;
                    break;
                }
            }
            if(isEqual) {
                foundIt = 1;
                bzero(buff, MAX);
                buff[0] = '&';
                for(int j = 1; j < 5; j++) {
                    buff[j] = listOfGroups[i][j - 1];
                }
                break;
            }
        }
        if(foundIt) {
            char newPort[4];
            for(int i = 0; i < 4; i++) {
                newPort[i] = buff[i + 1];
            }
            printf("with Broadcast Port %s\n", buff);
            printf("Joined Group , ip %s , port %d \n" ,  
            inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
            // signal(SIGALRM, recvhandler);
            // alarm(30);
        }
        else {
            printf("didn't joinGroup, It didn't exist ... \n");
            char message2[80] = "NOGROUP";
            bzero(buff, MAX); 
            for(int j = 0; j < strlen(message2); j++) {
                buff[j] = message2[j];
            } 
        }
    }
    if ((strncmp(buff, "you Left This Group\n", 20)) == 0) { 
        // alarm(0);
    }
    if ((strncmp(buff, "listOfGroups", 12)) == 0) { 
        getpeername(sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        printf("Asked For List Of Groups , ip %s , port %d \n" ,  
        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        bzero(buff, MAX); 
        n = 0; 
        for(int i = 0; i < lastGroup; i++) {
            for(int j = 0; j < strlen(listOfGroups[i]) - 4; j++) {
                buff[n++] = listOfGroups[i][j + 4];
            } 
        }
	}

    if((strncmp(buff, "_rivateChat", 11)) == 0) {
        memmove(buff, buff+12, strlen(buff));
        int foundIt = 0;
        char newPort[4];
        for(int i = 0; i < lastUser; i++) {
            int isEqual = 1;
            for(int j = 0; j < strlen(buff); j++) {
                if(strlen(onlineUsers[i]) != (strlen(buff))) {
                    isEqual = 0;
                    break;
                }
                if(buff[j] != onlineUsers[i][j]) {
                    isEqual = 0;
                    break;
                }
            }
            if(isEqual) {
                foundIt = 1;
                bzero(buff, MAX);
                buff[0] = '_';
                for(int i = 0; i< 4; i++) {
                    char r = (((rand() % 9)+1) + '0');
                    buff[i+1] = r;
                    newPort[i] = r;
                }
                for(int j = 0; j < strlen(onlineUsers[i]); j++) {
                    buff[j + 5] = onlineUsers[i][j];
                }
                break;
            }
        }
        if(foundIt) {
            printf("chat started ... %s\n", buff);
            char message2[80] = "ONLINEEE";
            int sd;
            for (int i = 0; i < MAX_CLIENTS; i++){
                sd = client_socket[i];
                send(sd, buff, 80, 0);
            }
            bzero(buff, MAX); 
            for(int j = 0; j < strlen(message2); j++) {
                buff[j] = message2[j];
            } 
            for(int j = 0; j < 4; j++) {
                buff[j+8] = newPort[j];
            }
            // printf("::::  %s\n", buff);
        }
        else {
            printf("chat didn't started because user was offline ... \n");
            char message2[80] = "OFFLINEE";
            bzero(buff, MAX); 
            for(int j = 0; j < strlen(message2); j++) {
                buff[j] = message2[j];
            } 
        }  
    }
    send(sockfd, buff, 80, 0);
    return 0;
} 

int main(int argc, char const *argv[]) 
{
    int opt = 1;
    int master_socket, addrlen, new_socket, activity,i , valread, sd;   
    int max_sd; 
    fd_set readfds; 
    char buffer[1025];
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < MAX_CLIENTS; i++)   
    {   
        client_socket[i] = 0;   
    }
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }
    //for multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = inet_addr("127.0.0.1");   
    address.sin_port = htons(atoi(argv[1]));  
    //bind
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", atoi(argv[1]));   
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    } 
    addrlen = sizeof(address);   
    puts("Waiting for clients to connect ...");   
    while(1)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);    
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
        
        for ( i = 0 ; i < MAX_CLIENTS ; i++)   
        {
            sd = client_socket[i];   
            if(sd > 0)   
                FD_SET( sd , &readfds);   
            if(sd > max_sd)   
                max_sd = sd;   
        }   
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }
        if (FD_ISSET(master_socket, &readfds))   
        {
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }  
            printf("New connection -> socket fd : %d , ip : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
            for (i = 0; i < MAX_CLIENTS; i++)   
            {   
                if( client_socket[i] == 0 )   
                {   
                    addToThisNumer = i;
                    client_socket[i] = new_socket;   
                    printf("Adding to the list of sockets as number %d\n" , i);  
                    break;   
                }   
            }   
        }
        for (int i = 0; i < MAX_CLIENTS; i++){
            sd = client_socket[i];
            if(FD_ISSET(sd,&readfds)){
                if(func(sd, address) == 1) {
                    client_socket[i] = 0;
                    strcpy(onlineUsers[i], "");
                }
                continue;
            }
        } 
    } 
} 
