//
// Created by tal&almog on 03/05/2021.
//

#include<stdio.h>
#if defined _WIN32
#include<winsock2.h>   //winsock2 should be before windows
#pragma comment(lib,"ws2_32.lib")
#else
// Linux and other UNIXes
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <time.h>
#endif
#define SIZE 1024
#define SERVER_PORT 5060  //The port that the server listens

void measure_data(char* filename,int sockfd,int filesize) {
    int n;
    char buffer[SIZE];
    long total_bytes;
    FILE *fp;
    fp=fopen(filename,"w");
    unsigned long bytes_recv;
    struct timeval stop, start,total_start,total_finish;
    long millis;
    total_bytes=0;
    int single_filesize = filesize;
    gettimeofday(&total_start, NULL);
    for (int i = 0; i < 5; ++i) {
        printf("starting to receive file No. %d\n",i+1);
        gettimeofday(&start, NULL);
        for (int j = 0; j <filesize/SIZE ; ++j) {
            bytes_recv = recv(sockfd, buffer, SIZE, 0);
            if (bytes_recv < 0) {
                perror("[-]Cannot receive file");
                exit(1);}
            total_bytes += bytes_recv;
            fprintf(fp, "%s", buffer);
            fflush(fp);
            bzero(buffer, SIZE);
        }
        if(filesize%SIZE>0) {
            bytes_recv = recv(sockfd, buffer, filesize%SIZE, 0);
            if (bytes_recv < 0) {
                perror("[-]Cannot receive file");
                exit(1);}
            total_bytes += bytes_recv;
            fprintf(fp, "%s", buffer);
            fflush(fp);
            bzero(buffer, filesize%SIZE);}
        gettimeofday(&stop, NULL);
        millis = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
        printf("%dth file received time: %f\n", i+1,millis/(double)1000000);
    }
    gettimeofday(&total_finish, NULL);
    millis = (total_finish.tv_sec - total_start.tv_sec) * 1000000 + total_finish.tv_usec - total_start.tv_usec;
    printf("finished receiving files\n");
    printf("time took: %f. average time: %f\n\n",millis/(double)1000000,millis/(double)1000000/5);
    //printf("total bytes received : %lu\n\n",total_bytes);
    fclose(fp);
    return;
}

