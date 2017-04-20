
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define PORT 8087

int main(){

	int maxClients = 5, clientSocket[5], masterSocket, newSocket, sd, maxSd, activity, addressLength, valRead;
    char buffer[1025];
    struct sockaddr_in address;

    fd_set readfds;
	
	// init all client to 0
	for(int i=0; i<maxClients; i++){
		clientSocket[i] = 0;
	}

    // create master socket
    if((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Master socket creation error");
    }

    // set socket type
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind master socket to port
    if(bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Master socket binding error");
    }

    // listen max 3 pending connections
    if(listen(masterSocket, 3) < 0){
        perror("listen error");
    }

    // accept incoming connection
    addressLength = sizeof(address);
    printf("Waiting for connections \n");

    while(1){

        // clear readfds socket set
        FD_ZERO(&readfds);
        maxSd = masterSocket;

        // add master socket to readfds socket set
        FD_SET(masterSocket, &readfds);

        // add child socket to readfds set
        for(int i=0; i<maxClients; i++){

            sd = clientSocket[i];

            if(sd > 0){
                FD_SET(sd, &readfds);
            }

            if(sd > maxSd){
                maxSd = sd;
            }

        }

        // wait for an activity on sockets
        activity = select(maxSd+1, &readfds, NULL, NULL, NULL);

        // if something changed on master socket
        if(FD_ISSET(masterSocket, &readfds)){

            if((newSocket = accept(masterSocket, (struct sockaddr *)&address, ((socklen_t*)&addressLength))) < 0){
                perror("accept error");
            }

            printf("New connection - socket fd : %d\n", newSocket);
        
            for(int i = 0; i<maxClients; i++){

                if(clientSocket[i] == 0){
                    clientSocket[i] = newSocket;
                    printf("Socket added : %d\n", i);
                    break;
                }

            }
        }

        for (int i = 0; i < maxClients; i++){
            
            sd = clientSocket[i];

            if(FD_ISSET(sd, &readfds)){

                if((valRead = read(sd, buffer, 1024)) == 0){

                    printf("Someone disconnected\n");
                    close(sd);
                    clientSocket[i] = 0;

                } else {

                    for (int i = 0; i < valRead; ++i)
                    {
                        printf("%c", buffer[i]);
                    }
                    printf("\n");
                }

            }

        }

    }

}
