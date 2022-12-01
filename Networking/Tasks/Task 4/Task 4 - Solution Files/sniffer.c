//
// Created by tal on 01/06/2021.
//
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
//#include "headers.h"
#define ETHER_ADDR_LEN 6
#define PCKT_LEN 1024
#define DATALEN (PCKT_LEN - sizeof(struct icmp_hdr) - sizeof(struct ip_hdr))

/* Ethernet header (original) */
struct eth_hdr {
    unsigned char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address (1 byte) */
    unsigned char ether_shost[ETHER_ADDR_LEN];	  /* Source host address (1 byte) */
    u_short ether_type;					   /* IP? ARP? RARP? etc (2 bytes) */
};

/* IP header (original) */
struct ip_hdr {
    unsigned int ip_hl:4;                /* header length (4 bytes) */
    unsigned int ip_v:4;
    u_int8_t ip_tos;                        /* type of service (1 byte) */
    u_short ip_len;                        /* total length (2 bytes) */
    u_short ip_id;                        /* identification (2 bytes) */
    u_short ip_off;                        /* fragment offset field (2 bytes) */
    u_int8_t ip_ttl;                        /* time to live (1 byte) */
    u_int8_t ip_p;                        /* protocol (1 byte) */
    u_short ip_sum;                        /* checksum (2 bytes) */
    struct in_addr ip_src, ip_dst;        /* source and dest address */
};

struct icmp_hdr {
    unsigned char icmp_type;        /* icmp type (1 byte) */
    unsigned char icmp_code;        /* icmp code (1 byte) */
    unsigned short icmp_cksum;		/* icmp checksum (2 bytes) */
    unsigned short icmp_id;				/* icmp identifier (2 bytes) */
    unsigned short icmp_seq;			/* icmp sequence number (2 bytes) */
};


struct packet_s {
    struct ip_hdr ip_hdr;
    struct icmp_hdr icmp_hdr;
    char echoData[DATALEN];
};

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct packet_s* pckt =(struct packet_s*)(packet+sizeof (struct eth_hdr));
    if(pckt->ip_hdr.ip_p==IPPROTO_ICMP) {
        printf("\033[1;34m-------------------------------\033[0m\n");
        printf("\033[4;33mIP DETAILS:\033[0m\n");
        printf("Source:  %s\n", inet_ntoa(pckt->ip_hdr.ip_src));//->ip_src));
        printf("Destination: %s\n", inet_ntoa(pckt->ip_hdr.ip_dst));
        printf("\033[4;33mICMP DETAILS:\033[0m\n");
        printf("Type: %hhu\n", pckt->icmp_hdr.icmp_type);
        printf("Code: %hhu\n", pckt->icmp_hdr.icmp_code);
        printf("Id: %hu\n", pckt->icmp_hdr.icmp_id);
        printf("Seq: %hu\n", pckt->icmp_hdr.icmp_seq);
        printf("Data : %s\n",pckt->echoData);
    }
}

int main() {
    /* A pcap_t is a handle used to read packets from a network interface,
       or from a pcap (or, in newer versions of libpcap, pcap-ng) file containing packets. */
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    /* BPF allows a user-program to attach a filter to the socket,
       which tells the kernel to discard unwanted packets. */
    struct bpf_program fp;
    /* a filter to attach */
    char filter_exp[] = "icmp";

    bpf_u_int32 net;
    // Step 1: Open live pcap session on NIC (Network interface controller) with name ens33
    /*
    Initialize a raw socket, set the network device into promiscuous mode (1).
    1000 is packet buffer timeout in milliseconds.
    */
    handle = pcap_open_live("ens33", BUFSIZ, 1, 1000, errbuf);
    // Step 2: Compile filter_exp into BPF psuedo-code
    /*
     _______________________________________________________
    |int pcap_compile(pcap_t *p,                            |
    |                 struct bpf_program *fp,               |
    |                 const char *str,                      |
    |                 int optimize,                         |
    |                 bpf_u_int32 netmask                   |
    |);                                                     |
    |used to compile the string str into a filter program   |
    |returns 0 on success and PCAP_ERROR on failure.        |
    |_______________________________________________________|
    */
    pcap_compile(handle, &fp, filter_exp, 0, net);
    /*
     _______________________________________________________
    |int pcap_setfilter(pcap_t *p, struct bpf_program *fp); |
    |used to specify a filter program.                      |
    |returns 0 on success and PCAP_ERROR on failure.        |
    |_______________________________________________________|
    */
    pcap_setfilter(handle, &fp);
    // Step 3: Capture packets
    /*
     ________________________________________________________________
    |processes packets from a live capture                           |
    |first arg - pcap_t                                              |
    |second arg - cnt (-1 for nonstop iterations)                    |
    |third arg - function to call to when packet captured            |
    |last arg - can pass arg in any custom data you need to          |
    |    access to from within your handler function,                |
    |    so you don't need a global variable to accomplish the same. |
    |    (in this case we pass NULL).                                |
    |returns 0 if exhausted, -1 if an error occurs,                  |
    |returns -2 if the loop terminated due to a call to              |
    |    pcap_breakloop() before any packets were processed.         |
    |    instead, it attempts to read more packets.                  |
    |It does not return when live read timeouts occur;               |
    |________________________________________________________________|
    */
    pcap_loop(handle, -1, got_packet, NULL);
    pcap_close(handle);   //Close the handle
    return 0;
}