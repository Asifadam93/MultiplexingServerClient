#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "get_nick"

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
            strcpy(nick, mot);
            isNick = 1;
        }
        i++;
        //printf("%2d %s\n", i++, mot);
        mot = strtok(NULL, " ");
    }
    return isNick;
}


int main() {

    /////////////// TEST 1
    /*char test[80], blah[80];
    char *sep = "\\/:;=-";
    char *word, *phrase, *brkt, *brkb;

    strcpy(test, "This;is.a:test:of=the/string\\tokenizer-function.");

    for (word = strtok_r(test, sep, &brkt); word; word = strtok_r(NULL, sep, &brkt)) {
        strcpy(blah, "blah:blat:blab:blag");

        for (phrase = strtok_r(blah, sep, &brkb); phrase; phrase = strtok_r(NULL, sep, &brkb)) {
            printf("So far we're at %s:%s\n", word, phrase);
        }
    }*/

    /////////////// TEST 2
    /*char buffer[1024];

    strcpy(buffer, "Ceci est un test");


    char *mot = strtok(buffer, " ");

    int i = 0;

    printf("cpt  mot\n");

    printf("==========\n");

    while (mot) {
        printf("%2d %s\n", i++, mot);
        mot = strtok(NULL, " ");
    }*/

    /////////////// TEST 3


    /*char *mots;
    mots = malloc(sizeof(char));
    strcpy(mots, "coucou coucou coucou");
       */

    /*
    //test pas concluant
    char *mots[5];
    mots[0] = malloc(strlen("testblablabla") + 1);
    strcpy(mots[0], "test");

    printf("Mot : -%s-\n", mots[0]);

    if (strcmp(mots[0], "test") == 0) {
        printf("ok\n");

    }
    */


    /*
    char buffer[1024];

    strcpy(buffer, "Ceci est un test");


    char *mot = strtok(buffer, " ");
    int i = 0;

    printf("cpt  mot\n");

    printf("==========\n");

    while (mot) {
        printf("%2d %s\n", i++, mot);
        mot = strtok(NULL, " ");
    }
     */
    char buffer[1024];
    char nick[25];
    strcpy(buffer, "NICK fabien");

    if (getNick(buffer, nick) == 1) {
        printf("NICK : %s", nick);
    }

}