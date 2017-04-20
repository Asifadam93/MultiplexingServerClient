#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

int main(void)
{
	int socketFd;
 	struct sockaddr_in clientAddr;
 	char ch;

 	// socket creation
 	//fd : file descripteur
 	socketFd = socket(AF_INET,SOCK_STREAM,0);

 	if(socketFd < 0){
 		printf("Error socket\n");
 	}

 	bzero(&clientAddr, sizeof(clientAddr));

 	clientAddr.sin_family = AF_INET;
 	clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 	clientAddr.sin_port = htons(8087);

 	if(connect(socketFd, (struct sockaddr *) &clientAddr, 
 		sizeof(clientAddr)) < 0){
 		printf("Error connect\n");
 	} else {
 		write(socketFd,"Test", 4);
 		//read(socketFd,&ch,1);
 		//printf("%c\n", ch);
 	}


 	close(socketFd);
 	

	return 0;
}
