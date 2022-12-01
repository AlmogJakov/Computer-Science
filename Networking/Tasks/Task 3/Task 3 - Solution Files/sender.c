//
// Created by tal&almog on 03/05/2021.
//
#include <stdio.h>
#if defined _WIN32
// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#endif
#define SIZE 1024
#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "192.168.190.129"

void send_file(char *filename, FILE *fp, int sockfd) {
    ssize_t n;
    char buffer[SIZE];
    struct stat filest;
    stat(filename, &filest);
    long filesize = filest.st_size;
    long file_left = filesize % SIZE;
    unsigned long total_bytes = 0;
    unsigned long bytes_sent = 0;
    unsigned long r_val = 0;
    int single_filesize = filesize;
    for (int i = 0; i < 5; ++i) {
        while(single_filesize>0) {
            r_val = fread(buffer, SIZE, 1, fp);
            if (r_val < 0) {
                perror("[-]Error in reading file");
                exit(1);}
            bytes_sent = send(sockfd, buffer, SIZE, 0);
            if (bytes_sent < 0) {
                perror("[-]Error in sending file");
                exit(1);}
            single_filesize-=bytes_sent;
            total_bytes += bytes_sent;
        }
        single_filesize = filesize;
        rewind(fp);
    }
    printf("Total bytes sent : %lu \n", total_bytes);
}

int Tcp_CC() {
    socklen_t len;
    FILE *fp;
    char *filename = "1mb.txt";
    /* return information about a file (file size in this case) */
    struct stat filest;
    stat(filename, &filest);
    long filesize = filest.st_size;
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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("couldent create socket:");
    }
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
    struct sockaddr_in sockadrr_out;
    /*
     ____________________________________________________________________________________________
    |void memset(void *str,int c, size_t n)                                                      |
    |Deletes n first characters of the string str which points to it is obtained as a parameter, |
    |and instead writes c.                                                                       |
    |____________________________________________________________________________________________|
    */
    memset(&sockadrr_out, 0, sizeof(sockadrr_out));
    sockadrr_out.sin_family = AF_INET;
    sockadrr_out.sin_port = htons(SERVER_PORT);
    /*
     _______________________________________________________________________________________
    |struct sockaddr_in sa; // IPv4                                                         |
    |struct sockaddr_in6 sa6; // IPv6                                                       |
    |inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // IPv4                            |
    |inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); // IPv6                |
    |The function converts the addresses to binary representation                           |
    |The function returns -1 if there is an error or 0 if the conversion was not successful.|
    |need to make sure that the value returned by the inet_pton function is greater than 0. |
    |_______________________________________________________________________________________|

    */
    int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &sockadrr_out.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;}
    /*
     _________________________________________________________________________________
    |int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);                |
    |sockfd = socket descriptor which is returned by the socket() function            |
    |serv_addr = pointer to the sockaddr structure that contains the address and port |
    |            of the server to which you are connecting                            |
    |addrlen - size of sockaddr structure in bytes.                                   |
    |if the connection is not established, the function returns a value of -1.        |
    |_________________________________________________________________________________|
    */
    int k = connect(sock, (struct sockaddr *) &sockadrr_out, sizeof(sockadrr_out));
    if (k < 0) {
        printf("couldn't connect to server");}
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);}
    /*
     _____________________________________________________________________________
    |int send(int sockfd, const void *msg, int len, int flags)                    |
    |sockfd = socket descriptor through which information is sent                 |
    |msgid = Pointer to information that is sent                                  |
    |len = The size of the information in bytes                                   |
    |flags = its value is 0.                                                      |
    |The function returns the amount of bytes actually sent.                      |
    |Quantity could be less than we wanted to sent                                |
    |(need to make sure how many are actually sent and complete what is missing)  |
    |The function returns a value of -1 if there is an error.                     |
    |_____________________________________________________________________________|
    */
    send(sock, &filesize, sizeof(filesize), 0); // send single file size
    /* Send Data On Cubic Algorithm! */
    send_file(filename, fp, sock);
    char buf[256];
    len = sizeof(buf);
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) {
            perror("getsockopt");
            return -1;}
    printf("[+]Files data sent successfully over %s.\n",buf);
    /* setting & getting reno CC (CONGESTION CONTROL) algorithm */
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) != 0) {
            perror("setsockopt");
            return -1;}
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) { // Reno
        perror("getsockopt");
        return -1;}
    printf("starting to send %s\n",buf);
    /* Send Data On Reno Algorithm! */
    send_file(filename, fp, sock);
    printf("[+]Files data sent successfully over %s.\n",buf);
    printf("[+]Closing the connection.\n");
    fclose(fp);
    close(sock);
    return 0;
}

int main() {
    Tcp_CC();
}