int main() {
    int listeningSocket = -1;
    long filesize;
    char* filename="cubic.txt";
    /* 
     ___________________________________________________________________________________
    |int socket(int af, int type, int protocol, 0)                                      |
    |af = protocol (IPv4 value: AF_INET)                                                |
    |type = type of the socket (TCP: SOCK_STREAM, UDP: SOCK_DGRAM)                      |
    |protocol = transport layer protocol (TCP: SOCK_STREAM, UDP: SOCK_DGRAM)            |
    |[protocol 0 = default (depending on the type of socket)]                           |
    |If no socket is established the function will return the value -1 (INVALID_SOCKET) |
    |___________________________________________________________________________________|
    */
    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create listening socket : %d", errno);}
    else {printf("Created socket\n");}
    int yes = 1;
    /*
     _______________________________________________________________________________________________________
    |avoid "Address already in use" error.                                                                  |
    |int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) |
    |_______________________________________________________________________________________________________|
    */
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);}
    /* 
     _______________________________________________________________
    |(IPv4 only! for IPv6- please see struct sockaddr_in6)          |
    |struct sockaddr_in {                                           |
    |    short int sin_family; // Address family, AF_INET unsigned  |
    |    short int sin_port; // Port number                         |
    |    struct in_addr sin_addr; // Internet address               |
    |    unsigned char sin_zero[8]; // Same size as struct sockaddr |
    |};                                                             |
    |_______________________________________________________________|
    */
    struct sockaddr_in serverAddress;
    /*
     ____________________________________________________________________________________________
    |void memset(void *str,int c, size_t n)                                                      |
    |Deletes n first characters of the string str which points to it is obtained as a parameter, |
    |and instead writes c.                                                                       |
    |____________________________________________________________________________________________|
    */
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // AF_INET (IPv4) or AF_INET6 (IPv6) 
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);  //network order (Byte Order)
    /*
     ________________________________________________________________________________
    |Link address and port with socket by bind function                              |
    |int bind(int sockfd, struct sockaddr *my_addr, int addrlen);                    |
    |sockfd = socket descriptor which is returned by the socket function             |
    |my_addr = pointer to a sockaddr structure that contains the address and port    |
    |          of a server to which you connect                                      |
    |namelen = size of sockaddr structure in homes.                                  |
    |The function returns -1 if there is an error. Otherwise, zero.                  |
    |________________________________________________________________________________|
    */
    if (bind(listeningSocket, (struct sockaddr*)&serverAddress , sizeof(serverAddress)) == -1) {
        printf("Bind failed with error code : %d",errno);
        return -1;
    } else {printf("Binding successful\n");}
    /*
     _______________________________________________________________________
    |goes into standby mode using listen function (this is TCP only)        |
    |sockfd = socket descriptor which is returned by the socket function    |
    |backlog = Maximum size of the queue of connection requests.            |
    |The function returns a value of -1 if there is an error. Otherwise, 0. |
    |_______________________________________________________________________|
    */
    if (listen(listeningSocket, 500) == -1) { // 500 is a Max size of queue connection requests
        //number of concurrent connections
        printf("listen() failed with error code : %d",errno);
        return -1;}
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    while (1) {
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        /*
         _______________________________________________________________________________________________
        |request to connect from the queue of requests using accept function (TCP only)                 |
        |accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)                                  |
        |The function returns a socket descriptor of a newly created socket,                            |
        |and inserts into the sockaddr structure information about the connecting client (IP and Port)  |
        |addrlen is a local integer variable which defines the size of a sockaddr structure.            |
        |The function returns a value of -1 if there is an error. Otherwise, 0.                         |
        |_______________________________________________________________________________________________|
        */
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1) {
            printf("listen failed with error code : %d",errno);
            // TODO: close the sockets
            return -1;}
        /*
         ____________________________________________________________________
        |int recv(int sockfd, void *buf, int len, int flags)                 |
        |sockfd - socket descriptor through which information is read        |
        |buf - pointer to the buffer which is called inside                  |
        |len - maximum size of the buffer in bytes                           |
        |flags - its value is 0.                                             |
        |The function returns the amount of bytes read inside the buffer.    |
        |If the function returns a value of -1 then there is an error.       |
        |If a function returns 0 it reflects that the connection is closed.  |
        |____________________________________________________________________|
        */
        recv(clientSocket, &filesize, sizeof(filesize), 0); // get single file size
        /* getting current CC (CONGESTION CONTROL) algorithm */
        char buf[256];
        socklen_t len;
        len = sizeof(buf);
        if (getsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) {
            perror("getsockopt");
            return -1;}
        printf("Starting to receive file over %s\n",buf); // Cubic
        /* Measure Data On Cubic Algorithm! */
        measure_data(filename,clientSocket,filesize);
        /* setting & getting reno CC (CONGESTION CONTROL) algorithm */
        if (setsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) != 0) {
            perror("setsockopt");
            return -1;}
        if (getsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) { // Reno
            perror("getsockopt");
            return -1;}
        /* get the switching time between the algorithms  */
        time_t current_time;
        struct tm * time_info;
        char timeString[9];  // space for "HH:MM:SS\0"
        time(&current_time);
        time_info = localtime(&current_time);
        strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
        /* end get the switching time */
        printf("Starting to receive file over %s (%s)\n",buf,timeString);
        filename="reno.txt";
        /* Measure Data On Reno Algorithm! */
        measure_data(filename,clientSocket,filesize);
        close(clientSocket);
    }
    close(listeningSocket);
    return 0;
}