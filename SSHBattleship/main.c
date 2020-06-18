/*  Program:      Client
    Authors:      Tyler Mulvihill, Mark Liskowicz, Aaron Deak
    Filename:     main.c
    Compile:      cc client.c main.c -o client_main
    Run:          ./client_main
    Last updated: November 10, 2017
  
    A game client program that allows a user to connect to a game server and 
    engage another player in a game of battleships.
 */

#include "client.h" 

int main(int argc, char** argv) {
    char playerid;
    char status[1];
    char **board;
    char **shotboard;
    int server_socket, i;
    struct addrinfo *serverInfoList;
    int numOfBytes;
    char status_message[2];
    char access;

    if (argc != 2) {
        printf("Usage: client hostname");
        exit(1);
    }

    getServerInfoList(argv[1], &serverInfoList);
    server_socket = connectToSocket(serverInfoList);
    freeaddrinfo(serverInfoList);
    
    printf("Press r to register and s to sign in: ");
    scanf("%c", &access);
    
    if(access == 'r'){
        access = REGISTER_STATUS;
        send(server_socket, &access, 1, 0);
        registerUsername(server_socket);
    }
    else if(access == 's'){
        access = SIGNIN_STATUS;
        send(server_socket, &access, 1, 0);
        signIn(server_socket);
    }
    
    char mode = chooseMode(server_socket);
    if(mode == 'f'){
        findOpponent(server_socket);
        
    }
    
    numOfBytes = recv(server_socket, status_message, 2, 0); // Start status message is 2 bytes
    playerid = status_message[1];

    shotboard = createShotBoard();
    board = (char **) malloc(sizeof (char *)*GAME_SIZE);
    for (i = 0; i < GAME_SIZE; i++) {
        board[i] = (char *) malloc(sizeof (char)*GAME_SIZE);
    }

    place_ship("carrier", server_socket, board);
    place_ship("battleship", server_socket, board);
    place_ship("destroyer", server_socket, board);
    place_ship("submarine", server_socket, board);
    place_ship("patrol", server_socket, board);

    if (playerid == 1) {
        do {
            status[0] = MOVE_STATUS;
            send(server_socket, status, 1, 0);
            sendcoords(server_socket);
            numOfBytes = recv(server_socket, status_message, 2, 0);
            if(status_message[0] == INVALID_STATUS){
                printf("Invalid move. Try again:\n");
            }
        } while(status_message[0] == INVALID_STATUS);
        recieve_and_update(server_socket, board, shotboard, playerid, &status_message[1]);
        print_shotboard(shotboard);
        print_shipboard(board);
    }
    
    while (1) {
        //-------------Listen during opponents turn-------------------------------- 
        numOfBytes = recv(server_socket, status_message, 2, 0);
        
        if (status_message[0] == HIT_STATUS) {
            recieve_and_update(server_socket, board, shotboard, playerid, &status_message[1]);
            print_shotboard(shotboard);
            print_shipboard(board);
        }
        else if (status_message[0] == GAMEOVER_STATUS) {
            dogameover((int)status_message[1], server_socket); // player id 
            close(server_socket);
            exit(0);
        }
        else {
            printf("Something went wrong\n");
            close(server_socket);
            exit(1);
        }
        //--------------My Turn----------------------------------------------
        do {
            status[0] = MOVE_STATUS;
            send(server_socket, status, 1, 0);
            sendcoords(server_socket);
            numOfBytes = recv(server_socket, status_message, 2, 0);
            if(status_message[0] == INVALID_STATUS){
                printf("Invalid move. Try again:\n");
            }
        } while(status_message[0] == INVALID_STATUS);
        
        if (status_message[0] == HIT_STATUS) {
            recieve_and_update(server_socket, board, shotboard, playerid, &status_message[1]);
            print_shotboard(shotboard);
            print_shipboard(board);
        }
        else if (status_message[0] == GAMEOVER_STATUS) {
            
            dogameover(status_message[1], server_socket); // player id 
            close(server_socket);
            exit(0);
        }
        else {
            printf("Something went wrong\n");
            close(server_socket);
            exit(1);
        }
    }

    close(server_socket);

    return 0;
}
