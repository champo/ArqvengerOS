#include "shell/sudoku/sudoku.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/ctype.h"
#include "library/time.h"
#include "system/call/ioctl/keyboard.h"

#define NUM_BOARDS 20

typedef struct {
    int placed;
    int errors;
    char board[9][9];
    char solvedBoard[9][9];
    char original[9][9];
    int x;
    int y;
} GameState;

//Each char in a board stores the initial state in the lower 4 bits
// and the solved state in the higher 4 bits
static const char *boards[] = {
    "7006950309021004500018200700079803653000"
    "5184080004070900900000405300060768000900"
    "0748695231962137458531824976417982365396"
    "751842825346719179263584253418697684579123",
    "1003004500000100980438971003740506192000"
    "0100081097450000800397140712003693008624"
    "0189362457762415398543897162374258619295"
    "631784816974523628543971457129836931786245",
    "6713048059037000200001003005000006198100"
    "0000000003120040090300200086703030001070"
    "0671324895953786421248195367534278619812"
    "649573796531248467953182125867934389412756",
    "0000109000900050740016003020060002433000"
    "0051001705000600000200110843072523006048"
    "0524317968693825174871694352956178243382"
    "946517417253896749582631168439725235761489",
    "0100090820041030005708000040000703016000"
    "0149003209800070000680020501004090108000"
    "7316749582824153976579862134498675321657"
    "231498132498765743526819285917643961384257",
    "0080090206400500100000340590047600830800"
    "2307000750019640010603500030280101340596"
    "2538619427649257318271834659954761283186"
    "923574327548196492186735765392841813475962",
    "7354681096007915431002500000096020005739"
    "8460220600793406730548180217930635184629"
    "7735468129628791543194253768419632875573"
    "984612286517934967325481842179356351846297",
    "9020700100003000740070008032000453000080"
    "3000000100609200900000102000006001000870"
    "0932874615185362974647591823296145387478"
    "239156351786492869457231724913568513628749",
    "0086409510790130000540007608300002407614"
    "0009509080030600070008098726100454608907"
    "2328647951679513428154928763835196247761"
    "432895492875316213754689987261534546389172",
    "6050097803284760901003586007360902545940"
    "0031880204397026090040098376452000710206"
    "0645219783328476195179358642736891254594"
    "627318812543976261985437983764521457132869",
    "0000570203069281002001008400000100020587"
    "3000070000460080320000500008906009064003"
    "8981457326346928157275163849439816572658"
    "732914712594683863271495524389761197645238",
    "6050000798003960100035000005208149630400"
    "0000796103248578900000610008079403000700"
    "0615428379874396512293571648527814963348"
    "659127961732485789145236156283794432967851",
    "7090300244267059085089247603654908020470"
    "6030900030240695004600007418329018000004"
    "0719638524426715938538924761365497812247"
    "861359891352476953246187674183295182579643",
    "3509000008407020100005418030806050074060"
    "0805007000020800040378003016090000002000"
    "0351986472849732615627541893283695147416"
    "278359975314268192453786538167924764829531",
    "0000000800085076000013820008000502407500"
    "6090100040050042000000500000000490000601"
    "0572694183348517629691382457813759246754"
    "268931269431578426173895187925364935846712",
    "1000000203580064172900000060000072000210"
    "5900050001000308020000000350010061070050"
    "9167483925358926417294175836839647251721"
    "3596485468127939852613744735981626127345892",
    "0315702499078205306529007000003001082040"
    "0036530526000000000905709870501050040000"
    "0831576249947821536652943781769354128284"
    "197365315268974123689457498735612576412893",
    "6010000400000003002390001501600290300720"
    "3001580305029001004002609716200030090000"
    "0681593742754218369239476158165729834972"
    "834615843651297518347926497162583326985471",
    "0000000305040070000015800061000000000950"
    "6004300015300290087032400300209140030060"
    "0789416235564237819321589476138924567295"
    "768143647153982916875324853642791472391658",
    "3900086420200590304710600800608407007300"
    "0500000972630008000140090700450050068000"
    "1395178642628459137471362985162843759734"
    "915826859726314286591473917234568543687291"
};

const size_t boardLeft = 30, boardTop = 4;

static termios gameStatus = { 0, 0 };

static void intro(void);

static void moveInDir(GameState* state, int dir);

static void placeNumber(GameState* state, char num);

static void drawBoard(GameState* state);

/**
 * Sudoku entry point.
 *
 * @param argv The command input string.
 */
