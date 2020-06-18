#include "client.h"

void setAdditionalInfo(struct addrinfo *additionalInfo) {
    memset(additionalInfo, 0, sizeof *additionalInfo);
    additionalInfo->ai_family = AF_INET;
    additionalInfo->ai_socktype = SOCK_STREAM;
}

void getServerInfoList(char *hostname, struct addrinfo **result) {
    int status;
    struct addrinfo additionalInfo;

    setAdditionalInfo(&additionalInfo);

    if ((status = getaddrinfo(hostname, PORT, &additionalInfo, result)) != 0) {
        fprintf(stderr, "getaddrInfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

int connectToSocket(struct addrinfo *serverInfoList) {
    int ai_family;
    int socket_FileDescriptor = -1;

    struct addrinfo *s = serverInfoList;
    while (s != NULL) {
        if ((socket_FileDescriptor =
                socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1) {
            perror("Client: socket failed");
        } else if (connect(socket_FileDescriptor, s->ai_addr, s->ai_addrlen) == -1) {
            close(socket_FileDescriptor);
            perror("Client: connect failed");
        } else
            break;

        s = s->ai_next;
    }

    if (s == NULL) {
        fprintf(stderr, "Client: failed to connect.");
        exit(1);
    }

    ai_family = s->ai_family;
    printAcceptedConnection((struct sockaddr *) s->ai_addr, ai_family);
    return socket_FileDescriptor;
}

void printAcceptedConnection(struct sockaddr *socket_addr, int ai_family) {
    char playerIPv6[INET6_ADDRSTRLEN];

    inet_ntop(socket_addr->sa_family,
            getInetVersionSpecificAddress((struct sockaddr *) socket_addr),
            playerIPv6, sizeof playerIPv6);
    printf("CLient: got connection from %s\n", playerIPv6);
}

void *getInetVersionSpecificAddress(struct sockaddr *player_addr) {
    if (player_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) player_addr)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) player_addr)->sin6_addr);
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

void print_shotboard(char **board) {
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

void sendshipdata(int ss, char *data) {
    send(ss, data, 3, 0);
}

void senddata(int ss, char *data) {
    send(ss, data, 2, 0);
}

void update_hit(char **board, int x, int y, char *message) {
    board[y][x] = 'X';
    printf("%s", message);
}

void update_miss(char **board, int x, int y, char *message) {
    board[y][x] = 'O';
    printf("%s", message);
}

void recieve_and_update(int server_socket, char **board, char **shotboard, int playerid, char *length) {
    char hit_message[100];
    int numOfBytes;

    numOfBytes = recv(server_socket, hit_message, length[0], 0);
    hit_message[numOfBytes] = '\0';

    if (hit_message[0] == 1) { // hit
        if (hit_message[3] == playerid) { // my board was hit               
            update_hit(board, hit_message[1], hit_message[2], &hit_message[4]);
        } else { // my opponents board was hit
            update_hit(shotboard, hit_message[1], hit_message[2], &hit_message[4]);
        }
    } else { // miss
        if (hit_message[3] == playerid) { // my opponent missed
            update_miss(board, hit_message[1], hit_message[2], &hit_message[4]);
        } else { // I missed
            update_miss(shotboard, hit_message[1], hit_message[2], &hit_message[4]);
        }
    }

}

void sendshipcoords(int ss) {
    int x, y, r;

    printf("\nEnter X position: ");
    scanf("%d", &x);
    printf("\nEnter Y position: ");
    scanf("%d", &y);
    printf("\nEnter Orientation (0 for v, 1 for ): ");
    scanf("%d", &r);

    char tosend[3];
    tosend[0] = x;
    tosend[1] = y;
    tosend[2] = r;
    sendshipdata(ss, tosend);
}

void sendcoords(int ss) {
    int x, y;
    printf("Enter your next move! (x, y): ");
    scanf("%d%d", &x, &y);
    char tosend[2];
    tosend[0] = (char) x;
    tosend[1] = (char) y;
    senddata(ss, tosend);
}

void dogameover(int winner, int server_socket) {
    char buffer[100];
    char length, wins, losses;
    int numOfBytes;
    printf("\nWinner is Player %d: ", winner);
    numOfBytes = recv(server_socket, &length, 1, 0);
    numOfBytes = recv(server_socket, buffer, (int)length, 0);
    buffer[numOfBytes] = '\0';
    printf("%s\n", buffer);
    
    recv(server_socket, &length, 1, 0);
    numOfBytes = recv(server_socket, buffer, (int)length, 0);
    buffer[numOfBytes] = '\0';
    printf("\n%s\n", buffer);
    recv(server_socket, &wins, 1, 0);
    printf("Wins: %d\n", (int)wins);
    recv(server_socket, &losses, 1, 0);
    printf("Losses: %d\n",(int)losses);
    
    recv(server_socket, &length, 1, 0);
    numOfBytes = recv(server_socket, buffer, (int)length, 0);
    buffer[numOfBytes] = '\0';
    printf("\n%s\n", buffer);
    recv(server_socket, &wins, 1, 0);
    printf("Wins: %d\n", (int)wins);
    recv(server_socket, &losses, 1, 0);
    printf("Losses: %d\n",(int)losses);
}

void place_ship(char *name, int server_socket, char **board) {
    char message[1];
    int numOfBytes;
    int i;
    printf("Placing %s: ", name);
    do {
        sendshipcoords(server_socket);
        recv(server_socket, message, 1, 0);
    } while (message[0] == INVALID_STATUS);
    for (i = 0; i < GAME_SIZE; i++) {
        numOfBytes = recv(server_socket, board[i], GAME_SIZE, 0);
    }
    print_shipboard(board);
}

char **createShotBoard() {
    char **shotboard;
    shotboard = (char**) malloc(GAME_SIZE * sizeof (char*));
    for (int i = 0; i < GAME_SIZE; i++) {
        shotboard[i] = (char*) malloc(GAME_SIZE * sizeof (char));
    }
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++) {
            shotboard[i][j] = '*';
        }
    }
    return shotboard;
}

