#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "server.h"
#include "player_pool.h"

char PASSWORD[9] = "g!v3U5@n@";

void setAdditionalInfo(struct addrinfo *additionalInfo) {
    memset(additionalInfo, 0, sizeof *additionalInfo);
    additionalInfo->ai_family = AF_INET;
    additionalInfo->ai_socktype = SOCK_STREAM;
    additionalInfo->ai_flags = AI_PASSIVE;
}

void getServerInfoList(struct addrinfo **result) {
    int status;
    struct addrinfo additionalInfo;

    setAdditionalInfo(&additionalInfo);

    if ((status = getaddrinfo(NULL, PORT, &additionalInfo, result)) != 0) {
        fprintf(stderr, "getaddrInfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

int createAndBindReusableSocket(struct addrinfo *serverInfoList) {
    int yes = 1;
    int socket_FileDescriptor = -1;

    struct addrinfo *s = serverInfoList;
    while (s != NULL) {
        if ((socket_FileDescriptor =
                socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1) {
            perror("Server: socket failed");
        } else if (setsockopt(socket_FileDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)
                ) == -1) {
            perror("Server: setsockopt failed");
            exit(1);
        } else if (bind(socket_FileDescriptor, s->ai_addr, s->ai_addrlen) == -1) {
            close(socket_FileDescriptor);
            perror("Server: bind failed");
        } else
            break;

        s = s->ai_next;
    }

    if (s == NULL) {
        fprintf(stderr, "Server: failed to bind.");
        exit(1);
    }
    return socket_FileDescriptor;
}

void listenForConnections(int socket_FileDescriptor) {
    if (listen(socket_FileDescriptor, BACKLOG) == -1) {
        perror("Server: listen failed");
        exit(1);
    }
}

int acceptPlayerConnection(int socket_FileDescriptor) {
    struct sockaddr_storage player_addr;
    socklen_t playerAddrLength;
    int new_FileDescriptor;

    printf("Server: waiting for players to connect...\n");
    playerAddrLength = sizeof player_addr;

    new_FileDescriptor = accept(socket_FileDescriptor,
            (struct sockaddr *) &player_addr,
            &playerAddrLength);

    if (new_FileDescriptor == -1) {
        perror("Server: accept failed");
    } else printAcceptedConnection(player_addr);

    return new_FileDescriptor;
}

void *getInetVersionSpecificAddress(struct sockaddr *player_addr) {
    if (player_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) player_addr)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) player_addr)->sin6_addr);
}

void printAcceptedConnection(struct sockaddr_storage player_addr) {
    char playerIPv6[INET6_ADDRSTRLEN];

    inet_ntop(player_addr.ss_family,
            getInetVersionSpecificAddress((struct sockaddr *) &player_addr),
            playerIPv6, sizeof playerIPv6);
    printf("Server: got connection from %s\n", playerIPv6);
}

void initializeRecordsFile(){
    int file = open("records.bin", O_CREAT, S_IRUSR | S_IWUSR);
    close(file);
}

char *registerPlayer(int player_socket) {
    char *username = malloc(sizeof (char) * 30);
    char password[MAXDATASIZE];
    char length[1];
    int numOfBytes;
    
    recv(player_socket, length, 1, 0);
    numOfBytes = recv(player_socket, username, (int) length[0], 0);
    username[numOfBytes] = '\0';
    recv(player_socket, length, 1, 0);
    numOfBytes = recv(player_socket, password, (int) length[0], 0);
    password[numOfBytes] = '\0';

    if (strcmp(password, PASSWORD) == 0) {
        return username;
    }

    return NULL;
}
struct Player *signinPlayer(int player_socket, struct player_pool *storage) {
    char playerID;
    char password[MAXDATASIZE];
    char length;
    int numOfBytes;

    recv(player_socket, &playerID, 1, 0);
    recv(player_socket, &length, 1, 0);
    numOfBytes = recv(player_socket, password, (int) length, 0);
    password[numOfBytes] = '\0';

    if (strcmp(password, PASSWORD) == 0) {
        return retrieveRecordFromFile(playerID);
    }

    return NULL;
}

