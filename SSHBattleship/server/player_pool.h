#ifndef PLAYER_POOL_H
#define PLAYER_POOL_H

struct Player{
    int player_socket;
    struct Player *next;
    
    int playerID; // assigned by server
    char *username; // this is chosen by player. Like a screen name.
    
    int wins;
    int losses;
};

struct player_pool{
    struct Player *head;
    struct Player *last;
    int nextId; // this is NOT username
    int size;
};

void player_pool_init(struct player_pool **result);

void printPlayerPool(struct player_pool *pool);

struct Player *addNewPlayerToPool(struct player_pool *pool, int player_socket, char *username);
void appendPlayer(struct player_pool *pool, struct Player *player);
struct Player *findRecordByPlayerId(struct player_pool *pool, int playerID);
struct Player *restorePlayerRecord(struct player_pool *storage, int PlayerID);
void removeRecordByPlayerId(struct player_pool *pool, int playerID);
struct Player *nextPlayer(struct player_pool *pool);
struct Player *copyPlayerRecord(struct Player *originalNode);
void updateWins(struct player_pool *storage, struct Player *player);
void updateLosses(struct player_pool *storage, struct Player *player);
void storeRecordInFile(struct Player *player);
struct Player *retrieveRecordFromFile(int playerID);
void freePool(struct player_pool *players);
#endif /* PLAYER_POOL_H */
