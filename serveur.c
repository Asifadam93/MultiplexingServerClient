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
#define PORT 7777 //Parce que "Tous les 7 portent bonheur"
#define MAX_CLIENTS 20
#define DEBUG 1

//déclaration des variables
int clientSocket[MAX_CLIENTS],  //listes des sockets
    masterSocket,               //socket principal
    newSocket,                  //nouveau socket (quand il y a une nouvelle connexion)
    sd,                         //variable temporaire pour les sockets
    maxSd,                      //socket le plus haut (nécessaire pour les selects)
    activity,                   //variable permettant de savoir si on a eu une activité sur notre select
    addressLength,              //taille du client connecté
    nbChar,                     //nombre de carractère lu avec la commande read
    fdsAdded = 0,               //permet de savoir si on a ajouté un socket à clientSocket
    usersCount = 0;             //un conteur d'utilisateur connecté,

char buffer[255];               //buffer de lecture des entrées de notre serveur
char bufferRetour[255];         //buffer destiné à répondre au clients

struct sockaddr_in address;     //socket en entrée
fd_set readfds;                 //la liste des sockets à surveiller

//information sur les utilisateurs connectés (une structure serait plus adapté //TODO)
char usersNick[MAX_CLIENTS][20];//nick irc de l'utilisateur
char usersName[MAX_CLIENTS][40];//username irc de l'utilisateur
int inChannel[MAX_CLIENTS];     //permet de savoir si un utilisateur est connecté ou non


/**
 * Fonction utilitaire : permet de supprimer les \n et les \r
 * @param str
 */
void clean(char *str)
{
    //printf("AVANT : \"%s\"\n", str);fflush(stdout);

    char bufferTempo[100];
    memset(bufferTempo,0,100);

    int j=0;
    int len = strlen(str)+1;

    for(int i=0; i<len; i++) {
        if ((str[i] != '\n') && (str[i] != '\r'))  {
            //printf("%i : %c (%i)\n", i, str[i], (int) str[i]);
            bufferTempo[j] = str[i];
            j++;
        }
    }
    str[j] ='\0';
    strcpy(str, bufferTempo);
    //printf("APRES : \"%s\"\n", str);fflush(stdout);

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
 * //TODO très similaire à getNick cette fonction pourrait être factorisé avec getNick
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
            clean(mot);
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

    if ((c != NULL) && (position == 0)) {
        printf("\033[0;36mFound command %s\033[0m\n", needle);
        return 1;
    }
    return 0;
}

/**
 * Fonction utilisée pour accueillir un utilisateur
 * @param socket
 * @param nick
 */
void welcome(int socket, char* nick) {
    sprintf((char*) bufferRetour,":localhost 001 %s :Welcome to the Groupe 10 4AMOC1 ESGI 2016-2017 Internet Relay Chat Network %s\r\n", nick, nick);
    write(socket,bufferRetour,strlen(bufferRetour));

    sprintf((char*) bufferRetour,":localhost 002 %s :Your host is localhost[127.0.0.1/%i], running version 1.0\r\n", nick, PORT );
    write(socket,bufferRetour,strlen(bufferRetour));

    sprintf((char*) bufferRetour,":localhost 003 %s :This server was created Sat Jan 7 2017 at 12:04:42 EST\r\n", nick);
    write(socket,bufferRetour,strlen(bufferRetour));
}


/**
 * Fonction utilisée pour lister les utilisateur d'un channel
 * @param socket
 * @param newUser
 */
void listing(int socket, int newUser) {
    //message de listing des utilisateurs
    sprintf((char*) bufferRetour,":localhost 353 %s = #welcome :", usersNick[newUser]);

    for (int i =0; i<MAX_CLIENTS; i++) {
        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket)) {
            strcat(bufferRetour, usersNick[i]);
            strcat(bufferRetour, " ");
        }
    }

    strcat(bufferRetour, "\r\n");
    write(socket,bufferRetour,strlen(bufferRetour));

    //printf ("serveur => %s %s", usersNick[newUser], bufferRetour); fflush(stdout);

    //message de fin de listing
    sprintf((char*) bufferRetour,":localhost 366 %s #welcome :End of /NAMES list\r\n", usersNick[newUser]);
    write(socket,bufferRetour,strlen(bufferRetour));


    //printf ("serveur => %s %s", usersNick[newUser], bufferRetour); fflush(stdout);


}

