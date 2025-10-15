#include "bitboardlib.c"
#include <stdbool.h>

typedef struct move {
    int from;
    int to;
    bool is_capture;
    int enemy_pos;
} move;

typedef struct moveset {
    struct move moves[64]; // generous upper bound, 12 pieces * 4 directions < 64
} moveset;

typedef struct boardstate {
    unsigned long long black_pieces;
    unsigned long long red_pieces;
    unsigned long long kings;
    int turn; // even -> black's move, odd -> red's move
} boardstate;

typedef struct game {
    struct boardstate current_board;
    struct move history[4096]; // surely this is long enough (MAX_TURNS)
} game;

typedef struct int_array { // wrapper for arrays, since C functions can't return arrays
    int ints[64]; // won't need more than this
} int_array;

typedef struct xy_pair {
    int x;
    int y;
} xy_pair;

xy_pair ToXY(int position) {
    xy_pair xy;
    xy.x = -1; xy.y = -1;
    if (position < 0 || position > 63) {
        printf("something's not right. no such position %d.\n", position);
        return xy;
    }
    // else: valid position
    xy.x = position % 8; xy.y = position >> 3;
    return xy;
}
bool InBounds(xy_pair xy) {
    if (!(xy.x < 0 || xy.y < 0 || xy.x > 7 || xy.y > 7)) return true;
    return false;
}
int ToIndex(xy_pair xy) {
    if (!InBounds(xy)) {
        printf("warning: xy_pair (%d, %d) out of bounds.", xy.x, xy.y);
        return -1;
    }
    return xy.x + xy.y * 8;
}

void FillInvalid(int_array * a) { // default content is 0, but that is a valid position
    for (int i = 0; i < 64; i++) {
        (*a).ints[i] = -1;
    }
}

int_array GetPositionsFromBoard(unsigned long long board) {
    struct int_array positions; // will hold positions
    FillInvalid(&positions); // prepare for later

    int c = 0; // generic counter
    for (int i = 0; i < 64; i++) { // find all positions with pieces in them
        if (board % 2 == 1) { 
            positions.ints[c++] = i;
        }
        board = board >> 1;
    }

    return positions;
}

int_array GenerateNoncaptures(int position, bool is_king) {
    int_array noncaptures; FillInvalid(&noncaptures);
    if (position < 0 || position > 63) return noncaptures; // oops all invalid

    xy_pair xy = ToXY(position); // xy coordinate

    int c = 0; // generic counter

    int j_start = 1;
    if (is_king) j_start = -1; // allow moves to decrement y

    for (int i = -1; i <= 1; i += 2) {
        for (int j = j_start; j <= 1; j += 2) {
            xy_pair xy_ij = {xy.x + i, xy.y + j};
            if (InBounds(xy_ij)) noncaptures.ints[c++] = ToIndex(xy_ij);
        }
    }

    return noncaptures;
}

