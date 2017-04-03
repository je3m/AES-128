/* 
 * AES.c
 * Author: Jim Gildersleeve
 */
#include <stdint.h>
#include <stdio.h>

void parseFile(char* filename);

typedef struct
{
  uint64_t high;
  uint64_t low;
} uint128_t;

static uint32_t iterations;
static uint32_t rounds;
static uint128_t key;
static uint128_t plaintext;

int main(int argc, char const *argv[]) {
  parseFile((char *)argv[1]);

  printf("%hd iterations\n", iterations);
  printf("%hd rounds\n", rounds);

  printf("%llx", (long long unsigned int) key.high);
  printf("%llx key\n", (long long unsigned int) key.low);

  printf("%llx", (long long unsigned int) plaintext.high);
  printf("%llx plaintext\n", (long long unsigned int) plaintext.low);

  return 0;
}

void parseFile(char* fileName) {
  FILE* input = fopen(fileName, "r");
  int check = fscanf(input, "%d%d%16llx%16llx%16llx%16llx", 
    &iterations,
    &rounds,
    &key.high,
    &key.low,
    &plaintext.high,
    &plaintext.low);
}
