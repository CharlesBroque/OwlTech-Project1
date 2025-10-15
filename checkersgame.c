#include "checkerslib.c"
#include <stdio.h>
#include <time.h>

// compulsory capture?
// save/load/undo?
// help in main menu

int main() {
    game checkers = NewGame();
    
    char menu_choice[] = "x";
    bool quit = false;
    bool in_game = false;
    while (!quit) {
        printf("\n==== BITBOARD CHECKERS ====\n");
        printf("n - New Game\n");
        printf("c - Continue Game\n");
        printf("s - Save Game (to file)\n");
        printf("l - Load Game (from file; this will overwrite the current game)\n");
        printf("h - Help (learn move notation)\n");
        printf("q - Quit\n");
        printf("\n");
        printf("Choose (type) an option to begin: \n");
        scanf("%1[^\n]", &menu_choice); getchar();
        printf("\n");
        switch (menu_choice[0]) {
            case 'h': // help
                printf("In Bitboard Checkers, moves are entered as pairs of integers ranging 0 to 63.\n");
                printf("The first integer is the origin square (to identify the piece) and the second is the destination.\n");
                printf("Typically this looks like incrementing by 7 or 9, though not all such increments are valid. Backward moves decrement the index similarly. Captures by analogy use differences of 14 or 18.\n");

                printf("\n");
                printf("To identify specific squares, see the board. Rows begin on multiples of 8 and count up to the right.\n");
                printf("To find the index of a square, add the value of the row and column.\n");
                printf("56 -> ##..##..##..##..\n");
                printf("48 -> ..##..##..##..##\n");
                printf("40 -> ##..##..##..##..\n");
                printf("32 -> ..##..##..##..##\n");
                printf("24 -> ##..##..##..##..\n");
                printf("16 -> ..##..##..##..##\n");
                printf("08 -> ##..##..##..##..\n");
                printf("00 -> ..##..##..##..##\n");
                printf("       ^ ^ ^ ^ ^ ^ ^ ^\n");
                printf(" +     0 1 2 3 4 5 6 7\n");

                printf("\n");
                printf("Pieces are represented by letters. 'bb' for black and 'rr' for red. Kings are capitalized (BB or RR) and allowed to move backward.\n");
                printf("All valid moves are diagonal. Captures travel double the distance of an ordinary move and remove the opposing piece in the middle.\n");
                printf("The capturing move from black in this position looks like \"18 36\". The noncapturing move looks like \"18 25\".\n");
                printf("56 -> ##..##..##..##..\n");
                printf("48 -> ..##..##..##..##\n");
                printf("40 -> ##..##..##..##..\n");
                printf("32 -> ..##..##..##..##\n");
                printf("24 -> ##..##rr##..##..\n");
                printf("16 -> ..##bb##..##..##\n");
                printf("08 -> ##..##..##..##..\n");
                printf("00 -> ..##..##..##..##\n");
                printf("       ^ ^ ^ ^ ^ ^ ^ ^\n");
                printf(" +     0 1 2 3 4 5 6 7\n");

                printf("\n");
                printf("The game is over when one side has lost all their pieces.\n");
                printf("This is all you need to play the game. Happy gaming!\n\n");
                break;
            case 's': // save game
                FILE *savefile = fopen("savegame.txt", "w");
                int t = 0;
                move m = checkers.history[t];
                while (m.from != -1) {
                    fprintf(savefile, "%d. %d -> %d (x%d)\n", t, m.from, m.to, m.enemy_pos);
                    t++;
                    m = checkers.history[t];
                }
                fclose(savefile);
                printf("Saved. Don't rename or edit your save file!\n");
                break;
            case 'l': // load game
                FILE *loadfile = fopen("savegame.txt", "r");
                if (loadfile == NULL) {
                    printf("Cannot load game, savegame.txt is missing.\n");
                    break;
                }
                checkers = NewGame();
                char line[256];
                while (fgets(line, sizeof(line), loadfile) != NULL) { // read lines
                    line[strcspn(line, "\n")] = '\0'; // strip newline character
                    line[strcspn(line, ".")] = ' '; // remove .
                    line[strcspn(line, "-")] = ' '; // remove - (should not affect -1 as it only hits the first appearance)
                    line[strcspn(line, ">")] = ' '; // remove >
                    line[strcspn(line, "(")] = ' '; // remove (
                    line[strcspn(line, ")")] = ' '; // remove )
                    line[strcspn(line, "x")] = ' '; // remove x

                    char arg[100] = ""; move m;
                    strcpy(arg, strtok(line, " ")); // turn (this is probably not necessary)
                    strcpy(arg, strtok(NULL, " ")); // move.from
                    m.from = atoi(arg);
                    strcpy(arg, strtok(NULL, " ")); // move.to
                    m.to = atoi(arg);
                    strcpy(arg, strtok(NULL, " ")); // move.enemy_pos
                    m.enemy_pos = atoi(arg);
                    if (m.enemy_pos != -1) m.is_capture = true;
                    else m.is_capture = false;

                    TakeTurn(&checkers, m);
                }
                fclose(loadfile);
                printf("Loaded game! Press c in the main menu to continue.\n");
                break;
            case 'n': // new game
                checkers = NewGame();
                srand(time(NULL)); // no break
            case 'c': // continue game
                in_game = true;
                char to_move[6];
                int turn;
                while (in_game) {
                    printf("\n");
                    // UglyPrint(checkers.current_board, true, ""); // really ugly
                    PrettyPrint(checkers.current_board);
                    turn = checkers.current_board.turn;
                    if (turn % 2 == 0) strcpy(to_move, "black\0");
                    else strcpy(to_move, "red\0");
                    printf("\nTurn %d, %s to move.\n", turn, to_move);
                    
                    if (checkers.current_board.black_pieces == 0 || checkers.current_board.red_pieces == 0) {
                        printf("Game over, %s has lost all their pieces.\n", to_move);
                        in_game = false; // just in case
                        break;
                    }
                    
                    // move
                    bool valid_move = false;
                    char action[] = "x";
                    move mymove;
                    while(!valid_move) {
                        printf("Enter a command:\n");
                        printf("m - move\n");
                        printf("p - reprint board\n");
                        printf("g - have a move suggested\n");
                        printf("r - move randomly\n");
                        printf("l - list all possible moves\n");
                        printf("q - quit to menu\n");
                        scanf("%1[^\n]", &action); getchar();
                        moveset all_moves = GetLegalMoves(checkers.current_board);
                        switch(action[0]) {
                            case 'q': // quit
                                valid_move = true;
                                in_game = false;
                                break;
                            case 'm': // move
                                printf("Enter a beginning and ending position for the move.\n");
                                int s = scanf("%d %d", &mymove.from, &mymove.to); getchar();
                                if (s != 2) {
                                    printf("Invalid input, try again.\n");
                                    continue;
                                }
                                // else: could be valid!
                                if (!InBounds(ToXY(mymove.from)) || !InBounds(ToXY(mymove.to))) {
                                    printf("Invalid move, try again.\n");
                                    continue;
                                }
                                xy_pair xyfrom = ToXY(mymove.from); xy_pair xyto = ToXY(mymove.to);
                                if ((abs(xyfrom.y - xyto.y) == 2) && (abs(xyfrom.x - xyto.x) == 2)) { // this could be a capture
                                    xy_pair xycap = {(xyfrom.x + xyto.x) / 2, (xyfrom.y + xyto.y) / 2};
                                    mymove.is_capture = true;
                                    mymove.enemy_pos = ToIndex(xycap);
                                }
                                else { // not a capture
                                    mymove.is_capture = false;
                                    mymove.enemy_pos = -1;
                                }
                                if (IsLegalMove(checkers.current_board, mymove)) {
                                    valid_move = true;
                                    TakeTurn(&checkers, mymove);
                                }
                                else {
                                    printf("Invalid move, try again.\n");
                                    // MakeMove(checkers.current_board, mymove); // this tells you the reason(s) it's invalid
                                }
                                break;
                            case 'g':
                                int c = 0; // count legal moves
                                for (int i = 0; i < 64; i++) {
                                    move m = all_moves.moves[i];
                                    if (m.from == -1) break;
                                    c++;
                                }
                                if (c == 0) {
                                    // ???
                                    printf("no moves! how did you get here?\n");
                                    break;
                                }
                                int sug = rand() % c;
                                move m = all_moves.moves[sug];
                                printf("\nConsider %d -> %d.\n\n", m.from, m.to);
                                break;
                            case 'r':
                                c = 0; // count legal moves
                                for (int i = 0; i < 64; i++) {
                                    move m = all_moves.moves[i];
                                    if (m.from == -1) break;
                                    c++;
                                }
                                if (c == 0) {
                                    // ???
                                    printf("no moves! how did you get here?\n");
                                    break;
                                }
                                int pick = rand() % c;
                                m = all_moves.moves[pick];
                                valid_move = true;
                                TakeTurn(&checkers, m); // no need to revalidate
                                printf("\n%s moved %d -> %d.\n", to_move, m.from, m.to);
                                break;
                            case 'l':
                                printf("\nHere is every legal move:\n\n");
                                for (int i = 0; i < 64; i++) {
                                    move m = all_moves.moves[i];
                                    if (m.from == -1) break;
                                    printf("%d -> %d (captures?: %d)\n", m.from, m.to, m.is_capture);
                                }
                                break;
                            case 'p':
                                printf("\n");
                                // UglyPrint(checkers.current_board, true, "");
                                PrettyPrint(checkers.current_board);
                                printf("\n");
                                break;
                            default:
                                printf("Invalid input, try again.\n");
                                continue;
                        }
                    }                    
                }
                break;
            case 'q': // quit
                quit = true;
                break;
            default:
                printf("Unrecognized command, try again.\n");
                break;
        }
    }
    
    
    return 0;
}