void informNewUserConnected(newUser)
{
    for (int i =0; i<MAX_CLIENTS; i++) {
        //envoi aux utilisateurs déjà connectés qu'un nouvel utilisateur est arrivé
        sprintf((char*) bufferRetour,":%s!%s@locahost JOIN #welcome\r\n", usersNick[newUser], usersNick[newUser]);

        //:tifa!tifa@locahost JOIN #welcome
        //:blabla!BLABLA@locahost JOIN #welcome
        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket) && (i != newUser)) {
            printf ("serveur => %s (%i) %s", usersNick[i], clientSocket[i], bufferRetour); fflush(stdout);

            write(clientSocket[i],bufferRetour,strlen(bufferRetour));
            write(clientSocket[i],"\r\n",strlen("\r\n"));
        }
    }
}

/**
 * Méthode utilisée pour rejoindre un channel (avec envoie des message d'accueil et co)
 * @param socket
 * @param newUser
 */
void join(int socket, int newUser)
{

    //message de bienvenue du channel étape 1
    sprintf((char *) bufferRetour, ":%s!~%s@localhost JOIN #welcome\r\n", usersNick[newUser], usersName[newUser]);
    write(socket, bufferRetour, strlen(bufferRetour));

    //message de bienvenue du channel étape 2
    sprintf((char *) bufferRetour, ":localhost 332 %s #welcome :Official Welcome channel\r\n", usersNick[newUser]);
    write(socket, bufferRetour, strlen(bufferRetour));

    listing(socket, newUser);
    informNewUserConnected(newUser);

}

/**
 * Fonction permettant à un utilisateur de quitter le channel et de prévenir les autres utilisateurs
 * @param oldUSER
 */
void part(int oldUSER)
{
    for (int i =0; i<MAX_CLIENTS; i++) {
        //message aux utilisateurs déjà connectés qu'un utilisateur vient de partir
        sprintf((char*) bufferRetour,":%s!%s@locahost PART #welcome\r\n", usersNick[oldUSER], usersNick[oldUSER]);

        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket)) {
            printf ("serveur => %s (%i) %s", usersNick[i], clientSocket[i], bufferRetour); fflush(stdout);

            write(clientSocket[i],bufferRetour,strlen(bufferRetour));
            write(clientSocket[i],"\r\n",strlen("\r\n"));
        }
    }
    inChannel[oldUSER] = 0;
}

/**
 * Fonction permettant à un utilisateur de quitter le serveur et de préveni les autres utilisateurs
 * @param oldUser
 */
void quit(int oldUser)
{

    for (int i =0; i<MAX_CLIENTS; i++) {
        //message aux utilisateurs déjà connectés qu'un utilisateur vient de partir
        sprintf((char*) bufferRetour,":%s!%s@locahost QUIT\r\n", usersNick[oldUser], usersNick[oldUser]);

        if ((clientSocket[i] != 0)  && (clientSocket[i] != masterSocket)  && (i != oldUser)) {
            printf ("serveur => %s (%i) %s", usersNick[i], clientSocket[i], bufferRetour); fflush(stdout);

            write(clientSocket[i],bufferRetour,strlen(bufferRetour));
            write(clientSocket[i],"\r\n",strlen("\r\n"));
        }
    }

    printf("\033[1;31m%s disconnected !\033[0m", usersNick[oldUser] );

    close(clientSocket[oldUser]);
    clientSocket[oldUser] = 0;
    strcpy(usersName[oldUser], "");
    strcpy(usersNick[oldUser], "");
    inChannel[oldUser] = 0;


}

/**
 * permet de vérifier que le channel que tente de rejoindre un utilisateur est correct ou non
 * @param chaine
 * @param user
 * @return
 */
