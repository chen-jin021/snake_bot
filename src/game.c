#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linked_list.h"
#include "mbstrings.h"
#include "common.h"

#define WRAP 1

extern int g_game_over;
extern int g_score;
extern int g_name_len;

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 2!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */
void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.

    // TODO: implement!
    // check if game ended
    if(g_game_over == 1){
        return;
    }
    
    if(growing == 0){
        if(input != INPUT_NONE){
            snake_p->dir = input;
        }

        int new_loc = *(int*)get_last(snake_p->loc);

        if(snake_p->dir == INPUT_UP){
            new_loc -= width;
        } else if (snake_p->dir == INPUT_DOWN){
            new_loc += width;
        } else if (snake_p->dir == INPUT_LEFT){
            new_loc -= 1;
        } else{
            new_loc += 1;
        }

        
        // check if hit the WALL
        if(cells[new_loc] == FLAG_WALL){
            g_game_over = 1;
            return;
        }

        // for wraparound <- applies for borders WITHOUT WALL
        #if defined WRAP
        int w = (int)width;
        int h = (int)height;
        if(cells[new_loc] == FLAG_WALL && 
            (new_loc < w || new_loc >= w * (h -1) ||
            new_loc % w == 0 || (new_loc + 1) % w == 0)){

            // check directions
            if(snake_p->dir == INPUT_UP){
                new_loc += w * (h -2);
            } else if (snake_p->dir == INPUT_DOWN){
                new_loc -= w * (h -2);
            } else if (snake_p->dir == INPUT_LEFT){
                new_loc += (w - 2);
            } else{
                new_loc -= (w - 2);
            }
        }
        #endif
        

        // check if player got food
        if(cells[new_loc] == FLAG_FOOD){
            place_food(cells, width, height);
            g_score++;
        }

        // set cur (before move) cell to plain cell
        cells[*(int*)get_first(snake_p->loc)] = FLAG_PLAIN_CELL;
        cells[new_loc] = FLAG_SNAKE;
        // add new_loc to snake linkedlist
        insert_last(&snake_p->loc, &new_loc, sizeof(int));
        // remove the last node
        void* removed = remove_first(&snake_p->loc);
        free(removed);
        // snake_p->loc = new_loc;
    } else{
        // deals with backing into itself
        if((snake_p->dir == INPUT_LEFT && input == INPUT_RIGHT) ||
            (snake_p->dir == INPUT_RIGHT && input == INPUT_LEFT) || 
            (snake_p->dir == INPUT_UP && input == INPUT_DOWN) || 
            (snake_p->dir == INPUT_DOWN && input == INPUT_UP)){
            input = INPUT_NONE;
        }

        if(input != INPUT_NONE){
            snake_p->dir = input;
        }

        int new_loc = *(int*)get_last(snake_p->loc);

        if(snake_p->dir == INPUT_UP){
            new_loc -= width;
        } else if (snake_p->dir == INPUT_DOWN){
            new_loc += width;
        } else if (snake_p->dir == INPUT_LEFT){
            new_loc -= 1;
        } else{
            new_loc += 1;
        }

        // check if hit the WALL
        if(cells[new_loc] == FLAG_WALL ){
            g_game_over = 1;
            return;
        }

        // for wraparound <- applies for borders WITHOUT WALL
        #if defined WRAP
        int w = (int)width;
        int h = (int)height;
        if(cells[new_loc] == FLAG_WALL && 
            (new_loc < w || new_loc >= w * (h -1) ||
            new_loc % w == 0 || (new_loc + 1) % w == 0)){

            // check directions
            if(snake_p->dir == INPUT_UP){
                new_loc += w * (h -2);
            } else if (snake_p->dir == INPUT_DOWN){
                new_loc -= w * (h -2);
            } else if (snake_p->dir == INPUT_LEFT){
                new_loc += (w - 2);
            } else{
                new_loc -= (w - 2);
            }
        }
        #endif
        

        // check if player got food
        if(cells[new_loc] == FLAG_FOOD){
            place_food(cells, width, height);
            g_score++;
        }

        // check if set cur (before move) cell to plain cell
        if(cells[new_loc] != FLAG_FOOD){
            cells[*(int*)get_first(snake_p->loc)] = FLAG_PLAIN_CELL;
            void* removed = remove_first(&snake_p->loc);

             // check if hit itself
            if(cells[new_loc] == FLAG_SNAKE && new_loc != *(int*)removed){
                cells[*(int*) removed] = FLAG_SNAKE;
                g_game_over = 1;
                free(removed);
                return;
            }
            free(removed);
        }

        // we don't have to check if snake git itself outside of the above if statement
        // because if it does, new_loc != FLAG_FOOD

        cells[new_loc] = FLAG_SNAKE;
        // add new_loc to snake linkedlist
        insert_last(&snake_p->loc, &new_loc, sizeof(int)); 
    }
    
}

/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    unsigned food_index = generate_index(width * height);
    if (*(cells + food_index) == FLAG_PLAIN_CELL) {
        *(cells + food_index) = FLAG_FOOD;
    } else {
        place_food(cells, width, height);
    }
    /* DO NOT MODIFY THIS FUNCTION */
}

/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    // TODO: implement! (remove the call to strcpy once you begin your
    // implementation)
    printf("Name > ");
    fflush(0); // fd we are reading from
    int fd = 0; // for command line reads
    size_t n = read(fd, write_into, 1000);
    if(n == 1 && write_into[0] == '\n'){
        fprintf(stderr, "Name Invalid: must be longer than 0 characters.\n");
        read_name(write_into);
    }else{
        write_into[n-1] = '\0';
    }
}

/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 2)
 */
void teardown(int* cells, snake_t* snake_p) {
    // TODO: implement!
    free(cells);
    while(snake_p->loc != NULL){
        void* data = remove_last(&(snake_p->loc));
        free(data);
    }
}
