/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   record_file.c
 * Author: Tyler Mulvihill
 * 
 * Created on November 22, 2017, 9:47 PM
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "record_file.h"
#include "player_pool.h"

int readRecord(int file, void *record, int nbytes){
    int bytes_read;
    int result = 0;
    
    while(nbytes){
        bytes_read = read(file, record, nbytes);
        if(bytes_read == 0){
            result = 1;
            break;
        }
        if(bytes_read < 0){
            result = 1;
            break;
        }
        nbytes -= bytes_read;
        record += bytes_read;
    }
    return result;
}

int writeRecord(int file, void *record, int nbytes){
    int bytes_written;
    int result = 0;
    
    while(nbytes){
        bytes_written = write(file, record, nbytes);
        if(bytes_written <= 0){
            printf("Error writing to file\n");
            result = 1;
            break;
        }
        nbytes -= bytes_written;
        record += bytes_written;
    }
    return result;
}