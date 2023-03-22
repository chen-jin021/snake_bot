#include "game_setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

// import global variables
extern int g_game_over;
extern int g_score;
extern char* g_name;


int checkValid(char* token); // function declare

/** Initializes the board with walls around the edge of the board.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;

    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = FLAG_PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;
    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 2!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    enum board_init_status res = INIT_UNIMPLEMENTED;


    snake_p->loc = 0;
    snake_p->dir = INPUT_RIGHT;

    if(board_rep == NULL){
        // 1. call initialize board
        res = initialize_default_board(cells_p, width_p, height_p);
        int init_index = 20 * 2 + 2;
        insert_last(&snake_p->loc, &init_index, sizeof(int)); // init 3rd row 3rd col    
    } else{
        res = decompress_board_str(cells_p, width_p, height_p, snake_p, board_rep);
    }

    if(res != INIT_SUCCESS){
        return res;
    }

     // 2. set global variables    
    g_game_over = 0;
    g_score = 0;

    place_food(*cells_p, *width_p, *height_p);
    return res;
}

/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 2!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    int length = strlen(compressed)+1;
    char s[length];
    strcpy(s, compressed);// copy the string
    char * token = strtok(s, "|");

    // 1. get board width and height given format
    // check format
    regex_t pattern;
    regcomp(&pattern, "B[0-9]+x[0,9]+", REG_EXTENDED | REG_NOSUB);
    if(!regexec(&pattern, token, 0, NULL, 0)){
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }
    regfree(&pattern);
    
    int width = -1, height = -1;
    sscanf(token, "B%dx%d", &height, &width);
    *width_p = width;
    *height_p = height;

    // initialize cells
    int* cells = malloc(width * height * sizeof(int));
    *cells_p = cells;

    int snakeFound = 0;
    size_t curHeight = 0;
    size_t cell_index = 0;
    token = strtok(NULL, "|"); // a row
    while(token != NULL){
        curHeight++;
        if (curHeight > *height_p){ // check height doesn't exceed
            return INIT_ERR_INCORRECT_DIMENSIONS;
        }
        if(checkValid(token)){
            int i = 0, len = strlen(token);
            size_t curWidth = 0;
            while(i < len){
                int cell = -1;
                int occur = 0;

                if(token[i] == 'W'){
                    cell = FLAG_WALL;
                } else if (token[i] == 'S'){
                    cell = FLAG_SNAKE;
                } else{
                    cell = FLAG_PLAIN_CELL;
                } 
                i++;

                while(i < len && token[i] >= '0' && token[i] <= '9'){
                    occur = occur * 10 + (token[i] - '0');
                    i++;
                }
                curWidth += occur;

                if (curWidth > *width_p){ // check width doesn't exceed
                    return INIT_ERR_INCORRECT_DIMENSIONS;
                }

                //check snake num = 1
                if((cell == FLAG_SNAKE && snakeFound) || (cell == FLAG_SNAKE && occur > 1)){
                    return INIT_ERR_WRONG_SNAKE_NUM;
                } 
                if(cell == FLAG_SNAKE && occur == 1){
                    snakeFound = 1;
                    insert_last(&snake_p->loc, &cell_index, sizeof(int));
                } 
                
                // fill in cell's row
                int counter = 0;
                while(counter < occur){
                    cells[cell_index] = cell;
                    cell_index++;
                    counter++;
                }
            }
            if (curWidth != *width_p){
                return INIT_ERR_INCORRECT_DIMENSIONS;
            }
        } else{
            return INIT_ERR_BAD_CHAR; // not valid row
        }
        token = strtok(NULL, "|"); // new row
    }

    // check dimensions <- height
    if(curHeight != *height_p){
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }

    // check snake number
    if(!snakeFound){
        return INIT_ERR_WRONG_SNAKE_NUM;
    }
    return INIT_SUCCESS;
}

int checkValid(char* token){
    int i = 0, len = strlen(token);
    while(i < len){
        // first always cell sign
        if(token[i] != 'W' && token[i] != 'E' && token[i] != 'S'){
            return 0;
        }
        i++;
        while(i < len && token[i] >= '0' && token[i] <= '9'){
            i++;
        }
    }
    return 1;
}
