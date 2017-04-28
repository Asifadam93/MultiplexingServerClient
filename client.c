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
	char saisie[255];
	char buffer[1025];

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

 	if(connect(socketFd, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0) {
 		printf("Error connect\n");
 	} else {
 		while(1) {

			fgets(saisie, 4, stdin);
	 		write(socketFd,saisie, 4);
	 		//read(socketFd,&ch,1);
	 		//printf("%c\n", ch);
 		}
 	}


 	close(socketFd);
 	

	return 0;
}
