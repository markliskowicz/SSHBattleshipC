/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   record_file.h
 * Author: Tyler Mulvihill
 *
 * Created on November 22, 2017, 9:46 PM
 */

#ifndef RECORD_FILE_H
#define RECORD_FILE_H

int readRecord(int file, void *record, int nbytes);
int writeRecord(int file, void *record, int nbytes);

#endif /* RECORD_FILE_H */
