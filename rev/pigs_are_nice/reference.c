#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct KeyValuePair {
  char key;
  const char *value;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Needs exactly 1 argument.\n");
    return 1;
  }

  struct KeyValuePair lookup[] = {
      {'A', "aaaaa"}, {'B', "aaaab"}, {'C', "aaaba"}, {'D', "aaabb"},
      {'E', "aabaa"}, {'F', "aabab"}, {'G', "aabba"}, {'H', "aabbb"},
      {'I', "abaaa"}, {'J', "abaab"}, {'K', "ababa"}, {'L', "ababb"},
      {'M', "abbaa"}, {'N', "abbab"}, {'O', "abbba"}, {'P', "abbbb"},
      {'Q', "baaaa"}, {'R', "baaab"}, {'S', "baaba"}, {'T', "baabb"},
      {'U', "babaa"}, {'V', "babab"}, {'W', "babba"}, {'X', "babbb"},
      {'Y', "bbaaa"}, {'Z', "bbaab"}};

  const char *encoded =
      "aaaab aaaaa aaaba abbba abbab abaaa baaba aaaba baaab abaaa abbaa aabaa";
  const char *input_string = argv[1];

  char decipher[256] = "";
  size_t i = 0;
  while (1) {
    if (i < strlen(encoded) - 4) {
      char substr[6];
      strncpy(substr, encoded + i, 5);
      substr[5] = '\0';

      if (substr[0] != ' ') {
        for (size_t j = 0; j < sizeof(lookup) / sizeof(lookup[0]); j++) {
          if (strcmp(lookup[j].value, substr) == 0) {
            decipher[strlen(decipher)] = lookup[j].key;
            break;
          }
        }
        i += 5;
      } else {
        decipher[strlen(decipher)] = ' ';
        i += 1;
      }
    } else {
      break;
    }
  }

  char he[256] = "";

  size_t c = 0;
  for (size_t j = 0; j < strlen(decipher); j += 2) {
    he[c] = toupper(decipher[j]);
    c++;
  }
  he[c] = '\0';

  if (strcmp(he, input_string) == 0) {
    printf("Valid, paradox_ctf{%s}\n", input_string);
  } else {
    printf("Invalid\n");
  }

  return 0;
}
