
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

#define PORT 7777 //8087 parceque Asif :)

int main(){

	int maxClients = 2, clientSocket[3], masterSocket, newSocket, sd, maxSd, activity, addressLength, nbChar;
    char buffer[255];
    char bufferRetour[255];
    struct sockaddr_in address;
    int fdsAdded = 0;
    int retourClientAFaire = 0;

    fd_set readfds;
	
	// init all client to 0
	for(int i=0; i<maxClients; i++){
		clientSocket[i] = 0;
	}


    // create master socket en ipv4 (af_inet) et en TCP (SOCK_STREAM) 
    if((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("\033[1;31mMaster socket creation error \033[0m");
        exit(EXIT_FAILURE);
    }

    // clear address memory
    bzero(&address, sizeof(address));

    // configuration du socket adress (AF_INET : ipv4, INADDR_ANY : toute les interfaces) 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind master socket to port
    if(bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("\033[1;31mMaster socket binding error \033[0m");
        exit(EXIT_FAILURE);
    }

    // listen max 3 pending connections
    if(listen(masterSocket, 4) < 0){
        perror("\033[1;31mlisten error \033[1m");
        exit(EXIT_FAILURE);
    }

    // accept incoming connection
    addressLength = sizeof(address);
    printf("\033[1;33mWaiting for connections \033[0m\n");

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

            //TODO : comprendre, voir le man du select
            if(sd > maxSd){
                maxSd = sd;
            }
        }



        /*
         * PARTIE QUI PERMET D'AJOUTER UN NOUVEAU FD A LIRE
         */
        activity = select(maxSd+1, &readfds, NULL, NULL, NULL);

        // Si on a une demande de connexion sur notre serveur
        if(FD_ISSET(masterSocket, &readfds)){

            //récupération de la socket entrante
            if((newSocket = accept(masterSocket, (struct sockaddr *)&address, ((socklen_t*)&addressLength))) < 0){
                perror("\033[1;31mError : \033[1m accept \n");
            }

            //affichage du fd du socket entrant
            printf("\033[1;32mNew connection - socket fd : \033[0m %d \n", newSocket);
        
            //on ajoute le nouveau socket
            fdsAdded = -1;
            for(int i = 0; i<maxClients; i++){

                if(clientSocket[i] == 0){
                    fdsAdded = i;
                    clientSocket[i] = newSocket;
                    break;
                }
            }

            //affichage du socket ajouté ou pas (si erreur)
            if (fdsAdded>= 0) {
                printf("\033[1;32mSocket added : \033[0m %d \n", fdsAdded);
            } else {
                printf("\033[1;31mCan't add more socket !\033[0m \n");
            }

        }

        /*
         * PARTIE QUI PERMET DE LIRE LES FD DES CLIENTS
         */
        for (int i = 0; i < maxClients; i++){
            
            sd = clientSocket[i];


            if(FD_ISSET(sd, &readfds)){

                if((nbChar = read(sd, buffer, 1024)) == 0){

                    printf("\033[1;31mSomeone disconnected !\033[0m \n");

                    close(sd);
                    clientSocket[i] = 0;

                } else {

                    for (int i = 0; i < nbChar; ++i)
                    {
                        printf("%c", buffer[i]);
                    }
                }

                printf("nbChar = %i\n", nbChar);
            }

            /*if (retourClientAFaire == 1) {
                printf("TEST");
                strcpy(bufferRetour, "Message bien Recu");
                write(sd,bufferRetour,17);

                retourClientAFaire = 0;
            }*/


        }

    }

}
