// This is the actual program's code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  FILE *file = fopen("flag.txt", "r");
  if (file == NULL) {
    printf("Failed to open file.\n");
    return 1;
  }

  // Read the content of the file
  char str[256];
  if (fgets(str, sizeof(str), file) == NULL) {
    printf("Failed to read from file.\n");
    fclose(file);
    return 1;
  }
  fclose(file);

  int len = strlen(str);

  int halfLen = len / 2;

  char firstHalf[halfLen + 1];
  char secondHalf[halfLen + 1];
  char xorResult[halfLen + 1];

  // Splitting the string into two equal halves
  strncpy(firstHalf, str, halfLen);
  firstHalf[halfLen] = '\0';
  strncpy(secondHalf, str + halfLen, halfLen);
  secondHalf[halfLen] = '\0';

  // XOR the two halves
  for (int i = 0; i < halfLen; i++) {
    xorResult[i] = firstHalf[i] ^ secondHalf[i];
  }
  xorResult[halfLen] = '\0';

  // Print the result as a hex string
  for (int i = 0; i < halfLen; i++) {
    printf("%02x", (unsigned char)xorResult[i]);
  }
  printf("\n");

  return 0;
}
