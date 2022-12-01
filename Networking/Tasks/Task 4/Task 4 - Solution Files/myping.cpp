// icmp.cpp
// Robert Iakobashvili for Ariel uni, license BSD/MIT/Apache
// 
// Sending ICMP Echo Requests using Raw-sockets.
//
// 1. Change SOURCE_IP and DESTINATION_IP to the relevant for your computer
// 2. Compile it using MSVC compiler or g++ ("g++ -o program_name ICMP.cpp")
// 3. Run it from the account with administrative permissions, since opening of
//    a raw-socket requires elevated preveledges. ("sudo ./program_name")
// 4. For debugging and development, run MS Visual Studio (MSVS) as admin by
//    right-clicking at the icon of MSVS and selecting from the right-click 
//    menu "Run as administrator"
//  Note. You can place another IP-source address that does not belong to your
//  computer (IP-spoofing), i.e. just another IP from your subnet, and the ICMP
//  still be sent, but do not expect to see ICMP_ECHO_REPLY in most such cases
//  since anti-spoofing is wide-spread.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

/* IP header len for echo req */
#define IP_HDRLEN 20
/* ICMP header (type + code + checksum + id + seq) len for echo req */
#define ICMP_HDRLEN 8
/* ICMP identifier (some number to trace the response) */
#define IDENTIFIER 18
/* current host ip-address */
#define SOURCE_IP "192.168.190.129"
/* i.e the gateway or ping to google.com for their ip-address */
#define DESTINATION_IP "8.8.8.8"

/* Checksum algo */
unsigned short calculate_checksum(unsigned short* paddress, int len);

struct cooked_packet {
    /* cook ip header to an unused array (unnecessary) */
    char IP_HDR[IP_HDRLEN];
    /* copy icmp header */
    struct icmphdr ICMP_HDR;
    /* copy icmp message */
    char MSG[64];
};