void handlePlayerMode(struct Player *player, struct player_pool *random,
        struct player_pool *friendly, struct player_pool *storage){
    struct Player *challenge;
    int numOfBytes;
    char mode[2], status, input, playerID, usernameSize, *username;
    
    numOfBytes = recv(player->player_socket, mode, 1, 0);
    mode[numOfBytes] = '\0';
    
    switch(mode[0]){
        case 'r' :
            printf("random match chosen\n");
            appendPlayer(random, copyPlayerRecord(player));
            break;
        case 'f' :
            printf("Friendly match chosen\n");
            sendFriendlyWaitingList(player->player_socket, friendly);
            recv(player->player_socket, &input, 1, 0);
            if((int)input == -1){
                appendPlayer(friendly, copyPlayerRecord(player));
            }
            else{
                playerID = input;
                challenge = findRecordByPlayerId(friendly, playerID);
                
                status = CHALLENGE_STATUS;
                send(challenge->player_socket, &status, 1, 0);
                
                usernameSize = strlen(player->username);
                send(challenge->player_socket, &usernameSize, 1, 0);
        
                username = player->username;
                send(challenge->player_socket, username, usernameSize, 0);
        
                playerID = player->playerID;
                send(challenge->player_socket, &playerID, 1, 0);
                
                recv(challenge->player_socket, &status, 1, 0);
                send(player->player_socket, &status, 1, 0);
                
                if(status == ACCEPT_STATUS){
                    removeRecordByPlayerId(friendly, challenge->playerID);
                    dispatchPlayersToSubserver(storage, copyPlayerRecord(player), challenge);
                }
            }
            break;
    }
}

void sendFriendlyWaitingList(int player_socket, struct player_pool *friendly){
    int i;
    char listLength = friendly->size;
    char playerID, usernameSize, *username;
    struct Player *player = friendly->head;
    
    send(player_socket, &listLength, 1, 0);
    
    for(i = 0; i<friendly->size; i++){
        usernameSize = strlen(player->username);
        send(player_socket, &usernameSize, 1, 0);
        
        username = player->username;
        send(player_socket, username, usernameSize, 0);
        
        playerID = player->playerID;
        send(player_socket, &playerID, 1, 0);
        
        player = player->next;
    }
}

void dispatchPlayersToSubserver(struct player_pool *storage,
        struct Player *player1, struct Player *player2) {
    pthread_t subServerMain;
    ThreadArgs threadargs;
    threadargs.player1 = player1;
    threadargs.player2 = player2;
    threadargs.storage = storage;

    printf("dispatching...\n");

    pthread_create(&subServerMain, NULL, sub_server_main, (void *) &threadargs);
}

void *sub_server_main(void * argv) {
    pthread_t player1PlaceShips, player2PlaceShips;
    int gameOver = 0;
    Game *player1_GameState = (struct GAME *) malloc(sizeof (struct GAME));
    Game *player2_GameState = (struct GAME *) malloc(sizeof (struct GAME));
    struct Player *player1 = ((ThreadArgs *) argv)->player1;
    struct Player *player2 = ((ThreadArgs *) argv)->player2;
    struct player_pool *storage = ((ThreadArgs *) argv)->storage;
    int player1_socket;
    int player2_socket;

    player1_GameState->player_socket = player1->player_socket;
    player2_GameState->player_socket = player2->player_socket;
    player1_socket = player1_GameState->player_socket;
    player2_socket = player2_GameState->player_socket;
    player1_GameState->playerid = 1;
    player2_GameState->playerid = 2;

    init_game(player1_GameState);
    init_game(player2_GameState);

    printf("sending turn info...");
    sendTurnInfo(player1_socket, player2_socket);
    printf("sent\n");

    printf("player1 place ships:\n");
    place_ships(player1_GameState);
    printf("player2 place ships: \n");
    place_ships(player2_GameState);

    while (1) {
        printf("Player 1's turn!\n");
        playerTurn(player1_socket, player2_socket, player2_GameState, &gameOver);
        if (gameOver) { // player 1 won
            updateWins(storage, player1);
            updateLosses(storage, player2);
            sendWinningUsername(player1, player2_socket, 1);
            break;
        }
        printf("Player2's turn!\n");
        playerTurn(player2_socket, player1_socket, player1_GameState, &gameOver);
        if (gameOver) { // player 2 won
            updateWins(storage, player2);
            updateLosses(storage, player1);
            sendWinningUsername(player2, player1_socket, 2);
            break;
        }
    }
    
    sendPlayersStats(player1, player2);
    sendPlayersStats(player2, player1);
    printPlayersStats(player1, player2);
    
    storeRecordInFile(player1);
    storeRecordInFile(player2);
            
    free(player1);
    free(player2);
    
    printf("Closing connection with player 1.\n");
    close(player1_socket);
    printf("Closing connection with player 2.\n");
    close(player2_socket);
    return NULL;
}

