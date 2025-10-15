#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// helper functions
void div_convert(unsigned long long n, int base, char *out) { // updated from A1
   // beginning with provided skeleton
   char temp[65]; // buffer
   int pos = 0;

   // zero case
   if (n == 0) {
      strcpy(out, "0");
      return;
   }

   // collect digits in reverse order
   while (n > 0) {
      int remainder = n % base;
      n = n / base;

      // digit to character
      if (remainder < 10)
         temp[pos++] = '0' + remainder;
      else
         temp[pos++] = 'A' + (remainder - 10);
   }
   temp[pos] = '\0'; // saves a lot of headaches later

   strrev(temp); // learned my lesson
   strcpy(out, temp);
}

// bit manipulations
unsigned long long OneBit(unsigned long long board, int position) {
    if (position < 0 || position > 63) {
        printf("yeah that's not right. no such bit at position %d.\n", position);
        return board;
    }
    unsigned long long mask = (unsigned long long) 1 << position;
    return board | mask;
}
unsigned long long ZeroBit(unsigned long long board, int position) {
    if (position < 0 || position > 63) {
        printf("yeah that's not right. no such bit at position %d.\n", position);
        return board;
    }
    unsigned long long mask = ~((unsigned long long) 1 << position);
    return board & mask;
}
unsigned long long FlipBit(unsigned long long board, int position) {
    if (position < 0 || position > 63) {
        printf("yeah that's not right. no such bit at position %d.\n", position);
        return board;
    }
    unsigned long long mask = (unsigned long long) 1 << position;
    return board ^ mask;
}

// gather bits
int GetBit(unsigned long long board, int position) {
    if (position < 0 || position > 63) {
        printf("yeah that's not right. no such bit at position %d.\n", position);
        return board;
    }
    return (board >> position) % 2;
}
int CountBits(unsigned long long board) { // count 1s
    char * myout = malloc(65);
    div_convert(board, 2, myout);
    int count = 0;
    for (int i = 0; i < strlen(myout); i++) {
        if (myout[i] == '1') count++; 
    }
    return count;
}

// what is this even for
unsigned long long ShiftLeft(unsigned long long board, int positions) {
    if (positions < 0) {
        printf("ShiftRight() is thataway buddy.\n");
        return board;
    }
    return board << positions;
}
unsigned long long ShiftRight(unsigned long long board, int positions) {
    if (positions < 0) {
        printf("ShiftLeft() is thataway buddy.\n");
        return board;
    }
    return board >> positions;
}

// print functions
void PrintBinary(unsigned long long board) {
    char * myout = malloc(65);
    div_convert(board, 2, myout);

    char looksgood[] = "00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000";
    int zero_chars = 64 - strlen(myout);
    int spaces = 0; // spaces in string so far
    for (int i = 0; i < 64; i++) {
        if ((i != 0) && (i % 8 == 0)) spaces++;
        if (i < zero_chars) continue;
        // else: start transfering myout into looksgood
        looksgood[i + spaces] = myout[i - zero_chars];
    }
    printf("%s\n", looksgood);
}
void PrintHex(unsigned long long board) { // seems not very useful to be honest
    char * myout = malloc(65);
    div_convert(board, 16, myout);

    char looksgood[] = "00 00 00 00 00 00 00 00";
    int zero_chars = 16 - strlen(myout);
    int spaces = 0; // spaces in string so far
    for (int i = 0; i < 16; i++) {
        if ((i != 0) && (i % 2 == 0)) spaces++;
        if (i < zero_chars) continue;
        // else: start transfering myout into looksgood
        looksgood[i + spaces] = myout[i - zero_chars];
    }
    printf("%s\n", looksgood);
}