moveset GetLegalMoves(boardstate myboard) {
    int t = myboard.turn % 2; // whose turn is it anyway
    boardstate bs = myboard; // in an earlier version this used to be a game struct. easier not to change the name.
    int bp_pos[64]; int rp_pos[64];

    memcpy(bp_pos, GetPositionsFromBoard(bs.black_pieces).ints, sizeof(int) * 64);
    memcpy(rp_pos, GetPositionsFromBoard(bs.red_pieces).ints, sizeof(int) * 64);
    unsigned long long occupied = bs.black_pieces | bs.red_pieces;

    moveset possible_moves;
    move m = {-1, -1, false, -1};
    for (int i = 0; i < 64; i++) { // initialize full of garbage so i can detect it later
        possible_moves.moves[i] = m;
    }

    if (t == 0) { // black's turn
        // for each piece, generate noncaptures. if noncapture move is invalid because occupied, check if opposite color, and if it is, generate capture move instead.
        
        int c = 0; // generic counter
        for (int i = 0; i < 64; i++) { // for each piece...
            if (bp_pos[i] == -1) break; // stop when no more pieces
            
            // generate noncaptures
            int to_try[64];
            if (GetBit(bs.black_pieces & bs.kings, bp_pos[i]) == 1) { // black and king
                memcpy(to_try, GenerateNoncaptures(bp_pos[i], true).ints, sizeof(int) * 64);
            }
            else { // black and not king
                memcpy(to_try, GenerateNoncaptures(bp_pos[i], false).ints, sizeof(int) * 64);
            }

            // check if noncapture move is occupied
            for (int j = 0; j < 64; j++) {
                if (to_try[j] == -1) break; // stop when no more noncaptures
                if (GetBit(bs.black_pieces, to_try[j]) == 1) continue; // can't move into yourself
                if (GetBit(bs.red_pieces, to_try[j]) == 1) { // can't move into opponent, but can maybe capture
                    xy_pair you = ToXY(bp_pos[i]);
                    xy_pair enemy = ToXY(to_try[j]);
                    xy_pair potential_capture = {you.x + 2 * (enemy.x - you.x), you.y + 2 * (enemy.y - you.y)};
                    if (!InBounds(potential_capture)) continue; // can't jump out of bounds
                    if (GetBit(occupied, ToIndex(potential_capture)) == 1) continue; // can't jump if jump is blocked
                    // else: capture is possible
                    move m = {bp_pos[i], ToIndex(potential_capture), true, ToIndex(enemy)};
                    possible_moves.moves[c++] = m;
                    continue;
                }
                // else: can move there
                move m = {bp_pos[i], to_try[j], false, -1};
                possible_moves.moves[c++] = m;
            }
        }
    }
    else { // red's turn. almost exactly the same code as above, but... there's a special optimization. see if you can catch it.
        // for each piece, generate noncaptures. if noncapture move is invalid because occupied, check if opposite color, and if it is, generate capture move instead.
        
        int c = 0; // generic counter
        for (int i = 0; i < 64; i++) { // for each piece...
            if (rp_pos[i] == -1) break; // stop when no more pieces
            
            // generate noncaptures "from red's perspective" as if they were black
            int to_try[64];
            if (GetBit(bs.red_pieces & bs.kings, rp_pos[i]) == 1) { // red and king
                memcpy(to_try, GenerateNoncaptures(63 - rp_pos[i], true).ints, sizeof(int) * 64); // (transform)
            }
            else { // red and not king
                memcpy(to_try, GenerateNoncaptures(63 - rp_pos[i], false).ints, sizeof(int) * 64); // (transform)
            }

            // check if noncapture move is occupied. have to untransform above
            /* transform count:
             * to_try: 1
             * if GetBit(etc): 2 === 0 (1 from to_try + 1)
             * you: 0
             * enemy: 2 === 0 (1 from to_try + 1)
             * potential_capture: 0 (inherited from you and enemy)
             * move (capture): 0
             * move (noncapture): 2 === 0 (1 from to_try + 1)
             */
            for (int j = 0; j < 64; j++) {
                if (to_try[j] == -1) break; // stop when no more noncaptures
                if (GetBit(bs.red_pieces, 63 - to_try[j]) == 1) continue; // can't move into yourself (untransform)
                if (GetBit(bs.black_pieces, 63 - to_try[j]) == 1) { // can't move into opponent, but can maybe capture (untransform)
                    xy_pair you = ToXY(rp_pos[i]);
                    xy_pair enemy = ToXY(63 - to_try[j]); // (untransform)
                    xy_pair potential_capture = {you.x + 2 * (enemy.x - you.x), you.y + 2 * (enemy.y - you.y)};
                    if (!InBounds(potential_capture)) continue; // can't jump out of bounds
                    if (GetBit(occupied, ToIndex(potential_capture)) == 1) continue; // can't jump if jump is blocked
                    // else: capture is possible
                    move m = {rp_pos[i], ToIndex(potential_capture), true, ToIndex(enemy)};
                    possible_moves.moves[c++] = m;
                    continue;
                }
                // else: can move there
                move m = {rp_pos[i], 63 - to_try[j], false, -1}; // (untransform)
                possible_moves.moves[c++] = m;
            }
        }
    }
    /* DEBUG
    for (int i = 0; i < 64; i++) {
        move m = possible_moves.moves[i];
        printf("%d, %d, %d, %d\n", m.from, m.to, m.is_capture, m.enemy_pos);
    }
    */
    return possible_moves;
}

bool IsMoveInMoveset(moveset mymoveset, move mymove) {
    for (int i = 0; i < 64; i++) {
        move m = mymoveset.moves[i];
        if (m.from == -1) return false; // if you run out of moves to check return false
        // else: compare mymove to mymoveset.moves[i]
        // how to compare? one thing at a time.
        if (m.from != mymove.from) continue;
        if (m.to != mymove.to) continue;
        if (m.is_capture != mymove.is_capture) continue;
        if (m.enemy_pos != mymove.enemy_pos) continue;
        // else: all four match
        return true;
    }
    // if there were 64 legitimate moves somehow and you failed to match any, return false anyway.
    return false;
}

int CountMoves(moveset mymoveset) { // could be used to determine victor
    for (int i = 0; i < 64; i++) {
        move m = mymoveset.moves[i];
        if (m.from == -1) return i;
    }
    return 64; // this should never happen
}

