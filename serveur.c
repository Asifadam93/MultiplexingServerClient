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

//https://stackoverflow.com/questions/3796479/how-to-remove-a-carriage-return-from-a-string-in-c
void remove_char_from_string(char *str)
{
    int i=0;
    int len = strlen(str)+1;

    for(i=0; i<len; i++)
    {
        printf("%i:%c", (int) str[i], str[i]);
        if (str[i] == '\n')
        {

            // Move all the char following the char "c" by one to the left.
            strncpy(&str[i],&str[i+1],len-i);
        }
    }
}

/**
 * Fonction permettant de récupérer le nickname de la commande : NICK
 *
 * @param chaine
 * @param nick
 * @return
 */
int getNick(char* chaine, char* nick) {

    char *mot = strtok(chaine, " ");

    int i = 0;
    int firstCommandOk = 0;
    int isNick = 0;

    while (mot) {
        //si la première commande est un NICK sans espace avant ou autre, c'est bon
        if (strcmp(mot, "NICK") == 0) {
            firstCommandOk = 1;
        }
        //on set le nick,
        if ((i == 1) && (firstCommandOk == 1)){
            remove_char_from_string(mot);
            strcpy(nick, mot);
            isNick = 1;
        }
        i++;
        //printf("%2d %s\n", i++, mot);
        mot = strtok(NULL, " ");
    }
    return isNick;
}
/**
 * Fonction permettant de récupérer le nickname de la commande : USER
 * @param chaine
 * @param username
 * @return
 */
int getUserName(char* chaine, char* username) {

    char *mot = strtok(chaine, " ");

    int i = 0;
    int firstCommandOk = 0;
    int isUserName = 0;

    while (mot) {
        //si la première commande est un USER sans espace avant ou autre, c'est bon
        if (strcmp(mot, "USER") == 0) {
            firstCommandOk = 1;
        }
        //on set le nick,
        if ((i == 1) && (firstCommandOk == 1)){
            strcpy(username, mot);
            remove_char_from_string(username);
            isUserName = 1;
        }
        i++;
        mot = strtok(NULL, " ");
    }
    return isUserName;
}


/**
 * Fonction permettant de savoir si un client a saisi une commande irc
 */
int searchCommandIRC(char* chaine, char* needle)
{
    char *c = strstr(chaine, needle);
    int position = c - chaine;

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
    char usersNick[maxClients][20];
    char usersName[maxClients][100];


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
                    sprintf((char*) usersName[i], "User_%d", usersCount);

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

                    printf("\033[1;31m%s disconnected !\033[0m", usersNick[i] );

                    close(sd);
                    clientSocket[i] = 0;

                } else {
                    printf("\033[1;33mRéception de la commande : \033[0m %s", buffer );

                    //Recherche des commandes IRC
                    if (searchCommandIRC(buffer, "NICK") == 1) {
                        char nick[20];

                        getNick(buffer, nick);

                        printf("L'utilisateur %s devient : -%s-", usersNick[i], nick);
                        
                        sprintf((char*) usersNick[i], nick);

                    }
                    //Recherche des commandes IRC
                    else if (searchCommandIRC(buffer, "USER") == 1) {
                        char userName[100];
                        getUserName(buffer, userName);

                        printf("L'utilisateur %s- devient : %s", usersNick[i], userName);
                        sprintf((char*) usersName[i], userName, usersCount);
                    }
                    //cas non gérés pour le moment
                    else {

                    }

                    // Transfert aux autres clients
                    /*
                    suppression pour cause de test avec un client IRC
                    strcpy(bufferRetour, usersNick[i]);
                    strcat(bufferRetour, " : ");
                    strcat(bufferRetour, buffer);
                    printf("%s", bufferRetour);
                    */


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
