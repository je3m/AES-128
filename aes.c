/* 
 * AES.c
 * Author: Jim Gildersleeve
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcon.h"
#include "sbox.h"
#include "mixColumnTable.h"

//constants for generating the key schedule
#define KEY_SIZE 16

#define ROUND_KEY_BITS (4+rounds*4)*4 

void parseFile(char* filename);
void generateKeySchedule();
void print4xN(uint8_t* schedule, uint8_t N);
void subBytes();
void addRoundKey(uint32_t roundNumber);
void shiftRows();
void mixColumns();

static uint32_t iterations;
static uint32_t rounds;
static uint8_t key[KEY_SIZE];
static uint8_t plaintext[KEY_SIZE];
static uint8_t currentState[KEY_SIZE];
static uint32_t* keySchedule;


int main(int argc, char const *argv[]) {
  parseFile((char *)argv[1]);

  generateKeySchedule();

  for (int i = 0; i < iterations; i++) {
    //each iteration xors previous ciphertext with plaintext
    if (i > 0) {
      for (int j = 0; j < KEY_SIZE; j++){
        currentState[j] ^= plaintext[j];
      }
    }

    //initial round key
    addRoundKey(0);

    for (int i = 1; i < rounds+1; i++) {

      subBytes();

      shiftRows();

      //you do not mix columns on the last round
      if(i < rounds){
        mixColumns();
      }

      addRoundKey(i);
    }

  }

  for(int j = 0; j < 16; j++) {
      printf("%02x", currentState[j]);
  }
  printf("\n");
  return 0;
}

/**
 * This function takes a filename with the number of iterations, rounds,
 * key, and plaintext seperated by new line characters and populates the 
 * corresponding global variables
 */
void parseFile(char* fileName) {
  FILE* input = fopen(fileName, "r");
  if(input == NULL){
     printf("ERROR READING INPUT FILE\n");
    exit(1);
  }
  int match = fscanf(input, "%d%d", &iterations, &rounds);
  if(match <= 0) {
    printf("ERROR READING ITERATIONS OR ROUNDS\n");
    exit(1);
  }

  for (uint8_t i = 0; i < 16; i++){
    match = fscanf(input, "%2hhx", &key[i]);
    if(match <= 0) {
      printf("ERROR READING KEY\n");
      exit(1);
    }
  }

  for (int i = 0; i < 16; i++) {
    match = fscanf(input, "%2hhx", &currentState[i]);
    if(match <= 0) {
      printf("ERROR READING PLAINTEXT\n");
      exit(1);
    }
  }
  memcpy(plaintext, currentState, KEY_SIZE * sizeof(uint8_t));
}

/**
 * This function generates the first column of a new chunk of the round key schedule
 * based on the previous column, the first in the last chunk and the chunk number 
 */
uint32_t getNextKeyColumn(uint32_t input, uint32_t input2, uint8_t rconValue) {
  uint32_t shiftedInput = (input >> 8) | (input << 24); //perform rotate on input

  uint8_t* shiftedInputbuffer = (uint8_t*) &shiftedInput; //lets call it an array of bytes

  for (int i = 0; i < 4; i++) {
    shiftedInputbuffer[i] = sbox[shiftedInputbuffer[i]]; //replace bytes with sbox contents
  }

  shiftedInput ^= input2;

  shiftedInputbuffer[0] ^=  rcon[rconValue];

  return shiftedInput;
}

/**
 * This function is responsible for generating the entire key schedule for encryption
 * and populating the global keySchedule
 */
void generateKeySchedule(){
  uint8_t rconIteration = 1;
  keySchedule = (uint32_t*) malloc(ROUND_KEY_BITS/4*sizeof(uint32_t));

  for (int i = 0; i < 4; i++) {
    memcpy(&keySchedule[i], &key[i*4], sizeof(uint8_t) * 4);
  }

  for (int i = 4; i < ROUND_KEY_BITS/4; i+=4) {
    keySchedule[i] = getNextKeyColumn(keySchedule[i-1], keySchedule[i-4], rconIteration);
    keySchedule[i+1] = keySchedule[i-3] ^ keySchedule[i];
    keySchedule[i+2] = keySchedule[i-2] ^ keySchedule[i+1];
    keySchedule[i+3] = keySchedule[i-1] ^ keySchedule[i+2];
    rconIteration++;
  }
}

/**
 * Replaces each element in the current state with it's corresponding entry
 * in the sbox
 */
void subBytes() {
  for (uint8_t i = 0; i < KEY_SIZE; i++) {
    currentState[i] = sbox[currentState[i]];
  }
}

/**
 * Debug method for printing out any array in a 4xN matrix
 */
void print4xN(uint8_t* buf, uint8_t N) {
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < N; j++) {
      printf("%02x ", buf[4*j+i]);
    }
    printf("\n");
  }
}

/**
 * Xors each entry in the currentState with values in the round key based
 * on the round number
 */
void addRoundKey(uint32_t roundNumber) {
  for (int i = 0; i < KEY_SIZE; i++) {
    uint8_t* keyScheduleBytes = (uint8_t*) keySchedule;
    currentState[i] ^= keyScheduleBytes[KEY_SIZE*roundNumber+i];
  }
}

/**
 * Shifts the second column in the currentState left one
 * Shifts the third column in the currentState left twice
 * Shifts the fourth column in the currentState left three times
 */
void shiftRows() {
  uint8_t tmp, tmp1, tmp2;

  //row 1
  tmp = currentState[1];
  currentState[1] = currentState[5];
  currentState[5] = currentState[9];
  currentState[9] = currentState[13];
  currentState[13] = tmp;

  //row 2
  tmp = currentState[2];
  tmp1 = currentState[6];
  currentState[2] = currentState[10];
  currentState[6] = currentState[14];
  currentState[10] = tmp;
  currentState[14] = tmp1;

  //row3
  tmp = currentState[3];
  tmp1 = currentState[7];
  tmp2 = currentState[11];
  currentState[3] = currentState[15];
  currentState[7] = tmp;
  currentState[11] = tmp1;
  currentState[15] = tmp2;
}

/**
 * Performs a galois matrix multiplication for each column in currentState
 */

void mixColumns() {
  uint8_t base = 0;
  for (int i = 0; i < 4; i++) {
    uint8_t tmp1 = currentState[base];
    uint8_t tmp2 = currentState[base+1];
    uint8_t tmp3 = currentState[base+2];

    currentState[base] = gmul2[currentState[base]] ^ 
                          gmul3[currentState[base+1]] ^
                          currentState[base+2] ^
                          currentState[base+3];

    currentState[base+1] = tmp1^ 
                          gmul2[currentState[base+1]] ^
                          gmul3[currentState[base+2]] ^
                          currentState[base+3];

    currentState[base+2] = tmp1 ^ 
                          tmp2 ^
                          gmul2[currentState[base+2]] ^
                          gmul3[currentState[base+3]];

    currentState[base+3] = gmul3[tmp1] ^ 
                          tmp2 ^
                          tmp3 ^
                          gmul2[currentState[base+3]];
    base += 4;
  }
}