void sendTurnInfo(int player1_socket, int player2_socket) {
    char player1TurnMessage[] = {START_STATUS, 1};
    char player2TurnMessage[] = {START_STATUS, 2};

    send(player1_socket, player1TurnMessage, 2, 0);
    send(player2_socket, player2TurnMessage, 2, 0);
}

void printPlayersStats(struct Player *player1, struct Player *player2) {
    printf("Stats for %s: \n", player1->username);
    printf("Wins: %d\nLosses: %d\n", player1->wins, player1->losses);

    printf("Stats for %s: \n", player2->username);
    printf("Wins: %d\nLosses: %d\n", player2->wins, player2->losses);
}

void sendPlayersStats(struct Player *me, struct Player *opponent){
    int numOfBytes;
    char length = strlen(me->username); 
    char *username = me->username; 
    char wins = me->wins; 
    char losses = me->losses;
    
    send(me->player_socket, &length, 1, 0);
    send(me->player_socket, username, (int)length, 0);
    send(me->player_socket, &wins, 1, 0);
    send(me->player_socket, &losses, 1, 0);
    
    send(opponent->player_socket, &length, 1, 0);
    send(opponent->player_socket, username, (int)length, 0);
    send(opponent->player_socket, &wins, 1, 0);
    send(opponent->player_socket, &losses, 1, 0);
    
    
}

void place_ships(void *argv) {
    char status[1];
    char ships_message[3];
    int numOfBytes, i;
    int isValid = 0;
    Game *gameState = ((Game *) argv);
    int player_socket = gameState->player_socket;

    printf("Player %d: placing carrier\n", gameState->playerid);
    while (!isValid) {
        numOfBytes = recv(player_socket, ships_message, 3, 0);
        isValid = place_carrier(gameState, ships_message[0], ships_message[1], ships_message[2]);
        print_shipboard(gameState->shipBoard);
        if (!isValid) {
            status[0] = INVALID_STATUS;
            send(player_socket, status, 1, 0);
        } else {
            status[0] = SHIP_STATUS;
            send(player_socket, status, 1, 0);
        }
    }
    printf("sending 1 ship board to player %d\n", gameState->playerid);
    for (i = 0; i < GAME_SIZE; i++) {
        send(player_socket, gameState->shipBoard[i], GAME_SIZE, 0);
    }

    printf("Player %d: placing battleship\n", gameState->playerid);
    isValid = 0;
    while (!isValid) {
        numOfBytes = recv(player_socket, ships_message, 3, 0);
        isValid = place_battleship(gameState, ships_message[0], ships_message[1], ships_message[2]);
        print_shipboard(gameState->shipBoard);

        if (!isValid) {
            status[0] = INVALID_STATUS;
            send(player_socket, status, 1, 0);
        } else {
            status[0] = SHIP_STATUS;
            send(player_socket, status, 1, 0);
        }
    }
    printf("sending 2 ship board to player %d\n", gameState->playerid);
    for (i = 0; i < GAME_SIZE; i++) {
        send(player_socket, gameState->shipBoard[i], GAME_SIZE, 0);
    }

    printf("Player %d: placing destroyer\n", gameState->playerid);
    isValid = 0;
    while (!isValid) {
        numOfBytes = recv(player_socket, ships_message, 3, 0);
        isValid = place_destroyer(gameState, ships_message[0], ships_message[1], ships_message[2]);
        print_shipboard(gameState->shipBoard);

        if (!isValid) {
            status[0] = INVALID_STATUS;
            send(player_socket, status, 1, 0);
        } else {
            status[0] = SHIP_STATUS;
            send(player_socket, status, 1, 0);
        }
    }
    printf("sending 3 ship board to player %d\n", gameState->playerid);
    for (i = 0; i < GAME_SIZE; i++) {
        send(player_socket, gameState->shipBoard[i], GAME_SIZE, 0);
    }

    printf("Player %d: placing submarine\n", gameState->playerid);
    isValid = 0;
    while (!isValid) {
        numOfBytes = recv(player_socket, ships_message, 3, 0);
        isValid = place_submarine(gameState, ships_message[0], ships_message[1], ships_message[2]);
        print_shipboard(gameState->shipBoard);

        if (!isValid) {
            status[0] = INVALID_STATUS;
            send(player_socket, status, 1, 0);
        } else {
            status[0] = SHIP_STATUS;
            send(player_socket, status, 1, 0);
        }
    }
    printf("sending 4 ship board to player %d\n", gameState->playerid);
    for (i = 0; i < GAME_SIZE; i++) {
        send(player_socket, gameState->shipBoard[i], GAME_SIZE, 0);
    }

    printf("Player %d: placing patrol\n", gameState->playerid);
    isValid = 0;
    while (!isValid) {
        numOfBytes = recv(player_socket, ships_message, 3, 0);
        isValid = place_patrol(gameState, ships_message[0], ships_message[1], ships_message[2]);
        print_shipboard(gameState->shipBoard);

        if (!isValid) {
            status[0] = INVALID_STATUS;
            send(player_socket, status, 1, 0);
        } else {
            status[0] = SHIP_STATUS;
            send(player_socket, status, 1, 0);
        }
    }
    printf("sending 5 ship board to player %d\n", gameState->playerid);
    for (i = 0; i < GAME_SIZE; i++) {
        send(player_socket, gameState->shipBoard[i], GAME_SIZE, 0);
    }

    printf("Ready to start\n");
    return;
}