void sudoku(char* argv) {

    termios oldStatus;

    time_t initialTime = time(NULL);
    time_t elapsedTime;
    srand(initialTime);
    const char* board = boards[rand() % NUM_BOARDS];
    int in, i, j;

    GameState state;

    ioctl(0, TCGETS, (void*) &oldStatus);
    ioctl(0, TCSETS, (void*) &gameStatus);

    intro();

    state.placed = 0;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {

            state.original[i][j] = state.board[i][j] = board[i+j*9];
            state.solvedBoard[i][j] = board[81+(i+j*9)];

            if (state.board[i][j] != '0') {
                state.placed++;
            }
        }
    }

    elapsedTime = time(NULL) - initialTime;

    state.x = 0;
    state.y = 0;
    state.errors = 0;

    // Reset the screen
    moveCursor(1, 1);
    clearScreen(CLEAR_ALL);

    drawBoard(&state);

    while ((in = getchar()) != 'q') {

        if (in == ESCAPE_CODE && getchar() == CSI) {

            in = getchar();
            if ('A' <= in && in <= 'D') {
                moveInDir(&state, in - 'A');
            }
        } else if (isdigit(in)) {

            placeNumber(&state, in);
            if (state.placed == 81 && state.errors == 0) {

                setForegroundColor(COLOR_GREEN);
                moveCursor(boardTop + 10 * 2 - 1, boardLeft);
                printf("Congratulations! You have solved it correctly.");

                moveCursor(boardTop + 10 * 2 , boardLeft);
                printf("Your time was %d minutes and %d seconds.",elapsedTime / 60, elapsedTime % 60);

                moveCursor(boardTop + 10 * 2 + 1, boardLeft);
                printf("Press enter to exit.");
                setForegroundColor(COLOR_WHITE);

                while (getchar() != '\n');
                break;
            }
        }
    }

    clearScreen(CLEAR_ALL);
    moveCursor(0, 0);
    ioctl(0, TCSETS, (void*) &oldStatus);
}

/**
 * Draw the initial state of the board.
 *
 * @param state The game state to draw.
 */
void drawBoard(GameState* state) {
    int i, j;


    for (i = 0; i < 9; i++) {


        moveCursor(boardTop + i * 2, boardLeft);
        for (j = 0; j < 2 * 9 + 1; j++) {

            if ((i % 3) == 0 || (j % 6) == 0) {
                setForegroundColor(COLOR_WHITE);
                setBold(1);
            } else {
                setForegroundColor(COLOR_BLUE);
            }

            printf("-");

            setBold(0);
        }

        moveCursor(boardTop + i * 2 + 1, boardLeft);
        for (j = 0; j < 9; j++) {

            if ((j % 3) == 0) {
                setForegroundColor(COLOR_WHITE);
                setBold(1);
            } else {
                setForegroundColor(COLOR_BLUE);
            }
            printf("|");

            setForegroundColor(COLOR_WHITE);
            setBold(1);
            if (state->board[i][j] != '0') {
                printf("%c", state->board[i][j]);
            } else {
                printf(" ");
            }
            setBold(0);
        }

        setForegroundColor(COLOR_WHITE);
        setBold(1);
        printf("|");
        setBold(0);
    }

    setBold(1);
    setForegroundColor(COLOR_WHITE);
    moveCursor(boardTop + 9 * 2, boardLeft);
    for (j = 0; j < 9; j++) {
        printf("--");
    }
    printf("-");
    setBold(0);

    moveCursor(boardTop + 1, boardLeft + 1);
}

/**
 * Print the man page for sudoku.
 */
void manSudoku(void) {
    setBold(1);
    printf("Usage:\n\tsudoku\n");
    setBold(0);
}

/**
 * Display the intro.
 */
void intro(void) {

    setBold(1);
    printf("Welcome to Sudoku!\n");
    setBold(0);
    printf("Use the arrow keys to move in the board,");
    printf(" and use the number keys to place a number.\n");
    printf("Inputing 0 will clear the field, if possible.\n");
    printf("You can exit at any time by pressing q.\n");

    printf("Press enter to start your game.");
    while (getchar() != '\n');
}

/**
 * Move the cursor to the cell in a given direction.
 *
 * @param state The game state.
 * @param dir The direction to move in.
 */
void moveInDir(GameState* state, int dir) {
    //UDRL
    switch (dir) {
        case 0:
            // Up!
            state->x--;

            if (state->x < 0) {
                state->x = 8;
            }
            break;
        case 1:
            // Down v
            state->x = (state->x + 1) % 9;
            break;
        case 2:
            // Right yo
            state->y = (state->y + 1) % 9;
            break;
        case 3:
            state->y--;
            if (state->y < 0) {
                state->y = 8;
            }
            break;
    }

    moveCursor(boardTop + 2 * state->x + 1, boardLeft + 2 * state->y + 1);
}

/**
 * Place a number in the current cell.
 *
 * @param state The game state.
 * @param num The number to place.
 */
void placeNumber(GameState* state, char num) {

    int x = state->x, y = state->y, correct, old;

    if (state->original[x][y] != '0') {
        return;
    }

    old = state->board[x][y];
    correct = state->solvedBoard[x][y];

    if (num == '0') {
        printf(" ");
    } else {

        if (correct != num) {
            setBackgroundColor(COLOR_RED);
        }
        printf("%c", num);

        setBackgroundColor(COLOR_BLACK);
    }

    if (old != '0') {
        if (old != correct) {
            state->errors--;
        }
        state->placed--;
    }

    if (num != '0') {
        state->placed++;
        state->errors += num != correct;
    }

    state->board[x][y] = num;
    moveCursor(boardTop + 2 * x + 1, boardLeft + 2 * y + 1);
}

