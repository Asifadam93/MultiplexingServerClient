#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 7777

int main(void)
{
	int socketFd;
 	struct sockaddr_in clientAddr;
 	char ch;
	char saisie[255];
	char buffer[255];
	int nbChar;
 	
 	
 	// socket creation
 	//fd : file descripteur
 	socketFd = socket(AF_INET,SOCK_STREAM,0);

 	if(socketFd < 0){
 		printf("Error socket\n");
 	}

 	bzero(&clientAddr, sizeof(clientAddr));

 	clientAddr.sin_family = AF_INET;
 	clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 	clientAddr.sin_port = htons(PORT);


 	if(connect(socketFd, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0) {
 		printf("Error connect\n");
 	} else {
 		while(1) {

			fgets(saisie, sizeof(saisie), stdin);
	 		write(socketFd,saisie, strlen(saisie));

			
			printf("%zu\n",strlen(saisie));  //DEBUG


			nbChar = read(socketFd, &buffer, sizeof(buffer));
            for (int i = 0; i < nbChar; ++i)
            {
                printf("\033[0;36m%c\033[0m", buffer[i]);
            }
            printf("\n");

		}


 	}


 	close(socketFd);
 	

	return 0;
}