bool IsLegalMove(boardstate myboard, move mymove) {
    moveset legal_moves = GetLegalMoves(myboard);
    return IsMoveInMoveset(legal_moves, mymove);
}

boardstate MakeMove(boardstate myboard, move mymove) {
    // it is NOT this function's job to make sure the move is legal before making it.
    // it just makes the move. but it will report anything weird.

    bool is_black = false; bool is_red = false; // normally mutually exclusive. normally.
    bool is_king = false;
    bool black_enemy = false; bool red_enemy = false; // same goes for this one.
    if (GetBit(myboard.black_pieces, mymove.from) == 1) is_black = true;
    if (GetBit(myboard.red_pieces, mymove.from) == 1) is_red = true;
    if (GetBit(myboard.kings, mymove.from) == 1) is_king = true;
    if (mymove.is_capture) {
        if (GetBit(myboard.black_pieces, mymove.enemy_pos) == 1) black_enemy = true;
        if (GetBit(myboard.red_pieces, mymove.enemy_pos) == 1) red_enemy = true;
    }
    
    
    //// warning zone ////
    // disagreements in space
    if (is_black && is_red) {
        printf("warning: piece at %d is both black and red.\n", mymove.from);
    }
    if (!is_black && !is_red) {
        printf("warning: moving nonexistent piece at %d.\n", mymove.from);
        if (is_king) {
            printf("warning: where's your crown, king nothing? %d.\n", mymove.from);
        }
    }
    if (GetBit(myboard.black_pieces | myboard.red_pieces, mymove.to) == 1) {
        printf("warning: moving piece to occupied position %d.\n", mymove.to);
    }

    // disagreements in time
    int t = myboard.turn % 2;
    if ((is_black && t == 1) || (is_red && t == 0)) {
        printf("warning: piece at %d being moved does not match player parity of turn (%d).\n", mymove.from, t);
    }

    // invalid position in bounds (out of bounds moves are caught by the bitboardlib.)
    xy_pair xyfrom = ToXY(mymove.from); xy_pair xyto = ToXY(mymove.to);
    if ((xyfrom.x + xyfrom.y) % 2 == 1 || (xyto.x + xyto.y) % 2 == 1) {
        printf("warning: moving piece to/from light square. (%d -> %d)\n", mymove.from, mymove.to);
        printf("coords: (%d, %d) -> (%d, %d)\n", xyfrom.x, xyfrom.y, xyto.x, xyto.y);
    }

    // invalid non-king movement
    if ((xyfrom.y > xyto.y && is_black && !is_king) || (xyfrom.y < xyto.y && is_red && !is_king)) {
        printf("warning: non-king moved backwards. (%d -> %d)\n", mymove.from, mymove.to);
    }

    // invalid captures
    if (mymove.is_capture && black_enemy && red_enemy) {
        printf("warning: (%d -> %d) captures enemy at %d that is both black and red.\n", mymove.from, mymove.to, mymove.enemy_pos);
    }
    if (mymove.is_capture && !(black_enemy || red_enemy)) {
        printf("warning: (%d -> %d) captures nonexistent enemy.\n", mymove.from, mymove.to);
    }
    if (mymove.is_capture && mymove.enemy_pos == -1) {
        printf("warning: (%d -> %d) captures enemy out of bounds.\n", mymove.from, mymove.to);
    }
    if (mymove.is_capture && ((is_black && black_enemy) || (is_red && red_enemy))) {
        printf("warning: (%d -> %d) captures friendly at %d.\n", mymove.from, mymove.to, mymove.enemy_pos);
    }
    //// end warnings ////

    // warnings out of the way... let's get moving.
    boardstate newboard = myboard;
    if (is_black) {
        newboard.black_pieces = ZeroBit(newboard.black_pieces, mymove.from);
        newboard.black_pieces = OneBit(newboard.black_pieces, mymove.to);
    }
    if (is_red) {
        newboard.red_pieces = ZeroBit(newboard.red_pieces, mymove.from);
        newboard.red_pieces = OneBit(newboard.red_pieces, mymove.to);
    }
    if (is_king) {
        newboard.kings = ZeroBit(newboard.kings, mymove.from);
        newboard.kings = OneBit(newboard.kings, mymove.to);
    }
    if (mymove.is_capture) {
        if (black_enemy) {
            newboard.black_pieces = ZeroBit(newboard.black_pieces, mymove.enemy_pos);
        }
        if (red_enemy) {
            newboard.red_pieces = ZeroBit(newboard.red_pieces, mymove.enemy_pos);
        }
        newboard.kings = ZeroBit(newboard.kings, mymove.enemy_pos);
    }

    // promotion
    if ((is_black && (xyto.y == 7)) || (is_red && (xyto.y == 0))) newboard.kings = OneBit(newboard.kings, mymove.to);

    return newboard;
}

