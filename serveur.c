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

#define PORT 8087 //8087 parceque Asif :)
#define MAX_CLIENTS 20

int maxClients = MAX_CLIENTS,
    clientSocket[MAX_CLIENTS],
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
char usersNick[MAX_CLIENTS][20];
char usersName[MAX_CLIENTS][40];



void clean(char *str)
{
    int i=0;
    int len = strlen(str)+1;

    for(i=0; i<len; i++)
    {
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
            clean(mot);
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

void welcome(int socket, char* nick) {
    sprintf((char*) bufferRetour,":localhost 001 %s :Welcome to the Groupe 10 4AMOC1 ESGI 2016-2017 Internet Relay Chat Network %s\r\n", nick, nick);
    write(socket,bufferRetour,strlen(bufferRetour));

    sprintf((char*) bufferRetour,":localhost 002 %s :Your host is localhost[127.0.0.1/%i], running version 1.0\r\n", nick, PORT );
    write(socket,bufferRetour,strlen(bufferRetour));

    sprintf((char*) bufferRetour,":localhost 003 %s :This server was created Sat Jan 7 2017 at 12:04:42 EST\r\n", nick);
    write(socket,bufferRetour,strlen(bufferRetour));
}


void joinWelcome(int socket, char* nick, char* userName)
{
    printf("%s",nick);fflush(stdout);
    printf("%s",userName);fflush(stdout);

    //message de bienvenue du channel étape 1
    sprintf((char*) bufferRetour,":%s!~%s@localhost JOIN #welcome\r\n", nick, userName);
    write(socket,bufferRetour,strlen(bufferRetour));

    //message de bienvenue du channel étape 2
    sprintf((char*) bufferRetour,":localhost 332 %s #welcome :Official Welcome channel\r\n", nick);
    write(socket,bufferRetour,strlen(bufferRetour));

    //message de listing des utilisateurs
    sprintf((char*) bufferRetour,":localhost 353 %s = #welcome :cloud ", nick);

    for (int i =0; i<MAX_CLIENTS; i++) {
        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket)) {
            strcat(bufferRetour, usersNick[i]);
            strcat(bufferRetour, " ");
        }
    }
    strcat(bufferRetour, "\r\n");
    write(socket,bufferRetour,strlen(bufferRetour));

    sprintf((char*) bufferRetour,":localhost 366 %s #welcome :End of /NAMES list\r\n", nick);
    write(socket,bufferRetour,strlen(bufferRetour));


    //envoi aux utilisateurs déjà connectés qu'un nouvel utilisateur est arrivé
    sprintf((char*) bufferRetour,":%s!%s@locahost JOIN #welcome\r\n", nick, nick);

    printf("%s",bufferRetour);fflush(stdout);

    for (int i =0; i<MAX_CLIENTS; i++) {
        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket)) {
            write(socket,bufferRetour,strlen(bufferRetour));
        }
    }







}


int main(){



    struct timeval tempo;
    tempo.tv_sec = 1;
    tempo.tv_usec = 0;

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
            if((newSocket = accept(masterSocket, (struct sockaddr *)&address, ((socklen_t*) &addressLength))) < 0){
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

                    //cast pour éviter un warning.

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

                        printf("L'utilisateur %s devient : %s\n", usersNick[i], nick);
                        fflush(stdout);
                        sprintf((char*) usersNick[i],"%s", nick);

                    }
                        //Recherche des commandes IRC
                    else if (searchCommandIRC(buffer, "USER") == 1) {
                        char userName[100];
                        getUserName(buffer, userName);


                        printf("L'username de  %s est %s\n", usersNick[i], userName);
                        fflush(stdout);
                        sprintf((char *) usersName[i], "%s", userName);

                        welcome(clientSocket[i], usersNick[i]);
                        joinWelcome(clientSocket[i], usersNick[i], usersName[i]);
                    }


                        //cas non gérés pour le moment
                    else {

                    }

                    for (int r = 0; r < maxClients; r++){

                        if ((r != i) && (clientSocket[r] > 0))  {

                            write(clientSocket[r],buffer,strlen(buffer));
                        }
                    }




                    memset(buffer, 0,sizeof(buffer));
                    memset(buffer, 0,sizeof(bufferRetour));



                }

            }
        }




    }

    return 0;

}