int main () {
    
    /////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// Prepare ICMP header //////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /* init ICMP-header struct */
    struct icmp request_icmphdr; // ICMP-header
    /* init ICMP message */
    char message[IP_MAXPACKET] = "This is the ping.";
    int messagelen = strlen(message) + 1;
    /* Message Type (8 bits): ICMP_ECHO_REQUEST */
    request_icmphdr.icmp_type = ICMP_ECHO;
    /* Message Code (8 bits): echo request */
    request_icmphdr.icmp_code = 0;
    /* Identifier (16 bits): some number to trace the response.
       It will be copied to the response packet and used to map response to the request sent earlier.
       Thus, it serves as a Transaction-ID when we need to make "ping" */
    request_icmphdr.icmp_id = IDENTIFIER; // hai
    /* Sequence Number (16 bits): starts at 0 */
    request_icmphdr.icmp_seq = 0;
    /* ICMP header checksum (16 bits): set to 0 not to include into checksum calculation */
    request_icmphdr.icmp_cksum = 0;
    /* Combine the packet */
    char packet[IP_MAXPACKET];
    /* Next, ICMP header */
    memcpy (packet, &request_icmphdr, ICMP_HDRLEN);
    /* After ICMP header, add the ICMP message. */
    memcpy (packet + ICMP_HDRLEN, message, messagelen);
    /* Calculate the ICMP header checksum */
    request_icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet),ICMP_HDRLEN+messagelen);
    /* should copy icmp header again after checksum */
    memcpy(packet, &request_icmphdr, ICMP_HDRLEN);
    struct sockaddr_in dest_in;
    memset(&dest_in, 0, sizeof (struct sockaddr_in));
    dest_in.sin_family = AF_INET;
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
    if (inet_pton(AF_INET, DESTINATION_IP, &(dest_in.sin_addr)) <= 0) {
        fprintf (stderr, "inet_pton() failed for destination-ip with error: %d", errno);
        return -1;
    }
    /* (The port is irrelant for Networking and therefore was zeroed). */
    /* Create raw socket
     ___________________________________________________________________________________
    |int socket(int af, int type, int protocol, 0)                                      |
    |af = protocol (IPv4 value: AF_INET)                                                |
    |type = type of the socket (TCP: SOCK_STREAM, UDP: SOCK_DGRAM)                      |
    |protocol = transport layer protocol (TCP: SOCK_STREAM, UDP: SOCK_DGRAM)            |
    |[protocol 0 = default (depending on the type of socket)]                           |
    |If no socket is established the function will return the value -1 (INVALID_SOCKET) |
    |___________________________________________________________________________________|
    */
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
        fprintf (stderr, "socket() failed with error: %d\n", errno);
        fprintf (stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }
    /* This socket option IP_HDRINCL says that we are building IPv4 header by ourselves, and
       the networking in kernel is in charge only for Ethernet header.
       so since we want that the kernel will build the IPv4 header we're not setting this option */

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// send Echo request ///////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /* prepare start time */
    struct timespec time_start, time_end;
    long double rtt_milli_sec=0, rtt_micro_sec;
    struct timeval tv_out;
    tv_out.tv_sec = 1; /* Gives the timeout delay for receiving packets (in seconds) */
    tv_out.tv_usec = 0;
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    /* end prepare start time */
    /* send echo request (using sendto() for sending datagrams)
     ___________________________________________________________________________________
    |int sendto(                                                                        |
    |           int sockfd                                                              |
    |           const void *buf                                                         |
    |           size_t len                                                              |
    |           int flags                                                               |
    |           const struct sockaddr *dest_addr                                        |
    |           socklen_t addrlen                                                       |
    |);                                                                                 |
    |___________________________________________________________________________________|
    */
    if (sendto(sock, packet, ICMP_HDRLEN+messagelen, 0, (struct sockaddr*)&dest_in, sizeof(dest_in)) == -1) {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// print Echo request //////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /* should print line divider in color */
    std::cout << "\033[1;34m-------------------------------\033[0m\n";
    /* should print "The echo request was sent successfully!" in color */
    std::cout << "\033[1;32mThe echo request was sent successfully!\033[0m\n";
    /* should print line divider in color */
    std::cout << "\033[1;34m-------------------------------\033[0m\n";
    /* print "ECHO REQUEST DETAILS" in color */
    std::cout << "\033[4;33mECHO REQUEST DETAILS:\033[0m\n";
    /* print the type */
    printf("ECHO REQUEST type: %d\n", request_icmphdr.icmp_type);
    /* print the code */
    printf("ECHO REQUEST code: %d\n", request_icmphdr.icmp_code);
    /* print the identifier */
    printf("ECHO REQUEST identifier: %d\n", request_icmphdr.icmp_id);
    /* print the sequence */
    printf("ECHO REQUEST sequence: %d\n", request_icmphdr.icmp_seq);
    /* print the message */
    printf("ECHO REQUEST message: \"%s\"\n", message);
    /* should print line divider in color */
    std::cout << "\033[1;34m-------------------------------\033[0m\n";

    /////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// receive Echo request //////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /*
     ___________________________________________________________________________________
    |int recvfrom(                                                                      |
    |           int sockfd                                                              |
    |           void *buf                                                               |
    |           size_t len                                                              |
    |           int flags                                                               |
    |           struct sockaddr *src_addr                                               |
    |           socklen_t *addrlen                                                      |
    |);                                                                                 |
    |___________________________________________________________________________________|
    */
    struct cooked_packet* reply_packet = (struct cooked_packet*)malloc(sizeof(struct cooked_packet));
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    while(1) {
        if (recvfrom(sock, reply_packet, sizeof(struct cooked_packet), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0) {
            fprintf (stderr, "recvfrom() failed with error: %d", errno);
            return -1;
        } else {
            /* prepare end time */
            clock_gettime(CLOCK_MONOTONIC, &time_end);
            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
            rtt_milli_sec = (time_end.tv_sec-time_start.tv_sec) * 1000.0 + timeElapsed;
            rtt_micro_sec = (rtt_milli_sec / 1000.0); // convert tv_sec & tv_usec to millisecond
            /* end prepare end time */
            /* check: rcho_(ping)_reply_type == 0. echo_reply_code == 0 (stands for echo replay) */
            if(reply_packet->ICMP_HDR.type==0&&reply_packet->ICMP_HDR.code==0&&
                reply_packet->ICMP_HDR.un.echo.id==IDENTIFIER) {break;}
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// print Echo request ///////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /* should print "The echo reply was received successfully!" in color */
    std::cout << "\033[1;32mThe echo reply was received successfully!\033[0m\n";
    /* should print RTT time in color */
    std::cout << "\033[1;36m";
    printf("RTT = %Lf milliseconds (%Lf microseconds).", rtt_milli_sec, rtt_micro_sec);
    std::cout << "\033[0m\n";
    /* should print line divider in color */
    std::cout << "\033[1;34m-------------------------------\033[0m\n";
    /* print "ECHO REPLY DETAILS" in color */
    std::cout << "\033[4;33mECHO REPLY DETAILS:\033[0m\n";
    /* print the type */
    printf("ECHO REPLY type: %d\n", reply_packet->ICMP_HDR.type);
    /* print the code */
    printf("ECHO REPLY code: %d\n", reply_packet->ICMP_HDR.code);
    /* print the identifier */
    //printf("ECHO REPLY identifier: %d\n", reply_packet->ICMP_ID);
    printf("ECHO REPLY sequence: %d\n", reply_packet->ICMP_HDR.un.echo.id);
    /* print the sequence */
    // printf("ECHO REPLY sequence: %d\n", reply_packet->ICMP_SEQ);
    printf("ECHO REPLY sequence: %d\n", reply_packet->ICMP_HDR.un.echo.sequence);
    /* print the message */
    printf("ECHO REPLY message: \"%s\"\n", reply_packet->MSG);
    /* free allocation */
    free(reply_packet);
    /* Close the raw socket descriptor. */
    close(sock);
    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short * paddress, int len) {
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1) {
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}
	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits
	return answer;
}