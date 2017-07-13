# MultiplexingServerClient

### Exemple de communication avec le serveur

Tout d'abord il faudra depuis son client IRC se connecté au serveur avec la commande suivante : /server 127.0.0.1 8087

#### Connexion au serveur

Serveur | Client | Commentaire
--- | ---  | ---
 | NICK fabien  | Déclaration du pseudo (doit être unique)
 | USER PetitGrigri 0 * :Fabien Griselles  | Déclaration du nom d'utilisateur et le nom réel (0 * indique le hostname et le servername)
:localhost 001 fabien :Welcome to the Groupe 10 4AMOC1 ESGI 2016-2017 Internet Relay Chat Network fab | | information sur le réseau IRC utilisé
:localhost 002 fabien :Your host is localhost[127.0.0.1/8087], running version 1.0 | | information sur le server host
:localhost 003 fabien :This server was created Sat Jan 7 2017 at 12:04:42 EST | | information sur la date de création du serveur


#### Connexion à un channel

 Serveur | Client | Commentaire
 --- | ---  | ---
 |  JOIN #welcome | Permet au client de demander l'accès à un channel (#welcome)
:fabien!~PetitGrigri@localhost JOIN #welcome | | Permet d'indiquer que l'utilisateur à rejoint le channel #Welcome
:localhost 332 fabien #welcome :Official Welcome channel | | Permet d'indiquer le topic du channel #welcome
:localhost 353 fabien = #welcome :pierre paul jacques | | Permet d'indiquer la liste des utilisateurs présents sur le channel #welcome
:localhost 366 fabien #welcome :End of /NAMES list. | |  Permet d'indiquer que le serveur à terminé le listing des utilisateurs

 #### Communication sur un channel

 Serveur | Client | Commentaire
 --- | ---  | ---
 | PRIVMSG #welcome :Bonjour :) | Message du client sur le channel #welcome
:pierre!pierreTest@localhost PRIVMSG #welcome :Salut mec ! | | Message d'un autre utilisateur sur le channel #welcome (pierre)
:paul!paulTest@localhost PRIVMSG #welcome :Yo ! | | Un autre message sur le channel #welcome (paul)

#### Message privé à un utilisateur
Serveur | Client | Commentaire
--- | ---  | ---
|PRIVMSG paul :Salut paul :)|Message privé du client à un autre client (paul)
:paul!paulTest@localhost PRIVMSG fabien :Yo mec comment ca va ?  | | Message de Paul à fabien

#### Départ du channel
Serveur | Client | Commentaire
--- | ---  | ---
:pierre!PierreTest@localhost PART #welcome| | Message permet d'indiquer qu'un utilisateur a quitté le chanel #welcome
| PART #welcome | Message pour indiquer que le client quitte le channel
:fabien!PetitGrigri@localhost PART #welcome | | Message permettant d'indiquer que le client vient de quitter le channel
