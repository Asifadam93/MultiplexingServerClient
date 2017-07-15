all: serveur client

serveur: serveur.o
	gcc serveur.c -o serveur

client: client.o
	gcc client.c -o client