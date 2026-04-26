# Wilo Chat
C'est un réseau social open source, et ce repo contient surtout le backend écrit en C.

En fait, ce backend sert uniquement de relai internet afin d'acheminer les messages à travers les NAT des routeurs.
Il permet également de garder en mémoire les messages destinés à des utilisateurs non connectés.

## Stack
Pour la stack, j'ai choisi quelque chose de plutôt classique.
1. Mongoose pour le réseau
2. Sqlite3
3. Les libraries standards C

> Assurez-vous d'avoir `libsqlite3-dev` installé sur votre système, le nom du paquet peut différer en fonction de la distribution Linux sur laquelle vous êtes.

### Compilation
```Bash
make
./wlch
```

## Fonctionnement
Le fonctionnement de Wilo Chat est un peu spécial car c'est un réseau social côté utilisateur, le serveur ne se charge donc pas de la sécurité et ne fait que relayer des messages.
On ne sait pas si un utilisateur existe vraiment, mais s'il dit qu'il existe, c'est que c'est le cas non ?

> Tout le protocole fonctionne en WebSockets, mais le format de messages peut également fonctionner en Bluetooth, LAN, ou encore en LoRa.

### Envoi d'un message
Pour envoyer un message, il suffit de donner quelques informations.
1. Le **TYPE** de message
2. Le **FROM** du message
3. Le **TO** du message
4. Le **HASH** de celui qui envoie
5. La **SIGN**, la signature base64 du contenu du message
6. La **LEN**, la longueur du contenu du message
7. Le **CONTENT**, le contenu du message

Le **SIGN** sert à vérifier que l'entiereté du message est passé, mais il est facultatif.

Le **HASH** est la première brique du système de sécurité de BTMP. C'est la clé publique qui permet de déchiffrer le **CONTENT**, lui-même encrypté via une clé privée. Chaque utilisateurs à donc deux clé cryptographique, et celle qui est privée ne doit JAMAIS quitter la machine de son utilisateur sauf s'il le fait lui même.

Ça donne donc ceci :
```Text
TYPE;FROM;TO;HASH;SIGN;LEN;CONTENT
```
Le `;` sert à séparer les éléments, il est très important.

> ATTENTION: le serveur ne vérifie pas que le message est correct, il ne fait que relayer, la manière dont s'organise un message fait parti du protocole BTMP (Brute Text Message Protocol).

### Dire qu'on est connecté
Afin que le serveur connaisse notre existence, il est primordial de lui signaler notre existence à chaque fois qu'on se connecte.

Pour ce faire, on envoie :
1. **TYPE**
2. **NAME**
3. **HASH**

Le **NAME** est le **TO** utilisé pour envoyer des messages.
Le format est donc le suivant: 
```Text
TYPE;NAME;HASH
```

Le serveur envoie ensuite **OK** avec un challenge à résoudre.
C'est donc une réponse :
```Text
TYPE;NUMBER
```
Le **TYPE** étant **OK** et le **NUMBER** le nombre à encrypté avec notre clé privée avant de renvoyer sous un autre **OK** le résultat.
Le serveur essaye ensuite de décrypter via la clé publique, et si ça ne donne pas le nombre de départ, c'est que la personne tente d'usurper l'identité de quelqu'un d'autre donc on bloque.


*wilo, btw*
