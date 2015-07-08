#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>   
#include <errno.h>     
#include <string.h>    
#include <fcntl.h>    
#include <sys/wait.h>    
#include <sys/resource.h>    
#include <sys/mman.h>    
#include <signal.h>    

#define BUFF_SIZE 1024*32     /* Longest string to echo */
struct timeval tv;
time_t sec_begin, sec_end, sec_elapsed;

void DieWithError(char *errorMessage){  perror(errorMessage);  exit(1);}

int main(int argc, char *argv[]){

int child=fork();
if (child<0) { DieWithError("Error fork");}
//Parent
else if(child!=0){
/*	//Separate test
	sigset_t sig_proc;
	sigemptyset(&sig_proc);
	sigaddset (&sig_proc, SIGCONT);
	sigprocmask(SIG_BLOCK,&sig_proc, NULL);
	wait(NULL);
*/
	long long int count=0;				/* Count recived data*/
	int sock;	             			/* Socket */
	struct sockaddr_in ServAddr, ClntAddr;		/* Local address & client address*/
	unsigned int cliAddrLen;        		/* Length of incoming message */
	char Buffer[BUFF_SIZE];				/* Buffer for echo string */
	unsigned short ServPort=50705;			/* Server port */
	int recvMsgSize;		    		/* Size of received message */

	setpriority(PRIO_PROCESS, 0, -20);
	tv.tv_sec = 2;					/* Set the timeout for recv/recvfrom*/
	memset(&ServAddr, 0, sizeof(ServAddr));   	/* Zero out structure */
	ServAddr.sin_family = AF_INET;                	/* Internet address family */
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); 	/* Any incoming interface */
	ServAddr.sin_port = htons(ServPort);	  	/* Local port */

while(1){	
        printf("\nWaiting for udp messages........  ");
        /* Create socket for receiving datagrams */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            DieWithError("socket() failed");
        /* Set the socket as reusable */
	int true = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int))!=0) DieWithError("Error");
	//if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int))!=0) DieWithError("Error");
	/* Bind to the local address */
        if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
            DieWithError("bind() failed");
        /* Set the size of the in-out parameter */
                cliAddrLen = sizeof(ClntAddr);
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, Buffer, BUFF_SIZE, 0,(struct sockaddr *) &ClntAddr, &cliAddrLen)) < 0)
                DieWithError("recvfrom() failed");
        printf("\n\tHandling client %s\n", inet_ntoa(ClntAddr.sin_addr));
        /* Send received datagram back to the client */
        if (sendto(sock,"OK", 2, 0,(struct sockaddr *) &ClntAddr, sizeof(ClntAddr)) != recvMsgSize)
                DieWithError("sendto() sent a different number of bytes than expected");
	/*Set waiting limit*/
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) DieWithError("Error");
	/*Send signal to the child to start streaming*/
	kill( child, SIGCONT );
	/*Reciving*/
	count=0;
	while(1){
		recvMsgSize = recvfrom(sock, Buffer, BUFF_SIZE, 0,(struct sockaddr *) &ClntAddr, &cliAddrLen);
		if(errno==EAGAIN) { setsockopt(sock, SOL_SOCKET, 0,&tv,sizeof(tv));
			    printf("\t(Timeout is reached)");     break; }
		//if(recvMsgSize<0)  DieWithError("\nReciving failed\n ");
		count=count+recvMsgSize;
		//printf("\r\t\t\t\tRecived %lld MB",count);
		//memset(&Buffer,0,sizeof(Buffer) );
		}//End local while
	printf("\n\tRecived %lld MB\n",count/1024/1024);
	close(sock);
} //End global while (Ctrl-C)
wait( NULL );
printf("child terminated\n");
exit(2);
} //End parent programm

else if(child==0){
	time_t sec_begin, sec_end, sec_elapsed;
	long long int count=0;				/* Count recived data*/
	int sock;	             			/* Socket */
	struct sockaddr_in ServAddr, ClntAddr;		/* Local address & client address*/
	unsigned int cliAddrLen;        		/* Length of incoming message */
	char Buffer[BUFF_SIZE];				/* Buffer for echo string */
	unsigned short ServPort=50707;			/* Server port */
	int recvMsgSize,sendMsgSize;    		/* Size of received message */

	setpriority(PRIO_PROCESS, 0, -20);
	tv.tv_sec = 2;					/* Set the timeout for recv/recvfrom*/
	memset(&ServAddr, 0, sizeof(ServAddr));   	/* Zero out structure */
	ServAddr.sin_family = AF_INET;                	/* Internet address family */
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); 	/* Any incoming interface */
	ServAddr.sin_port = htons(ServPort);	  	/* Local port */
while(1){
        printf("\n\t\t\t\t\tWaiting for udp messages........  ");
        /* Create socket for sending/receiving datagrams */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            DieWithError("socket() failed");
        /* Set the socket as reusable */
	int true = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int))!=0) DieWithError("Error");
	//if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int))!=0) DieWithError("Error");
	/* Bind to the local address */
        if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
            DieWithError("bind() failed");
        /* Set the size of the in-out parameter */
                cliAddrLen = sizeof(ClntAddr);
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, Buffer, BUFF_SIZE, 0,(struct sockaddr *) &ClntAddr, &cliAddrLen)) < 0)
                DieWithError("recvfrom() failed");
        printf("\n\t\t\t\t\tHandling client %s", inet_ntoa(ClntAddr.sin_addr));
        /* Send received datagram back to the client */
        if (sendto(sock,"OK", 2, 0,(struct sockaddr *) &ClntAddr, sizeof(ClntAddr)) != recvMsgSize)
                DieWithError("sendto() sent a different number of bytes than expected");

	count=0;
	/*Wait for father signal*/
	//pause();
	memset(&Buffer,0,sizeof(Buffer) );

	sec_begin= time(NULL);sec_elapsed=0;
	while(sec_elapsed<10)
	{
	        sec_end=time(NULL);
	        sec_elapsed=sec_end-sec_begin;

		sendMsgSize=sendto(sock,Buffer, BUFF_SIZE, 0,(struct sockaddr *) &ClntAddr, sizeof(ClntAddr)); 
        	if (recvMsgSize<0){ DieWithError("sendto() failed");}
		count=count+sendMsgSize;
		}//End local while
	printf("\n\t\t\t\t\tSended %lld MB",count/1024/1024);
	close(sock);

} //End global while (Ctrl-C)

} //End child program
return 0;
}//End of life

