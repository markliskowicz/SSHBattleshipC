// server side game logic
// Author: Mark Liskowicz
// date of last modifcation: 11/8/17

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "battleship.h"

// init variables and allocate the arrays
void init_game(Game *ptr){
   Game *game = ptr;
   game->cstatus = 0;
   game->bstatus = 0;
   game->dstatus = 0;
   game->sstatus = 0;
   game->pstatus = 0;
   game->shipBoard = (char**)malloc(GAME_SIZE * sizeof(char*));
   for(int i = 0; i<GAME_SIZE; i++){
      game->shipBoard[i] = (char*)malloc(GAME_SIZE * sizeof(char));
   }
   for(int i = 0; i<GAME_SIZE; i++){
      for(int j = 0; j<GAME_SIZE; j++){
         game->shipBoard[i][j] = '~';
      }
   }
}

// return 0 if the shot is invalid, 1 if valid
int validMove(int y, int x, Game *ptr){
   Game *game = ptr;
   if(y < GAME_SIZE && x < GAME_SIZE && game->shipBoard[y][x] != 'X'){
      return 1;
   } 
   else {
      return 0;
   }
}

// if there was a miss, returns '~', if there was a hit returns the character that 
// repesents it on the shipboard (C B D S P)
char check_shot(int ycoord, int xcoord, Game *ptr){
   Game *game = ptr;
   char shipChar = game->shipBoard[ycoord][xcoord];
   char result; 
   if(shipChar == '~'){
      result = '~';
   } 
   else if(shipChar == 'C'){
      game->cstatus++;
      result = 'C';
   } 
   else if(shipChar == 'B'){
      game->bstatus++;
      result = 'B';
   } 
   else if(shipChar == 'D'){
      game->dstatus++;
      result = 'C';
   } 
   else if(shipChar == 'S'){
      game->sstatus++;
      result = 'S';
   }
   else if(shipChar == 'P'){
      game->pstatus++;
      result = 'P';
   }
   game->shipBoard[ycoord][xcoord] = 'X';
   return result;
}

// returns 1 if the ship in question is sunk, 0 if not
int check_sunk(char shipChar, Game *ptr){
   Game *game = ptr;
   int result = 0;
   if(shipChar == 'C'){
      if(game->cstatus == CSIZE){
         result = 1;
      }
   }
   else if(shipChar == 'B'){
      if(game->bstatus == BSIZE){
         result = 1;
      }
   }
   else if(shipChar == 'D'){
      if(game->dstatus == DSIZE){
         result = 1;
      }
   }
   else if(shipChar == 'S'){
      if(game->sstatus == SSIZE){
         result = 1;
      }
   }
   else if(shipChar == 'P'){
      if(game->pstatus == PSIZE){
         result = 1;
      }
   }
   return result;
}

int place_carrier(Game *ptr, int xcoord, int ycoord, int orientation){
//ensure the placement is valid. in bounds, not on top of a ship
   Game *game = ptr;
   int displacement = 0;
   if(orientation == 0){
      if((xcoord < GAME_SIZE) && ((ycoord + CSIZE) < GAME_SIZE)){
         while(displacement < CSIZE){
            if(game->shipBoard[ycoord + displacement][xcoord] != '~'){
               return 0;
            } 
            else {
               displacement++;
            }
         }
      } 
      else {
         return 0;
      }
      displacement = 0;
      while(displacement < CSIZE){
         game->shipBoard[ycoord + displacement][xcoord] = 'C';
         displacement++;
      }
   } 
   else {
      if(orientation == 1){
         if(((xcoord + CSIZE) < GAME_SIZE) && ((ycoord) < GAME_SIZE)){
            while(displacement < BSIZE){
               if(game->shipBoard[ycoord][xcoord + displacement] != '~'){
                  return 0;
               } 
               else {
                  displacement++;
               }
            }
         } 
         else {
            return 0;
         }
         displacement = 0;
         while(displacement < CSIZE){
            game->shipBoard[ycoord][xcoord + displacement] = 'C';
            displacement++;
         }
      } 
      else {
         return 0;
      }
   }
   return 1;
}

// returns 0 if unable to place
int place_battleship(Game *ptr, int xcoord, int ycoord, int orientation){
   //ensure the placement is valid. in bounds, not on top of a ship
   Game *game = ptr;
   int displacement = 0;
   if(orientation == 0){
      if((xcoord < GAME_SIZE) && ((ycoord + BSIZE) < GAME_SIZE)){
         while(displacement < BSIZE){
            if(game->shipBoard[ycoord + displacement][xcoord] != '~'){
               return 0;
            } 
            else {
               displacement++;
            }
         }
      } 
      else {
         return 0;
      }
      displacement = 0;
      while(displacement < BSIZE){
         game->shipBoard[ycoord + displacement][xcoord] = 'B';
         displacement++;
      }
   } 
   else {
      if(orientation == 1){
         if(((xcoord + BSIZE) < GAME_SIZE) && ((ycoord) < GAME_SIZE)){
            while(displacement < BSIZE){
               if(game->shipBoard[ycoord][xcoord + displacement] != '~'){
                  return 0;
               } 
               else {
                  displacement++;
               }
            }
         } 
         else {
            return 0;
         }
         displacement = 0;
         while(displacement < BSIZE){
            game->shipBoard[ycoord][xcoord + displacement] = 'B';
            displacement++;
         }
      } 
      else {
         return 0;
      }
   }
   return 1;
}

