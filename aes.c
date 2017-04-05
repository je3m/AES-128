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

//constants for generating the key schedule
#define KEY_SIZE 16

#define ROUND_KEY_BITS (4+rounds*4)*4 

void parseFile(char* filename);
void generateKeySchedule();
void print4xN(uint8_t* schedule, uint8_t N);
void subBytes();
void addRoundKey(uint32_t roundNumber);
void shiftRows();

static uint32_t iterations;
static uint32_t rounds;
static uint8_t key[KEY_SIZE];
static uint8_t currentState[KEY_SIZE];
static uint32_t* keySchedule;


int main(int argc, char const *argv[]) {
  parseFile((char *)argv[1]);

  printf("%d iterations\n", iterations);
  printf("%d rounds\n", rounds);

  for (int i = 0; i < 16; i++){
    printf("%02x ", key[i]);
  }
  printf(" key\n");
  for (int i = 0; i < 16; i++){
    printf("%02x ", currentState[i]);
  }
  printf(" plaintext\n\n");

  generateKeySchedule();

  addRoundKey(0);
  printf("Initial addRoundKey\n");
  print4xN(currentState, 4);

  subBytes();

  printf("Initial SubBytes\n");
  print4xN(currentState, 4);

  printf("shift rows:\n");
  shiftRows();
  print4xN(currentState, 4);

  return 0;
}

void parseFile(char* fileName) {
  FILE* input = fopen(fileName, "r");
  fscanf(input, "%d%d", &iterations, &rounds);

  for (uint8_t i = 0; i < 16; i++){
    fscanf(input, "%2hhx", &key[i]);
  }

  for (int i = 0; i < 16; i++) {
    fscanf(input, "%2hhx", &currentState[i]);
  }
}

/**
 * This function rotates a 32-bit integer 
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

  printf("ROUND KEY SCHEDULE: \n");
  print4xN((uint8_t*) keySchedule, ROUND_KEY_BITS/4);
}

void subBytes() {
  for (uint8_t i = 0; i < KEY_SIZE; i++) {
    currentState[i] = sbox[currentState[i]];
  }
}


void print4xN(uint8_t* buf, uint8_t N) {

  for (int i = 0; i < 4; i++){
    for (int j = 0; j < N; j++) {
      printf("%02x ", buf[4*j+i]);
    }
    printf("\n");
  }
}

void addRoundKey(uint32_t roundNumber) {
  for (int i = 0; i < KEY_SIZE; i++) {
    uint8_t* keyScheduleBytes = (uint8_t*) keySchedule;
    currentState[i] ^= keyScheduleBytes[KEY_SIZE*roundNumber+i];
  }
}

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