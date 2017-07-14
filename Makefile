serveur: serveur.o
	gcc -o serveur serveur.o


serveur.o: serveur.c
	gcc -o serveur.o -c serveur.c