void print_shipboard(char **board) {
    for (int k = 0; k < GAME_SIZE; k++) {
        printf(" %d", k);
    }
    printf("\n");
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++) {
            printf(" %c", board[i][j]);
        }
        printf(" %d\n", i);
    }
}

void playerTurn(int turnPlayer, int opponent, Game *playerBoard, int *gameOver) {
    int isValid = 0;
    int x, y;
    char *temp;
    char ship;
    char status_message[2];
    char move_message[2];
    char hit_message[100];
    int numOfBytes;

    while (!isValid) {
        numOfBytes = recv(turnPlayer, status_message, 1, 0);
        if (status_message[0] == MOVE_STATUS) {
            numOfBytes = recv(turnPlayer, move_message, 2, 0);
            x = move_message[0];
            y = move_message[1];
            if (!validMove(x, y, playerBoard)) {
                status_message[0] = INVALID_STATUS;
                status_message[1] = 0; // client expecting two bits
                send(turnPlayer, status_message, 2, 0);
                continue;
            }

            hit_message[1] = x;
            hit_message[2] = y;

            if ((ship = check_shot(x, y, playerBoard)) == '~') {
                hit_message[0] = 0;
                hit_message[3] = playerBoard->playerid;
                hit_message[4] = '\0';
                temp = "It's a miss!\n";
                strcat(&hit_message[4], temp);
            } else {
                hit_message[0] = 1;
                hit_message[3] = playerBoard->playerid;
                if (check_sunk(ship, playerBoard)) {
                    switch (ship) {
                        case 'C':
                            hit_message[4] = '\0';
                            temp = "The carrier has been sunk!\n";
                            strcat(&hit_message[4], temp);
                            break;
                        case 'B':
                            hit_message[4] = '\0';
                            temp = "The battleship has been sunk!\n";
                            strcat(&hit_message[4], temp);
                            break;
                        case 'D':
                            hit_message[4] = '\0';
                            temp = "The destroyer has been sunk!\n";
                            strcat(&hit_message[4], temp);
                            break;
                        case 'S':
                            hit_message[4] = '\0';
                            temp = "The submarine has been sunk!\n";
                            strcat(&hit_message[4], temp);
                            break;
                        case 'P':
                            hit_message[4] = '\0';
                            temp = "The patrol has been sunk!\n";
                            strcat(&hit_message[4], temp);
                            break;
                    }
                } else {
                    hit_message[4] = '\0';
                    temp = "It's a hit!\n";
                    strcat(&hit_message[4], temp);
                }
            }


            if (game_over(playerBoard)) {
                *gameOver = 1;
                break;
            } else {
                status_message[0] = HIT_STATUS;
                status_message[1] = strlen(&hit_message[4]) + 5;
                send(turnPlayer, status_message, 2, 0);
                send(opponent, status_message, 2, 0);
            }
            print_shipboard(playerBoard->shipBoard);
            send(turnPlayer, hit_message, strlen(&hit_message[4]) + 5, 0);
            send(opponent, hit_message, strlen(&hit_message[4]) + 5, 0);
        }
        isValid = 1;
    }
}

void sendWinningUsername(struct Player *winner, int opponent, int turn) {
    char length = strlen(winner->username);
    char status_message[2];
    status_message[0] = GAMEOVER_STATUS;
    status_message[1] = turn;
    send(winner->player_socket, status_message, 2, 0);
    send(opponent, status_message, 2, 0);
                
    send(winner->player_socket, &length, 1, 0);
    send(winner->player_socket, winner->username, length, 0);
    send(opponent, &length, 1, 0);
    send(opponent, winner->username, length, 0);
}