int checkChannelName(char* chaine, int user) {
    char channel[100];
    char *mot = strtok(chaine, " ");

    char bufferLecture[512];
    memset(bufferLecture, 0, strlen(bufferLecture));

    int i = 0;
    int firstCommandOk = 0;

    while (mot) {
        //si la première commande est un USER sans espace avant ou autre, c'est bon
        if ((strcmp(mot, "JOIN") == 0) || (strcmp(mot, "PART") == 0)) {
            firstCommandOk = 1;
        }
        //on set le channel,
        if ((i == 1) && (firstCommandOk == 1)){
            strcpy(channel, mot);
            break;
        }

        i++;
        mot = strtok(NULL, " ");
    }

    clean(channel);

    if (strcmp(channel, "#welcome") == 0) {
        printf("OK on essaye de joindre le channel welcome\n");
        return 1;
    } else {
        printf("ERREUR : \"%s\" n'existe pas\n", channel);

        sprintf((char*) bufferRetour,":localhost 403 %s %s :Ce channel n'existe pas\r\n", usersNick[user], channel);
        write(clientSocket[user], bufferRetour,strlen(bufferRetour));
    }
    fflush(stdout);
    return 0;
}


/**
 * Fonction permettant de transmettre les messages privées ou sur le channel
 * @param chaine
 * @param expediteur
 */
void transfertMessage(char* chaine, int expediteur) {

    char destinataire[100];
    char *mot = strtok(chaine, " ");

    char bufferLecture[512];
    memset(bufferLecture, 0, strlen(bufferLecture));

    int i = 0;
    int firstCommandOk = 0;
    int hasDestinataire = 0;
    int addDoublePoint = 0;

    while (mot) {
        //si la première commande est un USER sans espace avant ou autre, c'est bon
        if (strcmp(mot, "PRIVMSG") == 0) {
            firstCommandOk = 1;
        }
        //on set le nick,
        if ((i == 1) && (firstCommandOk == 1)){
            strcpy(destinataire, mot);
            hasDestinataire = 1;
        }
        if ( i > 1) {
            strcat(bufferLecture, mot);
            strcat(bufferLecture, " ");
        }

        i++;
        mot = strtok(NULL, " ");
    }
    clean(bufferLecture);
    clean(destinataire);
    printf("J'ai lu %s à destination de %s\n",bufferLecture, destinataire );fflush(stdout);

    if (bufferLecture[0] != ':') {
        addDoublePoint = 1;
    }


    for (int j =0; j<MAX_CLIENTS; j++) {
        if ((destinataire[0] == '#') || (strcmp(destinataire, usersNick[j]) == 0) ) {
            //envoi du message
            if (addDoublePoint == 0) {
                sprintf((char*) bufferRetour,":%s!%s@localhost PRIVMSG %s %s \r\n", usersNick[expediteur], usersNick[expediteur], destinataire, bufferLecture);
            } else {
                sprintf((char*) bufferRetour,":%s!%s@localhost PRIVMSG %s :%s \r\n", usersNick[expediteur], usersNick[expediteur], destinataire, bufferLecture);
            }

            if ((clientSocket[j] != 0)  && (clientSocket[j] != masterSocket) && (j != expediteur)) {

                printf ("serveur => %s (%i) %s", usersNick[j], clientSocket[j], bufferRetour); fflush(stdout);

                write(clientSocket[j],bufferRetour,strlen(bufferRetour));
                write(clientSocket[j],"\r\n",strlen("\r\n"));
            }
        }
    }


}







/**
 * Programme principal
 * @return
 */
