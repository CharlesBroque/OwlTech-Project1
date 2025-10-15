# CS 3503 Project 1 - BitBoard Checkers Game

## Author
Dylan K.

## Description
Project for CS 3503. It's checkers-like (capture not compulsory, no multiple captures, but otherwise the same), and uses bitboards (unsigned long longs) under the hood.

## Build Instructions
If you know how to use the makefile you can use that. But I don't, and you really don't have to. #include takes care of it.
```Windows PowerShell
gcc -o checkers checkersgame.c
.\checkers.exe
```
Make sure you're not running it in a protected folder; the game supports saving and loading. Saved games (savegame.txt) double as lists of moves. Editing saves manually may cause unexpected behavior!

## Help (also available in the main menu)
```
In Bitboard Checkers, moves are entered as pairs of integers ranging 0 to 63.
The first integer is the origin square (to identify the piece) and the second is the destination.
Typically this looks like incrementing by 7 or 9, though not all such increments are valid. Backward moves decrement the index similarly. Captures by analogy use differences of 14 or 18.

To identify specific squares, see the board. Rows begin on multiples of 8 and count up to the right.
To find the index of a square, add the value of the row and column.
56 -> ##..##..##..##..
48 -> ..##..##..##..##
40 -> ##..##..##..##..
32 -> ..##..##..##..##
24 -> ##..##..##..##..
16 -> ..##..##..##..##
08 -> ##..##..##..##..
00 -> ..##..##..##..##
       ^ ^ ^ ^ ^ ^ ^ ^
 +     0 1 2 3 4 5 6 7

Pieces are represented by letters. 'bb' for black and 'rr' for red. Kings are capitalized (BB or RR) and allowed to move backward.
All valid moves are diagonal. Captures travel double the distance of an ordinary move and remove the opposing piece in the middle.
The capturing move from black in this position looks like "18 36". The noncapturing move looks like "18 25".
56 -> ##..##..##..##..
48 -> ..##..##..##..##
40 -> ##..##..##..##..
32 -> ..##..##..##..##
24 -> ##..##rr##..##..
16 -> ..##bb##..##..##
08 -> ##..##..##..##..
00 -> ..##..##..##..##
       ^ ^ ^ ^ ^ ^ ^ ^
 +     0 1 2 3 4 5 6 7

The game is over when one side has lost all their pieces.
This is all you need to play the game. Happy gaming!
```

## Notes
This project has been an exercise in learning how to use structs. Boy howdy, am I glad I did. Even though they are cumbersome and I still have to fiddle with pointers, it is much preferable to work with something resembling Objects for a project of this scale. As I said before, I don't know how to use a makefile and at this rate I may never know.
