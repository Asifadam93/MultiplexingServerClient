#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 8087

int main(void)
{
	int socketFd;
 	struct sockaddr_in clientAddr;
 	char ch;
	char saisie[255];
	char buffer[255];
	int nbChar;
 	fd_set readfds;	//EXPERIMENTAL
 	int maxFd;
 	int activity;


 	// socket creation
 	//fd : file descripteur
 	socketFd = socket(AF_INET,SOCK_STREAM,0);

 	if(socketFd < 0){
 		printf("Error socket\n");
 	}

 	bzero(&clientAddr, sizeof(clientAddr));

 	clientAddr.sin_family = AF_INET;
 	//clientAddr.sin_addr.s_addr = inet_addr("195.148.124.79");
 	clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 	clientAddr.sin_port = htons(PORT);

 	if(connect(socketFd, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0) {
 		printf("Error connect\n");
 	} else {
 		while(1) {

			maxFd = (socketFd > STDIN_FILENO)?socketFd:STDIN_FILENO;

	        // clear readfds socket set
	        FD_ZERO(&readfds);

	        // add master socket to readfds socket set
	        FD_SET(socketFd, &readfds);
	        FD_SET(STDIN_FILENO, &readfds);


	        // Wait for activity on sockets
	        activity = select(maxFd+1, &readfds, NULL, NULL, NULL);

	        if(activity < 0){
	            perror("\033[1;31mError : \033[0m select \n");
	        }


	        //Si l'utilisateur a saisi quelque chose
	        if(FD_ISSET(STDIN_FILENO, &readfds)){
	           // printf("L'utilisateur a saisi quelque chose : ");	//DEBUG

				if((nbChar = read(STDIN_FILENO, buffer, 1024)) == 0){
                    perror("\033[1;31mError : \033[0m STDIN_FILENO\n");
                } else {
                    //fputs(buffer, stdout);               //DEBUG
                    
                    write(socketFd, buffer, nbChar);
                    memset(buffer, 0,sizeof(buffer));

                    //printf("nbChar = %i\n", nbChar); 	//DEBUG
                }

	        }

	        //Si le serveur a envoyé quelque chose
	        if(FD_ISSET(socketFd, &readfds)){
	            //printf("Le serveur a envoyé quelque chose : ");

				if((nbChar = read(socketFd, buffer, 1024)) == 0){
                    perror("\033[1;31mError : \033[0m socket serveur");
                } else {

					printf("\033[0;37m%s\033[0m", buffer);

                    memset(buffer, 0,sizeof(buffer));
                }
	        }
		}


 	}


 	close(socketFd);
 	

	return 0;
}
