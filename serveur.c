
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

#define PORT 8001 //8087 parceque Asif :)

/**
 * Fonction permettant de savoir si l'utilisateur a saisi une commande irc
 *
 */
int searchCommandIRC(char* chaine, char* needle)
{
    char *c = strstr(chaine, needle);
    int position = c -chaine;

    if ((c != NULL) && (position >= 0)) {
        printf("\033[0;36mFound command %s\033[0m\n", needle);
        return 1;
    }


    return 0;
}




int main(){

	int maxClients = 2, 
        clientSocket[maxClients], 
        masterSocket, 
        newSocket, 
        sd, 
        maxSd, 
        activity, 
        addressLength, 
        nbChar,
        fdsAdded = 0,
        usersCount = 0;
    char buffer[255];
    char bufferRetour[255];

    struct sockaddr_in address;
    fd_set readfds;
    const char usersNick[maxClients][20];


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

        // add master socket to readfds socket set
        FD_SET(masterSocket, &readfds);
        maxSd = masterSocket;

        // add child socket to readfds set
        for(int i=0; i<maxClients; i++){

            // sd : socket descriptor
            sd = clientSocket[i];

            // add to read, if it's a valid socket descriptor
            if(sd > 0){
                FD_SET(sd, &readfds);
            }

            // highest socket descriptor, for select function
            if(sd > maxSd){
                maxSd = sd;
            }
        }


        // Wait for activity on sockets
        activity = select(maxSd+1, &readfds, NULL, NULL, NULL);

        if(activity < 0){
            perror("\033[1;31mError : \033[1m select \n");
        }

        // If any activity detected on masterSocket
        if(FD_ISSET(masterSocket, &readfds)){

            //Accept the connection
            if((newSocket = accept(masterSocket, (struct sockaddr *)&address, ((socklen_t*)&addressLength))) < 0){
                perror("\033[1;31mError : \033[1m accept \n");
            }

            //show client fd
            printf("\033[1;32mNew connection asked - socket fd : \033[0m %d \n", newSocket);
        
            //add new socket to clientSocket array
            fdsAdded = -1;
            for(int i = 0; i<maxClients; i++){

                // add if clientSocket is empty
                if(clientSocket[i] == 0){
                    fdsAdded = i;

                    usersCount++;

                    char str[20];
                    sprintf((char*) usersNick[i], "User_%d", usersCount); 
                     //cast pour éviter un warning. TODO : voir différence entre char* et const char *

                    clientSocket[i] = newSocket;
                    break;
                }
            }

            //Show socket added state
            if (fdsAdded>= 0) {
                printf("\033[1;32m  -> Added \033[0m \033[1;37m%s\033[0m\n", usersNick[fdsAdded] );


            } else {
                printf("\033[1;31m  -> Error : Can't add more socket !\033[0m \n");
            }

        }

        // Other socket operations
        for (int i = 0; i < maxClients; i++){
            //récupération du  
            sd = clientSocket[i];


            if(FD_ISSET(sd, &readfds)){

                // read incomming msg
                if((nbChar = read(sd, buffer, 1024)) == 0){

                    printf("\033[1;31m%s disconnected !\033[0m \n", usersNick[i] );

                    close(sd);
                    clientSocket[i] = 0;

                } else {
                    //Recherche des commandes IRC
                    if (searchCommandIRC(buffer, "NICK") == 1) {

                    }
                    //Recherche des commandes IRC
                    if (searchCommandIRC(buffer, "USER") == 1) {

                    }

                    // Transfert aux autres clients
                    /*
                    suppression pour cause de test avec un client IRC
                    strcpy(bufferRetour, usersNick[i]);
                    strcat(bufferRetour, " : ");
                    strcat(bufferRetour, buffer);
                    printf("%s", bufferRetour);
                    */

                    printf("%s", buffer);
                    for (int r = 0; r < maxClients; r++){
                        if ((r != i) && (clientSocket[r] > 0))  {
                            write(clientSocket[r],buffer,strlen(buffer));
                        }
                    }

                    memset(buffer, 0,sizeof(buffer));
                    memset(buffer, 0,sizeof(bufferRetour));
                }


                //strcpy(bufferRetour, "Message bien Recu");
                //write(sd,bufferRetour,strlen(bufferRetour));
            }
        }

    }

    return 0;

}





