#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "player_pool.h"
#include "record_file.h"

void printPlayerPool(struct player_pool *pool){
    struct Player *index = pool->head;   
    printf("\nprinting player pool...");
    while(index != NULL){
        printf("\n");
        printf("Player ID: %d\n", index->playerID);
        printf("Username: %s\n", index->username);
        printf("Wins: %d\n", index->wins);
        printf("Losses: %d\n", index->losses);
        index = index->next;
    }
}

void player_pool_init(struct player_pool **result) {
    (*result) = (struct player_pool *) malloc(sizeof (struct player_pool));
    (*result)->head = NULL;
    (*result)->last = NULL;
    (*result)->size = 0;
    (*result)->nextId = 0; // this is what will be assigned to players
}

struct Player *addNewPlayerToPool(struct player_pool *pool, int player_socket, char *username) {
    struct Player *new_player = (struct Player *) malloc(sizeof (struct Player));
    char playerIDMessage;
    
    new_player->player_socket = player_socket;
    new_player->losses = 0;
    new_player->wins = 0;
    new_player->playerID = pool->nextId; // assign unique player id
    printf("This is the Player ID assigned: %d\n", new_player->playerID);
    pool->nextId++;
    new_player->username = username;
    new_player->next = NULL;
    
    if (pool->size == 0) {
        pool->head = new_player;
        pool->last = new_player;
        pool->size++;
    } else {
        pool->last->next = new_player;
        pool->last = new_player;
        pool->size++;
    }

    playerIDMessage = new_player->playerID;
    send(player_socket, &playerIDMessage, 1, 0);
    
    return new_player;
}

struct Player *nextPlayer(struct player_pool *pool) {
    struct Player *result = pool->head;

    pool->head = pool->head->next;
    pool->size--;
    return result;
}

void freePool(struct player_pool *pool) {
    struct Player *node = pool->head;
    struct Player *temp;
    int i;
    for (i = 0; i < pool->size; i++) {
        temp = node;
        node = node->next;
        close(temp->player_socket);
        free(temp);
    }

    free(pool);
}

void appendPlayer(struct player_pool *pool, struct Player *player) {
    if (pool->size == 0) {
        pool->head = player;
        pool->last = player;
        pool->size++;
    } else {
        pool->last->next = player;
        pool->last = player;
        pool->size++;
    }
}

struct Player *findRecordByPlayerId(struct player_pool *pool, int playerID){
    struct Player *result = pool->head;
    
    while(result != NULL){
        if(result->playerID == playerID) break;
        result = result->next;
    }
    return result;
}

void removeRecordByPlayerId(struct player_pool *pool, int playerID){
    struct Player *index = pool->head;
    struct Player *temp;
    if(pool->size == 1){
        if(pool->head->playerID == playerID){
            pool->head = pool->head->next;
            pool->last = pool->head;
            pool->size--;
        }
    }
    else{
        if(pool->head->playerID == playerID){
            pool->head = pool->head->next;
            pool->size--;
        }
        while(index->next != NULL){
            if(index->next->playerID == playerID){
                temp = index->next;
                index->next = index->next->next;
                pool->size--;
                
                if(index->next == NULL){
                    pool->last = index;
                }
                return;
            }
            index = index->next;
        }
    }
}

void updateWins(struct player_pool *storage, struct Player *player){
    struct Player *index = storage->head;
    int found = 0;
    if (index == NULL) {
        printf("Head is null\n");
        return;
    }
    while (index != NULL) {
        if (index->playerID == player->playerID) {
            index->wins++;
            player->wins++;
            break;
        }
        index = index->next;
    }
}

void updateLosses(struct player_pool *storage, struct Player *player){
    struct Player *index = storage->head;
    int found = 0;
    if (index == NULL) {
        printf("Head is null\n");
        return;
    }
    while (index != NULL) {
        if (index->playerID == player->playerID) {
            index->losses++;
            player->losses++;
            break;
        }
        index = index->next;
    }
}

struct Player *copyPlayerRecord(struct Player *originalNode) {
    struct Player *newPlayer = (struct Player *) malloc(sizeof (struct Player));
    newPlayer->playerID = originalNode->playerID;
    newPlayer->username = originalNode->username;
    newPlayer->player_socket = originalNode->player_socket;
    newPlayer->wins = originalNode->wins;
    newPlayer->losses = originalNode->losses;
    newPlayer->next = NULL;
    return newPlayer;
}

void storeRecordInFile(struct Player *player){
    int file;
    int error = 0;
    struct Player *record;
    file = open("records.bin", O_RDWR);
    do{
        printf("Hi from storeRecordinFile\n");
        error = readRecord(file, (void *)record, sizeof(struct Player));
        if(error){
            record = NULL;
            break;
        }
    } while(record->playerID != player->playerID);
    
    if(record == NULL){
        close(file);
        file = open("records.bin", O_APPEND | O_WRONLY);
        writeRecord(file, (void *)player, sizeof(struct Player));
        printf("Wrote username: %s, player id: %d\n", player->username, player->playerID);
    }
    else{
        lseek(file, -(sizeof(struct Player)), SEEK_CUR);
        error = writeRecord(file, (void *)player, sizeof(struct Player));
        printf("Wrote player id: %d\n", player->playerID);
    }
    close(file);
}

struct Player *retrieveRecordFromFile(int playerID){
    struct Player *record = malloc(sizeof(struct Player));
    char id[2];
    id[1] = '\0';
    id[0] = (char)playerID;
    int file = 0;
    int error = 0;
    
    file = open(id, O_RDONLY);
    error = readRecord(file, (void *)record, sizeof(struct Player));  
    close(file);
    return record;
}