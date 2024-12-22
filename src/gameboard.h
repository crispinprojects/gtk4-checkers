/***************************************************************************
 *   Author Alan Crispin                                                   *
 *   crispinalan@gmail.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation.                                         *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/


#ifndef _GAMEBOARD_H
#define _GAMEBOARD_H

#include <ncurses.h>
#include <string.h> //needed for strlen
#include <stdlib.h>

#define BOARD_SIZE 8

void set_monitor(bool value);

void init_board(int board[][BOARD_SIZE]);
void clear_board(int board[][BOARD_SIZE]);
void init_all(void);
void create_info_win();

void draw_board(int board[][BOARD_SIZE], int cursor_x, int cursor_y);
bool move_white(int board[][BOARD_SIZE], int x1, int y1, int x2, int y2);
bool move_black(int board[][BOARD_SIZE], int x1, int y1, int x2, int y2);

bool can_capture(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int *x_cap, int *y_cap);

bool is_white_move_valid(int board[BOARD_SIZE][BOARD_SIZE],int x1, int y1, int x2, int y2);
void get_possible_white_moves(int board[BOARD_SIZE][BOARD_SIZE],int possibleMoves[100][4], int* numMoves);

bool is_black_move_valid(int board[BOARD_SIZE][BOARD_SIZE],int x1, int y1, int x2, int y2);
void get_possible_black_moves(int board[BOARD_SIZE][BOARD_SIZE],int possibleMoves[100][4], int* numMoves);

bool is_coord_valid(int x, int y);
//AI
void copy_board(int src[BOARD_SIZE][BOARD_SIZE], int dest[BOARD_SIZE][BOARD_SIZE]);
int evaluate_position(int board[BOARD_SIZE][BOARD_SIZE]);
int evaluate_position2(int board[BOARD_SIZE][BOARD_SIZE]);
void get_possible_moves(int board[BOARD_SIZE][BOARD_SIZE], int turn, int possibleMoves[100][4], int* numMoves);

int minimax_jumps(int board[BOARD_SIZE][BOARD_SIZE], int maxDepth, int depth, int turn, int alpha, int beta);

void make_move(int board[BOARD_SIZE][BOARD_SIZE], int x1, int y1, int x2, int y2);

void get_best_move_AI(int board[BOARD_SIZE][BOARD_SIZE], int turn, int maxDepth, int* x1, int* y1, int* x2, int* y2);

int get_number_white_pieces(int board[BOARD_SIZE][BOARD_SIZE]);
int get_number_black_pieces(int board[BOARD_SIZE][BOARD_SIZE]);
bool is_black_win(int board[BOARD_SIZE][BOARD_SIZE]);
bool is_white_win(int board[BOARD_SIZE][BOARD_SIZE]);

int get_number_BMAN(int board[BOARD_SIZE][BOARD_SIZE]);
int get_number_BKING(int board[BOARD_SIZE][BOARD_SIZE]);
int get_number_WMAN(int board[BOARD_SIZE][BOARD_SIZE]);
int get_number_WKING(int board[BOARD_SIZE][BOARD_SIZE]);


#endif
