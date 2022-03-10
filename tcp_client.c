#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <stdlib.h> //for exit(0);
#include<arpa/inet.h>
#include<unistd.h>
#include <netinet/tcp.h>	//Provides declarations for tcp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h> // inet_addr


unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}

//Create a Socket for server communication
short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}
//try to connect with server
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
// Send the data to the server and set the timeout of 20 seconds
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
    return shortRetval;
}
//receive the data from the server
int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    printf("Response %s\n",Rsp);
    return shortRetval;
}
//main driver program
int main(int argc, char *argv[])
{
    int hSocket, read_size;
    struct sockaddr_in server;
    char SendToServer[100] = {'a','b'};
    char server_reply[200] = {0};
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
    char dest_ip[32];
    
    
         
    //Create socket
    hSocket = SocketCreate();
    if(hSocket == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket is created\n");
    //Connect to remote server
    if (SocketConnect(hSocket,argv[2]) < 0)
    {
        perror("connect failed.\n");
        return 1;
    }
    struct sockaddr_in sin;
    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;
    sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = inet_addr (argv[2]);

    iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(data);
	iph->id = htonl (54321);	//Id of this packet
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;		//Set to 0 before calculating checksum
	iph->saddr = inet_addr (argv[1]);	//Spoof the source ip address
	iph->daddr = inet_addr(argv[2]);
	
	//Ip checksum
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);


    printf("Sucessfully conected with server\n");
    printf("Enter the Message: ");
    //Send data to the server
    for (int i =0; i < 100; i++) {
        // SocketSend(hSocket, SendToServer, strlen(SendToServer));
        if (sendto (hSocket, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		    {
			    perror("sendto failed");
		    }
		//Data send successfully
		else
		    {
			    printf ("Packet Send. Length : %d \n" , iph->tot_len);
		    }
        // sleep for 1 seconds
        sleep(1);
    }

    //Received the data from the server
    read_size = SocketReceive(hSocket, server_reply, 200);
    printf("Server Response : %s\n\n",server_reply);
    close(hSocket);
    shutdown(hSocket,0);
    shutdown(hSocket,1);
    shutdown(hSocket,2);
    return 0;
}