// finally... the game flow stuff.
game NewGame() {
    boardstate myboard;

    // positions for red pieces:
    //   57, 59, 61, 63,
    // 48, 50, 52, 54,
    //   41, 43, 45, 47.
    myboard.red_pieces = (unsigned long long) 12273903276444876800u;
    // positions for black pieces:
    // 16, 18, 20, 22,
    //    9, 11, 13, 15,
    //  0,  2,  4,  6.
    myboard.black_pieces = (unsigned long long) 5614165;
    myboard.kings = 0;
    myboard.turn = 0;

    game mygame;
    for (int t = 0; t < 4096; t++) { // MAX_TURNS
        move m = {-1, -1, false, -1};
        mygame.history[t] = m;
    }
    mygame.current_board = myboard;

    return mygame;
}

void TakeTurn(game * mygame, move mymove) {
    // MakeMove on mygame.current_board
    (*mygame).current_board = MakeMove((*mygame).current_board, mymove);
    // add move to history
    (*mygame).history[(*mygame).current_board.turn] = mymove;
    // increment turn counter
    (*mygame).current_board.turn++;
}

// sizeOf(collect) should be 72 or greater. awkward but multipurpose
void UglyPrint(boardstate myboard, bool to_console, char * collect) {
    // this should basically just be PrintBinary but in a different form
    int positions[64];

    char boardstring[] = "#.#.#.#.\n.#.#.#.#\n#.#.#.#.\n.#.#.#.#\n#.#.#.#.\n.#.#.#.#\n#.#.#.#.\n.#.#.#.#";
    for (int p = 0; p < 4; p++) {
        switch (p) {
            case 0:
                memcpy(positions, GetPositionsFromBoard(myboard.black_pieces & ~myboard.kings).ints, sizeof(int) * 64); // black, not kings
                break;
            case 1:
                memcpy(positions, GetPositionsFromBoard(myboard.black_pieces & myboard.kings).ints, sizeof(int) * 64); // black, kings
                break;
            case 2:
                memcpy(positions, GetPositionsFromBoard(myboard.red_pieces & ~myboard.kings).ints, sizeof(int) * 64); // red, not kings
                break;
            case 3:
                memcpy(positions, GetPositionsFromBoard(myboard.red_pieces & myboard.kings).ints, sizeof(int) * 64); // red, kings
                break;
        }
        for (int i = 0; i < 64; i++) {
            if (positions[i] == -1) break; // stop when you run out of pieces in this category
            char symbol = 'x'; // if this shows up something is very wrong
            switch(p) {
                case 0:
                    symbol = 'b'; // black, not kings
                    break;
                case 1:
                    symbol = 'B'; // black, kings
                    break;
                case 2:
                    symbol = 'r'; // red, not kings
                    break;
                case 3:
                    symbol = 'R'; // red, kings
                    break;
            }
            // convert coordinate (invert y)
            // board-coordinates
            // 56 .. .. .. .. .. .. ..
            // 48 .. .. .. .. .. .. ..
            // 40 .. .. .. .. .. .. ..
            // 32 .. .. .. .. .. .. ..
            // 24 .. .. .. .. .. .. ..
            // 16 .. .. .. .. .. .. ..
            // 08 .. .. .. .. .. .. ..
            // 00 01 02 03 04 05 06 07
            // string-coordinates
            // 00 01 02 03 04 05 06 07 +0
            // 08 .. .. .. .. .. .. .. +1
            // 16 .. .. .. .. .. .. .. +2
            // 24 .. .. .. .. .. .. .. +3
            // 32 .. .. .. .. .. .. .. +4
            // 40 .. .. .. .. .. .. .. +5
            // 48 .. .. .. .. .. .. .. +6
            // 56 .. .. .. .. .. .. .. +7
            xy_pair square = ToXY(positions[i]);
            xy_pair stringsquare = {square.x, 7 - square.y};
            int string_index = ToIndex(stringsquare) + ToIndex(stringsquare) / 8;
            boardstring[string_index] = symbol;
        }
    }
    if (to_console) printf("%s\n", boardstring);
    else strcpy(collect, boardstring);
}

void PrettyPrint(boardstate myboard) {
    char * collect = malloc(100);
    UglyPrint(myboard, false, collect);
    char boardstring[128 + 8] = "";
    int c = 0; int b = 0;
    do {
        boardstring[b++] = collect[c];
        if (collect[c] == '\n') continue;
        boardstring[b++] = collect[c];
    }
    while (collect[c++]);

    printf("%s\n", boardstring);
}
// just take every character in UglyPrint and double it (except the newlines)