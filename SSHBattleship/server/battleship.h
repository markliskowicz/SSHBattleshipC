// Author: Mark Liskowicz
#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#define GAME_SIZE 10
#define BSIZE 4
#define CSIZE 5
#define DSIZE 3
#define SSIZE 3
#define PSIZE 2


typedef struct GAME{
   char **shipBoard; // board to hold player N's ship placement
// ship statuses track the damage of each ship, if status matches the 
// size then it has been sunk
   int cstatus;
   int bstatus;
   int dstatus;
   int sstatus;
   int pstatus; 
   
   int playerid;
   int player_socket;
}Game;

void init_game(Game *ptr);
char check_shot(int ycoord, int xcoord, Game *ptr);
int check_winner(Game *ptr);
int validMove(int y, int x, Game *ptr);
int place_carrier(Game *game, int xcoord, int ycoord, int orientation);
int place_battleship(Game *game, int xcoord, int ycoord, int orientation);
int place_destroyer(Game *game, int xcoord, int ycoord, int orientation);
int place_submarine(Game *game, int xcoord, int ycoord, int orientation);
int place_patrol(Game *game, int xcoord, int ycoord, int orientation);
int game_over(Game *player);
int declare_winner(Game *ptr1, Game *ptr2);
int check_sunk(char shipChar, Game *ptr);

#endif