int place_destroyer(Game *ptr, int xcoord, int ycoord, int orientation){
 //ensure the placement is valid. in bounds, not on top of a ship
   Game *game = ptr;
   int displacement = 0;
   if(orientation == 0){
      if((xcoord < GAME_SIZE) && ((ycoord + DSIZE) < GAME_SIZE)){
         while(displacement < DSIZE){
            if(game->shipBoard[ycoord + displacement][xcoord] != '~'){
               return 0;
            } 
            else {
               displacement++;
            }
         }
      } 
      else {
         return 0;
      }
      displacement = 0;
      while(displacement < DSIZE){
         game->shipBoard[ycoord + displacement][xcoord] = 'D';
         displacement++;
      }
   } 
   else {
      if(orientation == 1){
         if(((xcoord + DSIZE) < GAME_SIZE) && ((ycoord) < GAME_SIZE)){
            while(displacement < DSIZE){
               if(game->shipBoard[ycoord][xcoord + displacement] != '~'){
                  return 0;
               } 
               else {
                  displacement++;
               }
            }
         } 
         else {
            return 0;
         }
         displacement = 0;
         while(displacement < DSIZE){
            game->shipBoard[ycoord][xcoord + displacement] = 'D';
            displacement++;
         }
      } 
      else {
         return 0;
      }
   }
   return 1;
}

int place_submarine(Game *ptr, int xcoord, int ycoord, int orientation){
   //ensure the placement is valid. in bounds, not on top of a ship
   Game *game = ptr;
   int displacement = 0;
   if(orientation == 0){
      if((xcoord < GAME_SIZE) && ((ycoord + SSIZE) < GAME_SIZE)){
         while(displacement < SSIZE){
            if(game->shipBoard[ycoord + displacement][xcoord] != '~'){
               return 0;
            } 
            else {
               displacement++;
            }
         }
      } 
      else {
         return 0;
      }
      displacement = 0;
      while(displacement < SSIZE){
         game->shipBoard[ycoord + displacement][xcoord] = 'S';
         displacement++;
      }
   } 
   else {
      if(orientation == 1){
         if(((xcoord + SSIZE) < GAME_SIZE) && ((ycoord) < GAME_SIZE)){
            while(displacement < SSIZE){
               if(game->shipBoard[ycoord][xcoord + displacement] != '~'){
                  return 0;
               } 
               else {
                  displacement++;
               }
            }
         } 
         else {
            return 0;
         }
         displacement = 0;
         while(displacement < SSIZE){
            game->shipBoard[ycoord][xcoord + displacement] = 'S';
            displacement++;
         }
      } 
      else {
         return 0;
      }
   }
   return 1;
}

int place_patrol(Game *ptr, int xcoord, int ycoord, int orientation){
 //ensure the placement is valid. in bounds, not on top of a ship
   Game *game = ptr;
   int displacement = 0;
   if(orientation == 0){
      if((xcoord < GAME_SIZE) && ((ycoord + PSIZE) < GAME_SIZE)){
         while(displacement < PSIZE){
            if(game->shipBoard[ycoord + displacement][xcoord] != '~'){
               return 0;
            } 
            else {
               displacement++;
            }
         }
      } 
      else {
         return 0;
      }
      displacement = 0;
      while(displacement < PSIZE){
         game->shipBoard[ycoord + displacement][xcoord] = 'P';
         displacement++;
      }
   } 
   else {
      if(orientation == 1){
         if(((xcoord + PSIZE) < GAME_SIZE) && ((ycoord) < GAME_SIZE)){
            while(displacement < PSIZE){
               if(game->shipBoard[ycoord][xcoord + displacement] != '~'){
                  return 0;
               } 
               else {
                  displacement++;
               }
            }
         } 
         else {
            return 0;
         }
         displacement = 0;
         while(displacement < PSIZE){
            game->shipBoard[ycoord][xcoord + displacement] = 'P';
            displacement++;
         }
      } 
      else {
         return 0;
      }
   }
   return 1;
}

// returns 0 if game is active
// returns 1 if there is a winner 
int game_over(Game *player){
   int result = 0;
   if((player->cstatus == CSIZE) && (player->bstatus == BSIZE) && (player->dstatus == DSIZE) 
   && (player->sstatus == SSIZE) && (player->pstatus == PSIZE)){
      result = 1;
   }
   return result;
}
// returns 2 for player 2 win, 1 for player 1 win
int declare_winner(Game *ptr1, Game *ptr2){
   Game *player1 = ptr1;
   Game *player2 = ptr2;
   int result = 0;
   if((player1->cstatus == CSIZE) && (player1->bstatus == BSIZE) && (player1->dstatus == DSIZE) 
   && (player1->sstatus == SSIZE) && (player1->pstatus == PSIZE)){
      result = 2;
   }
   if((player2->cstatus == CSIZE) && (player2->bstatus == BSIZE) && (player2->dstatus == DSIZE) 
   && (player2->sstatus == SSIZE) && (player2->pstatus == PSIZE)){
      result = 1;
   }
   return result;
}