void registerUsername(int server_socket) {
    char username[15];
    char password[10];
    char length[1];
    char status, playerID;

    do {
        printf("Enter a username:\n");
        scanf("%s", username);
        printf("Enter password:\n");
        scanf("%s", password);

        length[0] = strlen(username);
        send(server_socket, length, 1, 0);
        send(server_socket, username, (int) length[0], 0);
        length[0] = strlen(password);
        send(server_socket, length, 1, 0);
        send(server_socket, password, (int) length[0], 0);

        recv(server_socket, &status, 1, 0);
    } while (status == INVALID_STATUS);
    recv(server_socket, &playerID, 1, 0);

    printf("Your player ID is: %d\n", (int) playerID);
}

void signIn(int server_socket) {
    char password[10];
    char length[1];
    int playerIDTemp;
    char status, playerID;

    do {
        printf("Enter your player ID: \n");
        scanf("%d", &playerIDTemp);
        playerID = playerIDTemp;
        printf("Enter password:\n");
        scanf("%s", password);
    
        send(server_socket, &playerID, 1, 0);
        length[0] = 9;
        send(server_socket, length, 1, 0);
        send(server_socket, password, (int) length[0], 0);

        recv(server_socket, &status, 1, 0);
    } while (status == INVALID_STATUS);
    printf("Sign in successful\n");
}

char chooseMode(int server_socket) {
    char input;
    char absorbNewLine;
    while (input != 'r' && input != 'f') {
        printf("\nPress 'f' to play a game with a friend or press 'r' for random:\n");
        scanf("%c", &absorbNewLine);
        scanf("%c", &input);
        if (input == 'f' || input == 'r') {
            send(server_socket, &input, 1, 0);
        } else {
            printf("invalid input\n");
        }
    }
    return input;
}

void findOpponent(int server_socket) {
    int input;
    int numOfBytes = 0;
    int acceptStatus = ACCEPT_STATUS;
    int declineStatus = DECLINE_STATUS;
    char response;
    char username[30];
    char playerID, size, listLength, status, accept, absorbNewLine;

    // get the list
    numOfBytes = recv(server_socket, &listLength, 1, 0);
    printf("players waiting for challenge:\n");
    for (int i = 0; i < (int) listLength; i++) {
        numOfBytes = recv(server_socket, &size, 1, 0);
        numOfBytes = recv(server_socket, username, (int) size, 0);
        username[numOfBytes] = '\0';
        printf("%s ", username);
        numOfBytes = recv(server_socket, &playerID, 1, 0);
        printf("%d\n", (int)playerID);

    }
    while (1) {
        printf("\nenter the player ID of the person you want to challenge or enter -1 to wait for a challenge:\n");
        scanf("%d", &input);
        char message = (char) input;
        send(server_socket, &message, 1, 0);
        if (input == -1) {
            //waiting for a challenge 
            while(1){
            numOfBytes = recv(server_socket, &status, 1, 0);
            if ((int) status == CHALLENGE_STATUS) {
                // receive the username and playerID of the challenger
                numOfBytes = recv(server_socket, &size, 1, 0);
                numOfBytes = recv(server_socket, username, size, 0);
                username[numOfBytes] = '\0';
                numOfBytes = recv(server_socket, &playerID, 1, 0);
                printf("you have been challenged by %s %d. Do you accept (y/n):\n", username, (int)playerID);
                scanf("%c", &absorbNewLine);
                scanf("%c", &accept);
                while (accept != 'y' && accept != 'n') {
                    printf("invalid input. Enter y to accept or n to decline.\n");
                    scanf("%c", &absorbNewLine);
                    scanf("%c", &accept);
                }
                if (accept == 'y') {
                    send(server_socket, &acceptStatus, 1, 0);
                    return;
                } else {
                    send(server_socket, &declineStatus, 1, 0);
                }
            }
           }
        } else {
            //challenging a player
            numOfBytes = recv(server_socket, &response, 1, 0);
            printf("status = %d \n", (int)response);
            if ((int) response == ACCEPT_STATUS) {
                printf("opponent accepted\n");
                break;
            } else {
                printf("opponent declined\n");
            }
        }
    }
}