int main(){

    struct timeval tempo;
    tempo.tv_sec = 1;
    tempo.tv_usec = 0;

    //initialisation de tout les clients
    for(int i=0; i<MAX_CLIENTS; i++){
        clientSocket[i] = 0;
        inChannel[i] = 0;
    }


    //création d'un socket en ipv4 (af_inet) et en TCP (SOCK_STREAM)
    if((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("\033[1;31mMaster socket creation error \033[0m");
        exit(EXIT_FAILURE);
    }

    //nettoyage des addresse
    bzero(&address, sizeof(address));

    // configuration du socket adress (AF_INET : ipv4, INADDR_ANY : toute les interfaces)
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind masterSocket
    if(bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("\033[1;31mMaster socket binding error \033[0m");
        exit(EXIT_FAILURE);
    }

    // écoute de MAX_CLIENTS connexion(20 normallement) et du master
    if(listen(masterSocket, MAX_CLIENTS+1) < 0){
        perror("\033[1;31mlisten error \033[1m");
        exit(EXIT_FAILURE);
    }

    //on accepte les nouvelles connexions
    addressLength = sizeof(address);
    printf("\033[1;33mWaiting for connections \033[0m\n");

    //lancement du serveur
    while(1){
        //nettoyage des addresse en lecture
        FD_ZERO(&readfds);

        // ajout du masterSocket
        FD_SET(masterSocket, &readfds);
        maxSd = masterSocket;

        //ajout des sockets de chaque client
        for(int i=0; i<MAX_CLIENTS; i++){

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

        //Attente d'une activitée sur un des sockets que l'on écoute
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

            //affichage de la nouvelle connexion
            printf("\033[1;32mNew connection asked - socket fd : \033[0m %d \n", newSocket);

            //ajout des socket destinés aux clients lorsqu'il y en a
            fdsAdded = -1;
            for(int i = 0; i<MAX_CLIENTS; i++){

                //si la socket est à 0, alors il n'y a pas personne de connecté
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

            //On affiche le résultat de l'ajout
            if (fdsAdded>= 0) {
                printf("\033[1;32m  -> Added \033[0m \033[1;37m%s\033[0m\n", usersNick[fdsAdded] );
            } else {
                printf("\033[1;31m  -> Error : Can't add more socket !\033[0m \n");
            }
        }

        // Si il y a une action sur un des clients
        for (int i = 0; i < MAX_CLIENTS; i++){

            //récupération du socket en cours
            sd = clientSocket[i];

            //si on a une activitée sur le socket en lecture
            if(FD_ISSET(sd, &readfds)){

                //Si l'utilisateur se déconnecte sans passer par la commande QUIT
                if((nbChar = read(sd, buffer, 1024)) == 0){
                    quit(i);
                }
                //Réceptions des commandes IRC
                else {
                    printf("\033[1;33mRéception de la commande : \033[0m %s", buffer );

                    //gestion de la commande IRC qui permet à un utilisateur de déclarer son nick (NICK)
                    if (searchCommandIRC(buffer, "NICK") == 1) {
                        char nick[20];

                        getNick(buffer, nick);

                        printf("L'utilisateur %s devient : %s\n", usersNick[i], nick);
                        fflush(stdout);
                        sprintf((char*) usersNick[i],"%s", nick);

                    }
                    //gestion de la commande IRC qui permet à un utilisateur de décarer son username (USER)
                    else if (searchCommandIRC(buffer, "USER") == 1) {
                        char userName[100];


                        getUserName(buffer, userName);


                        printf("L'username de %s (%s) devient %s\n", usersNick[i], usersName[i], userName);
                        fflush(stdout);
                        sprintf((char *) usersName[i], "%s", userName);

                        welcome(clientSocket[i], usersNick[i]);
                        join(clientSocket[i], i);
                    }
                    //gestion de la commande IRC qui permet d'envoyer des messages aussi bien privé que sur un channel (PRIVMSG)
                    else if (searchCommandIRC(buffer, "PRIVMSG") == 1) {
                        transfertMessage(buffer, i);
                    }
                    //gestion de la commande IRC qui permet de rejoindre un channel (JOIN)
                    else if (searchCommandIRC(buffer, "JOIN") == 1) {
                        if (checkChannelName(buffer, i) == 1) {
                            join(clientSocket[i], i);
                        }
                    }
                    //gestion de la commande IRC qui permet de quitter un channel (PART)
                    else if (searchCommandIRC(buffer, "PART") == 1) {
                        if (checkChannelName(buffer, i) == 1) {
                            part(i);
                        }
                    }//gestion de la commande IRC qui permet de quitter le serveur (QUIT)
                    else if (searchCommandIRC(buffer, "QUIT") == 1) {
                        quit(i);
                    }
                    //débugage (envois de toute les commandes recu par le serveur à tout les clients connectés)
                    //
                    else if (DEBUG == 1){
                        for (int r = 0; r < MAX_CLIENTS; r++){

                            if ((r != i) && (clientSocket[r] > 0))  {
                                printf ("serveur => %s (%i) %s", usersNick[i], clientSocket[r], buffer); fflush(stdout);
                                write(clientSocket[r],buffer,strlen(buffer));
                            }
                        }
                    }

                    //nettoyage de la mémoire des bufer pour éviter les contenus incohérent
                    memset(buffer, 0,sizeof(buffer));
                    memset(buffer, 0,sizeof(bufferRetour));
                }
            }
        }
    }
    return 0;
}
