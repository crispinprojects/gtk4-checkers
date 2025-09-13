/* main.c
 *
 * gtk4 checkers game using minimax
 * 
 * Copyright 2025 Alan Crispin <crispinalan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include <gtk/gtk.h>
#include <math.h>   // For abs()
#include <stdlib.h> // For atoi(), rand(), srand()
#include <string.h> // For strcmp()
#include <limits.h> // For INT_MIN, INT_MAX
#include <time.h>   // For time() to seed rand

// --- Constants for Game Logic ---
#define EMPTY 0
#define WMAN 1
#define BMAN 2
#define WKING 3
#define BKING 4
#define WHITE 5
#define BLACK 6

// --- Game Modes ---
#define MODE_HUMAN_VS_AI 0
#define MODE_AI_VS_AI 1

// --- Transposition Table Constants ---
#define TT_SIZE 1048576 // 2^20 entries (1MB if each entry is 8 bytes)
#define TT_FLAG_EXACT 0 // Exact score
#define TT_FLAG_LOWERBOUND 1 // Score is a lower bound (alpha cutoff)
#define TT_FLAG_UPPERBOUND 2 // Score is an upper bound (beta cutoff)

// --- Global Game State Variables ---
int board_state[8][8];             // The actual checkers board state (x=col, y=row; 0,0 is top-left)
int current_side = BLACK;          // Current player's turn (WHITE or BLACK)
int game_mode = MODE_HUMAN_VS_AI;  // Default game mode
int consecutive_non_capture_moves = 0; // For draw by repetition rule (e.g., 40 moves without capture)

// For Threefold Repetition Detection
GArray *zobrist_history; // GArray of guint64 to store past board hashes

GtkWidget *cell_containers[8][8];  // Array of GtkBox widgets for each cell (for square background)
GtkWidget *cell_images[8][8];      // Array of GtkImage widgets for pieces
GtkWidget *diagonal_labels[8][8];  // Array of GtkLabel widgets for diagonal numbers

GtkWidget *status_label;           // Label to display current turn or game messages
GtkWidget *from_entry;             // Input field for "From" diagonal square (Human vs. AI)
GtkWidget *to_entry;               // Input field for "To" diagonal square (Human vs. AI)
GtkWidget *human_make_move_button; // Button for human player to make a move
GtkWidget *next_ai_move_button;    // Button to advance AI turn in AI vs AI mode, or trigger AI turn in Human vs AI
GtkWidget *human_input_box;        // HBox to contain human input elements

// Helper struct for coordinates
typedef struct {
    int x;
    int y;
} Coord;

// Structure to represent a possible move, including capture information
struct _Move {
    int x1; // Starting X of the sequence
    int y1; // Starting Y of the sequence
    int x2; // Ending X of the sequence
    int y2; // Ending Y of the sequence
    GArray *captured_coords; // GArray of Coord for all captured pieces
    gboolean is_jump_sequence; // TRUE if this move involves one or more jumps
};
typedef struct _Move Move;

// --- Zobrist Hashing Variables ---
guint64 zobrist_table[8][8][5]; // [x][y][piece_type: EMPTY, WMAN, BMAN, WKING, BKING]
guint64 zobrist_side_to_move;   // For current_side (WHITE or BLACK)

// --- Transposition Table Structure ---
typedef struct {
    guint64 hash_key; // The Zobrist hash of the board state
    int score;        // The minimax score
    int depth;        // The depth at which this score was calculated
    int flag;         // TT_FLAG_EXACT, TT_FLAG_LOWERBOUND, TT_FLAG_UPPERBOUND
} TranspositionEntry;

TranspositionEntry transposition_table[TT_SIZE];


// --- Function Prototypes (Declarations) ---
static gboolean check_white_win();
static gboolean check_black_win();
static gboolean apply_move_to_board(int board[8][8], const Move *move);
static int evaluate_board(int board[8][8]);
static int minimax(int board[8][8], int depth, int alpha, int beta, gboolean is_maximizing_player); // Added alpha, beta
static Move find_best_move(int current_board[8][8], int player_side);
static void find_all_jump_paths_recursive(int board[8][8], int current_x, int current_y, int initial_x, int initial_y,
                                           int player_side, GArray *current_captured_coords, GArray *all_jump_moves_found);
static GArray* get_all_possible_moves_for_player(int board[8][8], int player_side);
static gboolean diagonal_to_xy(int diag_num, int *out_x, int *out_y);
static int xy_to_diagonal(int x, int y);
static void update_cell_display(int x, int y);
static void update_all_display();
static void copy_board(int source_board[8][8], int dest_board[8][8]);
static void init_board();
static void print_board();
static void load_css(const char *css_data);
static void on_next_ai_move_button_clicked(GtkButton *button, gpointer user_data);
static void on_human_make_move_button_clicked(GtkButton *button, gpointer user_data);
static void reset_game();
static void set_game_mode(int mode);
static void on_mode_human_vs_ai_clicked(GtkButton *button, gpointer user_data);
static void on_mode_ai_vs_ai_clicked(GtkButton *button, gpointer user_data);
static gboolean check_threefold_repetition();


// --- Zobrist Hashing Functions ---

/**
 * @brief Generates a random 64-bit unsigned integer.
 * Uses rand() multiple times to fill a 64-bit value.
 * @return A random guint64.
 */
static guint64 generate_random_guint64() {
    guint64 r = 0;
    for (int i = 0; i < 4; i++) { // Fill 4 * 16 bits = 64 bits (assuming RAND_MAX is at least 32767)
        r = (r << 16) | (rand() & 0xFFFF);
    }
    return r;
}

/**
 * @brief Initializes the Zobrist keys with random numbers.
 * Should be called once at the start of the program.
 */
static void init_zobrist_keys() {
    srand(time(NULL)); // Seed the random number generator

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            for (int piece_type = 0; piece_type < 5; piece_type++) {
                zobrist_table[x][y][piece_type] = generate_random_guint64();
            }
        }
    }
    zobrist_side_to_move = generate_random_guint64();
    g_print("Zobrist keys initialized.\n");
}

/**
 * @brief Computes the Zobrist hash for a given board state.
 * @param board The board state to hash.
 * @param side_to_move The current side to move (WHITE or BLACK).
 * @return The Zobrist hash of the board.
 */
static guint64 compute_hash(int board[8][8], int side_to_move) {
    guint64 hash = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int piece = board[x][y];
            if (piece != EMPTY) {
                hash ^= zobrist_table[x][y][piece];
            }
        }
    }
    if (side_to_move == BLACK) { // Only XOR if black is to move (as white is default)
        hash ^= zobrist_side_to_move;
    }
    return hash;
}

// --- Transposition Table Functions ---

/**
 * @brief Looks up a board state in the transposition table.
 * @param hash The Zobrist hash of the board state.
 * @param depth The current search depth.
 * @param alpha The current alpha value.
 * @param beta The current beta value.
 * @param score_out Pointer to store the retrieved score.
 * @return TRUE if a valid entry is found and can be used, FALSE otherwise.
 */
static gboolean lookup_transposition_table(guint64 hash, int depth, int alpha, int beta, int *score_out) {
    TranspositionEntry *entry = &transposition_table[hash % TT_SIZE];

    if (entry->hash_key == hash) {
        // Entry found, check if depth is sufficient
        if (entry->depth >= depth) {
            g_print("TT HIT! (Hash: %llu, Depth: %d, Flag: %d)\n", hash, entry->depth, entry->flag);
            if (entry->flag == TT_FLAG_EXACT) {
                *score_out = entry->score;
                return TRUE;
            }
            if (entry->flag == TT_FLAG_LOWERBOUND && entry->score >= beta) {
                *score_out = entry->score;
                return TRUE;
            }
            if (entry->flag == TT_FLAG_UPPERBOUND && entry->score <= alpha) {
                *score_out = entry->score;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**
 * @brief Stores a board state and its score in the transposition table.
 * @param hash The Zobrist hash of the board state.
 * @param score The minimax score.
 * @param depth The depth at which this score was calculated.
 * @param flag The type of score (TT_FLAG_EXACT, TT_FLAG_LOWERBOUND, TT_FLAG_UPPERBOUND).
 */
static void store_transposition_table(guint64 hash, int score, int depth, int flag) {
    TranspositionEntry *entry = &transposition_table[hash % TT_SIZE];

    // Simple replacement strategy: always replace if new depth is greater or hash is different
    // For more advanced, replace only if new depth is greater, or if it's an exact score.
    if (entry->hash_key != hash || depth >= entry->depth) {
        entry->hash_key = hash;
        entry->score = score;
        entry->depth = depth;
        entry->flag = flag;
        g_print("TT STORED! (Hash: %llu, Score: %d, Depth: %d, Flag: %d)\n", hash, score, depth, flag);
    }
}


// --- Internal Helper Functions ---

/**
 * @brief Initializes the game board to its standard starting checkers position.
 * Black pieces on rows 0, 1, 2 (top of the board).
 * White pieces on rows 5, 6, 7 (bottom of the board).
 */
static void init_board() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            board_state[x][y] = EMPTY;
        }
    }

    // Initialize black pieces (top of the board, rows 0, 1, 2)
    // Black pieces are on dark squares (sum of coordinates is odd)
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 8; x++) {
            if ((x + y) % 2 == 1) { // Dark squares
                board_state[x][y] = BMAN;
            }
        }
    }

    // Initialize white pieces (bottom of the board, rows 5, 6, 7)
    // White pieces are on dark squares (sum of coordinates is odd)
    for (int y = 5; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((x + y) % 2 == 1) { // Dark squares
                board_state[x][y] = WMAN;
            }
        }
    }
    // Set current side to BLACK as per standard checkers rules
    current_side = BLACK;
}

/**
 * @brief Prints the current state of the board to the console for debugging.
 */
static void print_board() {
    g_print("======================================================\n");
    g_print("Board State (0,0 is top-left):\n");
    g_print("  0 1 2 3 4 5 6 7 (X)\n");
    g_print("------------------------------------------------------\n");
    for (int y = 0; y < 8; y++) {
        g_print("%d|", y); // Y-axis label
        for (int x = 0; x < 8; x++) {
            g_print("%d ", board_state[x][y]);
        }
        g_print("\n");
    }
    g_print("======================================================\n");
}

/**
 * @brief Converts a diagonal square number (1-32) to (x,y) board coordinates.
 * Assumes (0,0) is top-left, and numbering starts from top-left dark square.
 *
 * @param diag_num The diagonal square number (1-32).
 * @param out_x Pointer to store the X-coordinate.
 * @param out_y Pointer to store the Y-coordinate.
 * @return TRUE if conversion is successful, FALSE otherwise (invalid diag_num).
 */
static gboolean diagonal_to_xy(int diag_num, int *out_x, int *out_y) {
    if (diag_num < 1 || diag_num > 32) {
        return FALSE;
    }

    // Adjust for 0-based indexing for calculations
    int adjusted_diag = diag_num - 1;

    // Calculate row (y)
    *out_y = adjusted_diag / 4;

    // Calculate column (x) based on row parity
    if (*out_y % 2 == 0) { // Even rows (0, 2, 4, 6) start with dark squares at odd x
        *out_x = (adjusted_diag % 4) * 2 + 1;
    } else { // Odd rows (1, 3, 5, 7) start with dark squares at even x
        *out_x = (adjusted_diag % 4) * 2;
    }

    // Validate if the calculated (x,y) is indeed a dark square
    if ((*out_x + *out_y) % 2 == 0) { // If sum is even, it's a light square, something is wrong
        g_print("ERROR: Mapped diagonal %d to (%d, %d) which is a light square.\n", diag_num, *out_x, *out_y);
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Converts (x,y) board coordinates to a diagonal square number (1-32).
 * Assumes (0,0) is top-left.
 *
 * @param x The X-coordinate.
 * @param y The Y-coordinate.
 * @return The diagonal square number (1-32), or 0 if not a valid dark square.
 */
static int xy_to_diagonal(int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8 || (x + y) % 2 == 0) {
        return 0; // Not a valid dark square
    }

    int diag_num;
    if (y % 2 == 0) { // Even rows (0, 2, 4, 6)
        diag_num = (y * 4) + ((x - 1) / 2) + 1;
    } else { // Odd rows (1, 3, 5, 7)
        diag_num = (y * 4) + (x / 2) + 1;
    }
    return diag_num;
}


/**
 * @brief Updates the display of a single cell, including the piece and diagonal number.
 * @param x The X-coordinate of the cell.
 * @param y The Y-coordinate of the cell.
 */
static void update_cell_display(int x, int y) {
    GtkWidget *cell_container = cell_containers[x][y];
    GtkWidget *piece_image = cell_images[x][y];
    GtkWidget *diag_label = diagonal_labels[x][y];

    // Remove all possible background classes from the cell container first
    gtk_widget_remove_css_class(cell_container, "light-square");
    gtk_widget_remove_css_class(cell_container, "dark-square");
    gtk_widget_remove_css_class(cell_container, "selected");

    // Apply light/dark square background to the cell container
    if ((x + y) % 2 == 1) { // Dark squares (where pieces can be)
        gtk_widget_add_css_class(cell_container, "dark-square");
        if (diag_label) {
            gtk_widget_set_visible(diag_label, TRUE);
        }
    } else { // Light squares (where pieces cannot be)
        gtk_widget_add_css_class(cell_container, "light-square");
        if (diag_label) {
            gtk_widget_set_visible(diag_label, FALSE);
        }
    }

    // Update the piece image
    int piece_type = board_state[x][y];

    // Remove all piece-related CSS classes first
    gtk_widget_remove_css_class(piece_image, "wman");
    gtk_widget_remove_css_class(piece_image, "bman");
    gtk_widget_remove_css_class(piece_image, "wking");
    gtk_widget_remove_css_class(piece_image, "bking");
    gtk_widget_remove_css_class(piece_image, "empty");

    if (piece_type != EMPTY) {
        gtk_widget_set_visible(piece_image, TRUE);
        switch (piece_type) {
            case WMAN:  gtk_widget_add_css_class(piece_image, "wman"); break;
            case BMAN:  gtk_widget_add_css_class(piece_image, "bman"); break;
            case WKING: gtk_widget_add_css_class(piece_image, "wking"); break;
            case BKING: gtk_widget_add_css_class(piece_image, "bking"); break;
        }
    } else {
        gtk_widget_set_visible(piece_image, FALSE); // Hide image if cell is empty
        gtk_widget_add_css_class(piece_image, "empty"); // Add empty class for potential styling
    }
}

/**
 * @brief Updates the entire display of the game board.
 * This function iterates through the internal board_state and updates the
 * CSS classes of the GtkBox containers and GtkImage widgets accordingly.
 */
static void update_all_display() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            update_cell_display(x, y);
        }
    }
    // Update status label based on game mode
    if (game_mode == MODE_HUMAN_VS_AI) {
        gtk_label_set_text(GTK_LABEL(status_label), current_side == WHITE ? "White's Turn (AI)" : "Black's Turn (Human)");
    } else { // AI vs AI
        gtk_label_set_text(GTK_LABEL(status_label), current_side == WHITE ? "White's Turn (AI)" : "Black's Turn (AI)");
    }
}

/**
 * @brief Copies the content of one board state to another.
 * @param source_board The board state to copy from.
 * @param dest_board The board state to copy to.
 */
static void copy_board(int source_board[8][8], int dest_board[8][8]) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            dest_board[x][y] = source_board[x][y];
        }
    }
}

/**
 * @brief Recursive helper to find all possible jump paths for a given piece.
 * @param board The current board state (will be modified temporarily for recursion).
 * @param current_x Current X-coordinate of the jumping piece.
 * @param current_y Current Y-coordinate of the jumping piece.
 * @param initial_x Initial X-coordinate of the piece at the start of the sequence.
 * @param initial_y Initial Y-coordinate of the piece at the start of the sequence.
 * @param player_side The side of the player whose turn it is (WHITE or BLACK).
 * @param current_captured_coords GArray of Coord for pieces captured so far in this path.
 * @param all_jump_moves_found GArray where completed Move sequences are stored.
 */
static void find_all_jump_paths_recursive(int board[8][8], int current_x, int current_y,
                                           int initial_x, int initial_y, int player_side,
                                           GArray *current_captured_coords, GArray *all_jump_moves_found) {
    gboolean found_further_jump = FALSE;
    int piece = board[current_x][current_y];

    int dx_options[] = {-1, 1};
    int dy_options[] = {-1, 1};

    // Adjust directions for men
    if (piece == WMAN) {
        dy_options[1] = -1; // Only move up
    } else if (piece == BMAN) {
        dy_options[0] = 1; // Only move down
    }

    for (int i = 0; i < 2; i++) { // dx_options
        for (int j = 0; j < 2; j++) { // dy_options (adjusted for men)
            int dx = dx_options[i];
            int dy = dy_options[j];

            // Skip invalid directions for men if not kings
            if (piece == WMAN && dy == 1) continue; // White man cannot move down
            if (piece == BMAN && dy == -1) continue; // Black man cannot move up

            int mid_x = current_x + dx;
            int mid_y = current_y + dy;
            int next_x = current_x + 2 * dx;
            int next_y = current_y + 2 * dy;

            // Check if jump is within board bounds
            if (next_x >= 0 && next_x < 8 && next_y >= 0 && next_y < 8 &&
                board[next_x][next_y] == EMPTY) { // Landing spot must be empty

                int captured_piece = board[mid_x][mid_y];
                gboolean is_opponent_piece = FALSE;

                if (player_side == WHITE && (captured_piece == BMAN || captured_piece == BKING)) {
                    is_opponent_piece = TRUE;
                } else if (player_side == BLACK && (captured_piece == WMAN || captured_piece == WKING)) {
                    is_opponent_piece = TRUE;
                }

                if (is_opponent_piece) {
                    found_further_jump = TRUE;

                    // Simulate the jump on a temporary board
                    int temp_board[8][8];
                    copy_board(board, temp_board);
                    temp_board[next_x][next_y] = temp_board[current_x][current_y]; // Move piece
                    temp_board[current_x][current_y] = EMPTY;                     // Clear old position
                    temp_board[mid_x][mid_y] = EMPTY;                             // Remove captured piece

                    // Add captured coord to current path
                    Coord captured_coord = {mid_x, mid_y};
                    GArray *next_captured_coords = g_array_copy(current_captured_coords);
                    g_array_append_val(next_captured_coords, captured_coord);

                    // Check for kinging after this jump
                    int next_piece_type = piece;
                    if (piece == WMAN && next_y == 0) {
                        next_piece_type = WKING;
                    } else if (piece == BMAN && next_y == 7) {
                        next_piece_type = BKING;
                    }
                    temp_board[next_x][next_y] = next_piece_type; // Update piece type if kinged

                    // Recurse from the new position
                    find_all_jump_paths_recursive(temp_board, next_x, next_y, initial_x, initial_y,
                                                   player_side, next_captured_coords, all_jump_moves_found);

                    g_array_free(next_captured_coords, TRUE); // Free the copied array after recursion
                }
            }
        }
    }

    // If no further jumps are found from this position, and we have captured at least one piece,
    // then this path represents a complete multi-jump move.
    if (!found_further_jump && current_captured_coords->len > 0) {
        Move final_move;
        final_move.x1 = initial_x;
        final_move.y1 = initial_y;
        final_move.x2 = current_x;
        final_move.y2 = current_y;
        final_move.captured_coords = g_array_copy(current_captured_coords); // Copy to store in Move
        final_move.is_jump_sequence = TRUE;
        g_array_append_val(all_jump_moves_found, final_move);
    }
}


/**
 * @brief Gets all possible legal moves (including multi-jumps) for a given player on a given board.
 * Prioritizes jump moves if any are available.
 * @param board The board state to analyze.
 * @param player_side The side of the player (WHITE or BLACK).
 * @return GArray of Move structures. Caller is responsible for freeing the GArray and its contained GArray* captured_coords.
 */
static GArray* get_all_possible_moves_for_player(int board[8][8], int player_side) {
    GArray *all_moves = g_array_new(FALSE, FALSE, sizeof(Move));
    GArray *jump_moves_found = g_array_new(FALSE, FALSE, sizeof(Move));

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int piece = board[x][y];
            if ((player_side == WHITE && (piece == WMAN || piece == WKING)) ||
                (player_side == BLACK && (piece == BMAN || piece == BKING))) {

                // Find all jump paths starting from this piece
                GArray *current_captured_coords = g_array_new(FALSE, FALSE, sizeof(Coord));
                find_all_jump_paths_recursive(board, x, y, x, y, player_side,
                                               current_captured_coords, jump_moves_found);
                g_array_free(current_captured_coords, TRUE); // Free the temporary array
            }
        }
    }

    if (jump_moves_found->len > 0) {
        // If jumps are available, only return jump moves
        for (guint i = 0; i < jump_moves_found->len; i++) {
            Move m = g_array_index(jump_moves_found, Move, i);
            g_array_append_val(all_moves, m);
        }
    } else {
        // If no jumps, find standard moves
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int piece = board[x][y];
                if ((player_side == WHITE && (piece == WMAN || piece == WKING)) ||
                    (player_side == BLACK && (piece == BMAN || piece == BKING))) {

                    int dx_options[] = {-1, 1};
                    int dy_options[] = {-1, 1};

                    if (piece == WMAN) {
                        dy_options[1] = -1; // Only move up
                    } else if (piece == BMAN) {
                        dy_options[0] = 1; // Only move down
                    }

                    for (int i = 0; i < 2; i++) {
                        for (int j = 0; j < 2; j++) {
                            int dx = dx_options[i];
                            int dy = dy_options[j];

                            if (piece == WMAN && dy == 1) continue;
                            if (piece == BMAN && dy == -1) continue;

                            int next_x = x + dx;
                            int next_y = y + dy;

                            if (next_x >= 0 && next_x < 8 && next_y >= 0 && next_y < 8 &&
                                board[next_x][next_y] == EMPTY) {
                                // Standard non-capture move
                                Move m;
                                m.x1 = x;
                                m.y1 = y;
                                m.x2 = next_x;
                                m.y2 = next_y;
                                m.captured_coords = g_array_new(FALSE, FALSE, sizeof(Coord)); // Empty for non-jump
                                m.is_jump_sequence = FALSE;
                                g_array_append_val(all_moves, m);
                            }
                        }
                    }
                }
            }
        }
    }
    g_array_free(jump_moves_found, TRUE); // Free the temporary array
    return all_moves;
}


/**
 * @brief Applies a move (which can be a multi-jump sequence) to a given board state.
 * This function modifies the provided board.
 * @param board The board state to modify.
 * @param move The Move structure defining the action.
 * @return TRUE if the move was successful, FALSE otherwise.
 */
static gboolean apply_move_to_board(int board[8][8], const Move *move) {
    int piece = board[move->x1][move->y1];

    // Perform the move (piece movement)
    board[move->x2][move->y2] = piece;
    board[move->x1][move->y1] = EMPTY;

    // Remove all captured pieces in the sequence
    for (guint i = 0; i < move->captured_coords->len; i++) {
        Coord captured = g_array_index(move->captured_coords, Coord, i);
        board[captured.x][captured.y] = EMPTY;
    }

    // Check for kinging
    if (piece == WMAN && move->y2 == 0) { // White man reaches top row (y=0)
        board[move->x2][move->y2] = WKING;
    } else if (piece == BMAN && move->y2 == 7) { // Black man reaches bottom row (y=7)
        board[move->x2][move->y2] = BKING;
    }
    return TRUE;
}


/**
 * @brief Checks if white has won the game.
 * @return TRUE if white has won, FALSE otherwise.
 */
static gboolean check_white_win() {
    int black_pieces = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board_state[x][y] == BMAN || board_state[x][y] == BKING) {
                black_pieces++;
            }
        }
    }
    // White wins if no black pieces left, or black has no valid moves
    if (black_pieces == 0) return TRUE;

    // Check if black has any valid moves
    GArray *black_moves = get_all_possible_moves_for_player(board_state, BLACK);
    gboolean black_has_moves = (black_moves->len > 0);
    for (guint i = 0; i < black_moves->len; i++) {
        Move m = g_array_index(black_moves, Move, i);
        g_array_free(m.captured_coords, TRUE);
    }
    g_array_free(black_moves, TRUE);
    
    return !black_has_moves; // White wins if black has no moves
}

/**
 * @brief Checks if black has won the game.
 * @return TRUE if black has won, FALSE otherwise.
 */
static gboolean check_black_win() {
    int white_pieces = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board_state[x][y] == WMAN || board_state[x][y] == WKING) {
                white_pieces++;
            }
        }
    }
    // Black wins if no white pieces left, or white has no valid moves
    if (white_pieces == 0) return TRUE;

    // Check if white has any valid moves
    GArray *white_moves = get_all_possible_moves_for_player(board_state, WHITE);
    gboolean white_has_moves = (white_moves->len > 0);
    for (guint i = 0; i < white_moves->len; i++) {
        Move m = g_array_index(white_moves, Move, i);
        g_array_free(m.captured_coords, TRUE);
    }
    g_array_free(white_moves, TRUE);

    return !white_has_moves; // Black wins if white has no moves
}

/**
 * @brief Checks for a draw by threefold repetition.
 * A draw occurs if the same board position occurs three or more times.
 * @return TRUE if a threefold repetition is detected, FALSE otherwise.
 */
static gboolean check_threefold_repetition() {
    guint64 current_hash = compute_hash(board_state, current_side);
    int count = 0;
    for (guint i = 0; i < zobrist_history->len; i++) {
        guint64 past_hash = g_array_index(zobrist_history, guint64, i);
        if (past_hash == current_hash) {
            count++;
        }
    }
    return count >= 3;
}


/**
 * @brief Evaluates the current board state from White's perspective.
 * Positive scores favor White, negative scores favor Black.
 * @param board The board state to evaluate.
 * @return An integer score representing the board's value.
 */
static int evaluate_board(int board[8][8]) {
    int score = 0;
    int white_men = 0;
    int white_kings = 0;
    int black_men = 0;
    int black_kings = 0;
    int total_pieces = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            switch (board[x][y]) {
                case WMAN:  white_men++; total_pieces++; break;
                case BMAN:  black_men++; total_pieces++; break;
                case WKING: white_kings++; total_pieces++; break;
                case BKING: black_kings++; total_pieces++; break;
            }
        }
    }

    // Material count with increased king value
    score += (white_men * 100);
    score += (white_kings * 400); // Kings are even more valuable
    score -= (black_men * 100);
    score -= (black_kings * 400);

    // Positional advantages
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y] == WMAN) {
                // Reward white men for moving forward (closer to king row 0)
                score += (7 - y) * 15; // Increased positional bonus for men to king
            } else if (board[x][y] == BMAN) {
                // Reward black men for moving forward (closer to king row 7)
                score -= y * 15; // Increased positional bonus for men to king
            }
            
            // Center control bonus (for both men and kings)
            // Reward pieces closer to the center columns (3, 4)
            if (board[x][y] == WMAN || board[x][y] == WKING) {
                if (x >= 2 && x <= 5) score += 20; // Broader and higher center bonus
            } else if (board[x][y] == BMAN || board[x][y] == BKING) {
                if (x >= 2 && x <= 5) score -= 20; // Broader and higher center bonus
            }

            // King safety/mobility (discourage kings from being on the edge unless necessary)
            if (board[x][y] == WKING) {
                if (x == 0 || x == 7 || y == 0 || y == 7) score -= 10; // Increased penalty for edge kings
                // Bonus for kings that are central and have more mobility (simplified)
                if (x > 0 && x < 7 && y > 0 && y < 7) score += 10;
            } else if (board[x][y] == BKING) {
                if (x == 0 || x == 7 || y == 0 || y == 7) score += 10; // Increased bonus for black if white king is on edge
                if (x > 0 && x < 7 && y > 0 && y < 7) score -= 10;
            }
        }
    }

    // Endgame specific heuristics
    // If few pieces left, prioritize kinging and attacking opponent's kings/men
    if (total_pieces <= 6) { // Endgame threshold
        // Aggressive capture bonus in endgame
        score += (white_kings * 50); // Small bonus for having kings in endgame
        score -= (black_kings * 50);

        // Encourage attacking opponent's pieces if you have more kings
        if (white_kings > black_kings) {
            score += (black_men + black_kings) * 20; // Reward for remaining opponent pieces
        } else if (black_kings > white_kings) {
            score -= (white_men + white_kings) * 20;
        }

        // Encourage cornering opponent kings (simplified)
        if (black_kings == 1) { // If black has only one king left
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (board[x][y] == BKING) {
                        // Reward white for black king being in a corner
                        if ((x == 0 && y == 0) || (x == 7 && y == 0) || (x == 0 && y == 7) || (x == 7 && y == 7)) {
                            score += 100;
                        }
                    }
                }
            }
        }
        if (white_kings == 1) { // If white has only one king left
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (board[x][y] == WKING) {
                        // Reward black for white king being in a corner
                        if ((x == 0 && y == 0) || (x == 7 && y == 0) || (x == 0 && y == 7) || (x == 7 && y == 7)) {
                            score -= 100;
                        }
                    }
                }
            }
        }
    }


    // Check for win/loss conditions (very high/low scores)
    if (black_men == 0 && black_kings == 0) return INT_MAX; // White wins
    if (white_men == 0 && white_kings == 0) return INT_MIN; // Black wins

    return score;
}

/**
 * @brief Minimax algorithm with Alpha-Beta Pruning and Transposition Table.
 * @param board The current board state.
 * @param depth The remaining depth for the search.
 * @param alpha The alpha value (best score found for maximizing player so far).
 * @param beta The beta value (best score found for minimizing player so far).
 * @param is_maximizing_player TRUE if it's the maximizing player's turn (White), FALSE for minimizing (Black).
 * @return The best score achievable from this state.
 */
static int minimax(int board[8][8], int depth, int alpha, int beta, gboolean is_maximizing_player) {
    guint64 current_hash = compute_hash(board, is_maximizing_player ? WHITE : BLACK);
    int original_alpha = alpha;
    int score_from_tt;

    // Transposition Table Lookup
    if (lookup_transposition_table(current_hash, depth, alpha, beta, &score_from_tt)) {
        return score_from_tt;
    }

    // Base case: If depth is 0 or game is over, return the board's evaluation
    if (depth == 0) {
        int eval = evaluate_board(board);
        store_transposition_table(current_hash, eval, depth, TT_FLAG_EXACT);
        return eval;
    }

    // Check for win/loss conditions at current depth
    int white_pieces = 0;
    int black_pieces = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y] == WMAN || board[x][y] == WKING) white_pieces++;
            if (board[x][y] == BMAN || board[x][y] == BKING) black_pieces++;
        }
    }
    if (black_pieces == 0) {
        store_transposition_table(current_hash, INT_MAX, depth, TT_FLAG_EXACT);
        return INT_MAX; // White wins
    }
    if (white_pieces == 0) {
        store_transposition_table(current_hash, INT_MIN, depth, TT_FLAG_EXACT);
        return INT_MIN; // Black wins
    }


    if (is_maximizing_player) { // White's turn (maximizing)
        int max_eval = INT_MIN;
        GArray *possible_moves = get_all_possible_moves_for_player(board, WHITE);

        if (possible_moves->len == 0) { // No moves for maximizing player, could be a draw or loss
            int eval = evaluate_board(board);
            g_array_free(possible_moves, TRUE);
            store_transposition_table(current_hash, eval, depth, TT_FLAG_EXACT);
            return eval;
        }

        for (guint i = 0; i < possible_moves->len; i++) {
            Move move = g_array_index(possible_moves, Move, i);
            int new_board[8][8];
            copy_board(board, new_board); // Create a copy of the board
            apply_move_to_board(new_board, &move); // Pass address of move

            int eval = minimax(new_board, depth - 1, alpha, beta, FALSE); // Recurse for minimizing player
            if (eval > max_eval) {
                max_eval = eval;
            }
            g_array_free(move.captured_coords, TRUE); // Free captured_coords for this move

            alpha = (alpha > max_eval) ? alpha : max_eval; // Replaced G_MAX
            if (beta <= alpha) {
                // Beta cutoff
                store_transposition_table(current_hash, max_eval, depth, TT_FLAG_LOWERBOUND);
                g_array_free(possible_moves, TRUE);
                return max_eval;
            }
        }
        g_array_free(possible_moves, TRUE);
        store_transposition_table(current_hash, max_eval, depth, TT_FLAG_EXACT); // Exact score
        return max_eval;
    } else { // Black's turn (minimizing)
        int min_eval = INT_MAX;
        GArray *possible_moves = get_all_possible_moves_for_player(board, BLACK);

        if (possible_moves->len == 0) { // No moves for minimizing player, could be a draw or win
            int eval = evaluate_board(board);
            g_array_free(possible_moves, TRUE);
            store_transposition_table(current_hash, eval, depth, TT_FLAG_EXACT);
            return eval;
        }

        for (guint i = 0; i < possible_moves->len; i++) {
            Move move = g_array_index(possible_moves, Move, i);
            int new_board[8][8];
            copy_board(board, new_board); // Create a copy of the board
            apply_move_to_board(new_board, &move); // Pass address of move

            int eval = minimax(new_board, depth - 1, alpha, beta, TRUE); // Recurse for maximizing player
            if (eval < min_eval) {
                min_eval = eval;
            }
            g_array_free(move.captured_coords, TRUE); // Free captured_coords for this move

            beta = (beta < min_eval) ? beta : min_eval; // Replaced G_MIN
            if (beta <= alpha) {
                // Alpha cutoff
                store_transposition_table(current_hash, min_eval, depth, TT_FLAG_UPPERBOUND);
                g_array_free(possible_moves, TRUE);
                return min_eval;
            }
        }
        g_array_free(possible_moves, TRUE);
        store_transposition_table(current_hash, min_eval, depth, TT_FLAG_EXACT); // Exact score
        return min_eval;
    }
}

/**
 * @brief Finds the best move for the AI using the minimax algorithm for a given player.
 * @param current_board The current state of the game board.
 * @param player_side The side of the player (WHITE or BLACK) for whom to find the best move.
 * @return The best move found, or a default invalid move if no moves are possible.
 */
static Move find_best_move(int current_board[8][8], int player_side) {
    int best_score = (player_side == WHITE) ? INT_MIN : INT_MAX;
    Move best_move = {-1, -1, -1, -1, NULL, FALSE}; // Initialize with an invalid move and NULL GArray

    // Determine search depth dynamically
    int total_pieces = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (current_board[x][y] != EMPTY) {
                total_pieces++;
            }
        }
    }

    int search_depth = 4; // Default depth
    if (total_pieces <= 6) { // If 6 or fewer pieces, search deeper for endgame
        search_depth = 10; // Increased depth for endgame
        g_print("AI: Entering endgame. Increasing search depth to %d.\n", search_depth);
    } else if (total_pieces <= 10) { // Mid-game with fewer pieces
        search_depth = 7; // Increased depth for mid-game
        g_print("AI: Mid-game. Increasing search depth to %d.\n", search_depth);
    }


    GArray *possible_moves = get_all_possible_moves_for_player(current_board, player_side);

    if (possible_moves->len == 0) {
        g_print("AI (%s): No possible moves found.\n", (player_side == WHITE) ? "White" : "Black");
        g_array_free(possible_moves, TRUE);
        return best_move;
    }

    g_print("AI (%s): Evaluating %d possible moves with depth %d...\n", (player_side == WHITE) ? "White" : "Black", possible_moves->len, search_depth);

    for (guint i = 0; i < possible_moves->len; i++) {
        Move move = g_array_index(possible_moves, Move, i);
        int temp_board[8][8];
        copy_board(current_board, temp_board); // Create a temporary board for simulation
        apply_move_to_board(temp_board, &move); // Pass address of move

        // Call minimax for the resulting board state, assuming the opponent will play optimally
        int score = minimax(temp_board, search_depth - 1, INT_MIN, INT_MAX, (player_side == WHITE) ? FALSE : TRUE); // Next turn is opponent's

        g_print("AI (%s): Move %d-%d (x1:%d,y1:%d to x2:%d,y2:%d) -> Score: %d\n",
                (player_side == WHITE) ? "White" : "Black",
                xy_to_diagonal(move.x1, move.y1), xy_to_diagonal(move.x2, move.y2),
                move.x1, move.y1, move.x2, move.y2, score);


        if ((player_side == WHITE && score > best_score) || (player_side == BLACK && score < best_score)) {
            best_score = score;
            // Deep copy the move and its captured_coords
            if (best_move.captured_coords != NULL) {
                g_array_free(best_move.captured_coords, TRUE); // Free previous captured_coords
            }
            best_move = move; // Copy the struct
            best_move.captured_coords = g_array_copy(move.captured_coords); // Deep copy the GArray
        } else {
            // If not the best move, free its captured_coords immediately
            g_array_free(move.captured_coords, TRUE);
        }
    }
    g_array_free(possible_moves, TRUE); // Free the outer array
    return best_move;
}


/**
 * @brief Resets the game to its initial state.
 * This function is connected to the "Reset Game" button.
 */
static void reset_game() {
    init_board();
    current_side = BLACK; // Game starts with Black's turn
    consecutive_non_capture_moves = 0; // Reset draw counter
    
    // Clear and re-initialize zobrist_history
    if (zobrist_history != NULL) {
        g_array_free(zobrist_history, TRUE);
    }
    zobrist_history = g_array_new(FALSE, FALSE, sizeof(guint64));
    
    update_all_display();
    print_board();
    gtk_label_set_text(GTK_LABEL(status_label), "Game Reset. Black's Turn (AI)");
    
    // Clear human input fields if they are visible
    if (game_mode == MODE_HUMAN_VS_AI) {
        gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(from_entry)), "", -1);
        gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(to_entry)), "", -1);
    }
    g_print("Game Reset.\n");
}

/**
 * @brief Sets the current game mode and updates UI visibility.
 * @param mode The new game mode (MODE_HUMAN_VS_AI or MODE_AI_VS_AI).
 */
static void set_game_mode(int mode) {
    game_mode = mode;
    reset_game(); // Reset board when changing mode

    if (game_mode == MODE_HUMAN_VS_AI) {
        gtk_widget_set_visible(human_input_box, TRUE);
        gtk_widget_set_visible(human_make_move_button, TRUE);
        gtk_widget_set_visible(next_ai_move_button, FALSE); // Hide AI vs AI button
        gtk_label_set_text(GTK_LABEL(status_label), "Game Mode: Human vs AI. Black's Turn (Human)");
    } else { // MODE_AI_VS_AI
        gtk_widget_set_visible(human_input_box, FALSE);
        gtk_widget_set_visible(human_make_move_button, FALSE);
        gtk_widget_set_visible(next_ai_move_button, TRUE); // Show AI vs AI button
        gtk_label_set_text(GTK_LABEL(status_label), "Game Mode: AI vs AI. Black's Turn (AI)");
    }
}

/**
 * @brief Handles the "Human vs AI" mode button click.
 */
static void on_mode_human_vs_ai_clicked(GtkButton *button, gpointer user_data) {
    set_game_mode(MODE_HUMAN_VS_AI);
}

/**
 * @brief Handles the "AI vs AI" mode button click.
 */
static void on_mode_ai_vs_ai_clicked(GtkButton *button, gpointer user_data) {
    set_game_mode(MODE_AI_VS_AI);
}


/**
 * @brief Handles the "Make Move" button click for human input.
 */
static void on_human_make_move_button_clicked(GtkButton *button, gpointer user_data) {
    if (game_mode != MODE_HUMAN_VS_AI) {
        return; // This button is only active in Human vs AI mode
    }

    if (current_side != BLACK) {
        gtk_label_set_text(GTK_LABEL(status_label), "It's White's turn (AI)! Please wait or switch mode.");
        return;
    }

    const char *from_text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(from_entry)));
    const char *to_text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(to_entry)));

    int from_diag = atoi(from_text);
    int to_diag = atoi(to_text);

    int x1, y1, x2, y2;

    if (!diagonal_to_xy(from_diag, &x1, &y1)) {
        gtk_label_set_text(GTK_LABEL(status_label), g_strdup_printf("Invalid 'From' square: %s", from_text));
        return;
    }
    if (!diagonal_to_xy(to_diag, &x2, &y2)) {
        gtk_label_set_text(GTK_LABEL(status_label), g_strdup_printf("Invalid 'To' square: %s", to_text));
        return;
    }

    g_print("Attempting human move (diagonal): %d (%d,%d) to %d (%d,%d)\n", from_diag, x1, y1, to_diag, x2, y2);

    // Create a temporary move to validate against possible moves
    Move player_attempt_move = {x1, y1, x2, y2, NULL, FALSE};
    GArray *possible_player_moves = get_all_possible_moves_for_player(board_state, BLACK);
    gboolean is_valid_player_move = FALSE;
    Move actual_player_move; // To store the full move details from possible_player_moves

    for (guint i = 0; i < possible_player_moves->len; i++) {
        Move m = g_array_index(possible_player_moves, Move, i);
        if (m.x1 == player_attempt_move.x1 && m.y1 == player_attempt_move.y1 &&
            m.x2 == player_attempt_move.x2 && m.y2 == player_attempt_move.y2) {
            is_valid_player_move = TRUE;
            actual_player_move = m; // Copy the full move details
            break;
        } else {
            g_array_free(m.captured_coords, TRUE); // Free if not the selected move
        }
    }

    if (is_valid_player_move) {
        // Check if the human move was a capture
        if (actual_player_move.captured_coords->len > 0) {
            consecutive_non_capture_moves = 0; // Reset counter on capture
        } else {
            consecutive_non_capture_moves++; // Increment if no capture
        }

        apply_move_to_board(board_state, &actual_player_move);
        g_array_free(actual_player_move.captured_coords, TRUE); // Free after applying

        // Add current board hash to history for threefold repetition check
        guint64 current_board_hash = compute_hash(board_state, current_side);
        g_array_append_val(zobrist_history, current_board_hash);


        g_print("Human move successful!\n");
        current_side = WHITE; // Switch to AI's turn
        update_all_display();
        print_board();

        if (check_white_win()) {
            gtk_label_set_text(GTK_LABEL(status_label), "WHITE WINS!");
            g_print("WHITE WINS!\n");
        } else if (check_black_win()) {
            gtk_label_set_text(GTK_LABEL(status_label), "BLACK WINS!");
            g_print("BLACK WINS!\n");
        } else if (consecutive_non_capture_moves >= 40) { // Check for draw by 40-move rule
            gtk_label_set_text(GTK_LABEL(status_label), "Draw by 40-move rule (no capture)!");
            g_print("Draw by 40-move rule (no capture)!\n");
        } else if (check_threefold_repetition()) { // Check for draw by threefold repetition
            gtk_label_set_text(GTK_LABEL(status_label), "Draw by threefold repetition!");
            g_print("Draw by threefold repetition!\n");
        }
        else {
            gtk_label_set_text(GTK_LABEL(status_label), "Human moved. White (AI) is thinking...");
            gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(from_entry)), "", -1);
            gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(to_entry)), "", -1);
            
            // Immediately trigger AI's move after human move
            on_next_ai_move_button_clicked(NULL, NULL);
        }
    } else {
        gtk_label_set_text(GTK_LABEL(status_label), "Invalid move. Try again.");
        g_print("Invalid human move.\n");
    }
    g_array_free(possible_player_moves, TRUE);
}


/**
 * @brief Handles the "Next AI Move" button click.
 * This function will alternate turns between two AI players, or trigger AI's turn in Human vs AI mode.
 */
static void on_next_ai_move_button_clicked(GtkButton *button, gpointer user_data) {
    // Check for win condition before making a move
    if (check_white_win()) {
        gtk_label_set_text(GTK_LABEL(status_label), "WHITE WINS!");
        g_print("WHITE WINS!\n");
        return;
    } else if (check_black_win()) {
        gtk_label_set_text(GTK_LABEL(status_label), "BLACK WINS!");
        g_print("BLACK WINS!\n");
        return;
    }

    Move ai_move;
    if (current_side == BLACK) {
        gtk_label_set_text(GTK_LABEL(status_label), "Black (AI) is thinking...");
        ai_move = find_best_move(board_state, BLACK);
    } else { // current_side == WHITE
        gtk_label_set_text(GTK_LABEL(status_label), "White (AI) is thinking...");
        ai_move = find_best_move(board_state, WHITE);
    }

    if (ai_move.x1 != -1) { // If a valid AI move was found
        g_print("AI (%s) is making move: %d-%d\n",
                (current_side == WHITE) ? "White" : "Black",
                xy_to_diagonal(ai_move.x1, ai_move.y1), xy_to_diagonal(ai_move.x2, ai_move.y2));
        
        // Check if the AI move was a capture
        if (ai_move.captured_coords->len > 0) {
            consecutive_non_capture_moves = 0; // Reset counter on capture
        } else {
            consecutive_non_capture_moves++; // Increment if no capture
        }

        apply_move_to_board(board_state, &ai_move);
        g_array_free(ai_move.captured_coords, TRUE); // Free after applying

        // Add current board hash to history for threefold repetition check
        guint64 current_board_hash = compute_hash(board_state, current_side);
        g_array_append_val(zobrist_history, current_board_hash);

        g_print("AI move successful!\n");
        // Switch turns
        current_side = (current_side == WHITE) ? BLACK : WHITE;
        update_all_display(); // Update the display after a successful AI move
        print_board(); // Print board state to console

        // Re-check for win condition after the move
        if (check_white_win()) {
            gtk_label_set_text(GTK_LABEL(status_label), "WHITE WINS!");
            g_print("WHITE WINS!\n");
        } else if (check_black_win()) {
            gtk_label_set_text(GTK_LABEL(status_label), "BLACK WINS!");
            g_print("BLACK WINS!\n");
        } else if (consecutive_non_capture_moves >= 40) { // Check for draw by 40-move rule
            gtk_label_set_text(GTK_LABEL(status_label), "Draw by 40-move rule (no capture)!");
            g_print("Draw by 40-move rule (no capture)!\n");
        } else if (check_threefold_repetition()) { // Check for draw by threefold repetition
            gtk_label_set_text(GTK_LABEL(status_label), "Draw by threefold repetition!");
            g_print("Draw by threefold repetition!\n");
        }
        else {
            if (game_mode == MODE_AI_VS_AI) {
                gtk_label_set_text(GTK_LABEL(status_label), (current_side == WHITE) ? "White's Turn (AI)" : "Black's Turn (AI)");
            } else { // Human vs AI
                gtk_label_set_text(GTK_LABEL(status_label), "White (AI) moved. Black's Turn (Human).");
            }
        }
    } else {
        // No valid moves for the current AI player
        if (current_side == BLACK) {
            gtk_label_set_text(GTK_LABEL(status_label), "Black (AI) has no valid moves. White wins!");
            g_print("Black (AI) has no valid moves. White wins!\n");
        } else {
            gtk_label_set_text(GTK_LABEL(status_label), "White (AI) has no valid moves. Black wins!");
            g_print("White (AI) has no valid moves. Black wins!\n");
        }
    }
}


/**
 * @brief Loads CSS from a string and applies it to the default display.
 * @param css_data The CSS string.
 */
static void load_css(const char *css_data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    gtk_css_provider_load_from_string(provider, css_data);
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/**
 * @brief Creates the main game window and UI elements.
 * @param app The GtkApplication instance.
 */
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *main_vbox;
    GtkWidget *mode_selection_hbox; // New HBox for mode selection buttons
    GtkWidget *mode_human_vs_ai_button;
    GtkWidget *mode_ai_vs_ai_button;
    GtkWidget *control_hbox;
    GtkWidget *reset_button;

    // Load CSS
    load_css(
        "window {"
        "  background-color: #333;"
        "}"
        "label {"
        "  color: #eee;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "}"
        "entry {"
        "  padding: 5px;"
        "  border-radius: 5px;"
        "  border: 1px solid #555;"
        "  background-color: #444;"
        "  color: #eee;"
        "}"
        "button {"
        "  background-image: none;"
        "  background-color: #555;"
        "  color: #eee;"
        "  border-radius: 5px;"
        "  padding: 10px 15px;"
        "  font-weight: bold;"
        "  margin: 5px;"
        "  transition: all 0.2s ease-in-out;"
        "}"
        "button:hover {"
        "  background-color: #777;"
        "}"
        "button:active {"
        "  background-color: #333;"
        "}"
        ".board-grid {"
        "  border: 2px solid #555;"
        "  margin: 20px;"
        "}"
        ".cell-container {"
        "  min-width: 60px;"
        "  min-height: 60px;"
        "  display: flex;"
        "  align-items: center;"
        "  justify-content: center;"
        "  border: 1px solid rgba(0,0,0,0.1);" // Subtle border for cells
        "}"
        ".light-square {"
        "  background-color: #F0D9B5;" // Light wood color
        "}"
        ".dark-square {"
        "  background-color: #B58863;" // Dark wood color
        "}"
        ".selected {"
        "  background-color: #FFD700; /* Gold color for selected cell */"
        "  border: 2px solid #FFD700;"
        "}"
        ".wman { background-color: white; color: black; border-radius: 50%; }"      
        ".bman { background-color: black; color: white; border-radius: 50%; }"     
        ".wking { background-color: white; color: black; border-radius: 50%; border: 6px solid gold; }"       
        ".bking { background-color: black; color: white; border-radius: 50%; border: 6px solid silver; }"
        ".empty {"
        "  background-image: none;" // No image for empty squares
        "}"
        // Updated CSS for diagonal numbers
        ".diagonal-number-label {"
        "  font-size: 18px;" // 18px font size for  readability
        "  color: #FF69B4;"   // Pink color for high contrast
        "  font-weight: bold;"
        "  text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.7);" // Dark shadow for pink text
        "  padding: 2px;"
        "}"
    );

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Gtk4 Checkers (Minimax)");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 700);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); // Keep window fixed size for now

    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_vbox);
    gtk_widget_set_margin_start(main_vbox, 20);
    gtk_widget_set_margin_end(main_vbox, 20);
    gtk_widget_set_margin_top(main_vbox, 20);
    gtk_widget_set_margin_bottom(main_vbox, 20);

    status_label = gtk_label_new("Welcome to Checkers! Select Game Mode.");
    gtk_box_append(GTK_BOX(main_vbox), status_label);

    // Mode selection buttons
    mode_selection_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(mode_selection_hbox, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_vbox), mode_selection_hbox);

    mode_human_vs_ai_button = gtk_button_new_with_label("Human vs. AI");
    g_signal_connect(mode_human_vs_ai_button, "clicked", G_CALLBACK(on_mode_human_vs_ai_clicked), NULL);
    gtk_box_append(GTK_BOX(mode_selection_hbox), mode_human_vs_ai_button);

    mode_ai_vs_ai_button = gtk_button_new_with_label("AI vs. AI");
    g_signal_connect(mode_ai_vs_ai_button, "clicked", G_CALLBACK(on_mode_ai_vs_ai_clicked), NULL);
    gtk_box_append(GTK_BOX(mode_selection_hbox), mode_ai_vs_ai_button);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 0);
    gtk_widget_add_css_class(grid, "board-grid");
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER); // Center the board horizontally
    gtk_box_append(GTK_BOX(main_vbox), grid);

    // Initialize board UI
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            GtkWidget *cell_overlay = gtk_overlay_new(); // Main overlay for the cell
            GtkWidget *cell_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // For square background
            gtk_widget_add_css_class(cell_container, "cell-container");
            // Force the cell container to be a fixed size to ensure children have space
            gtk_widget_set_size_request(cell_container, 60, 60);
            gtk_overlay_set_child(GTK_OVERLAY(cell_overlay), cell_container); // Cell container is the base

            // Use GtkImage for pieces
            GtkWidget *piece_image = gtk_image_new();
            gtk_image_set_pixel_size(GTK_IMAGE(piece_image), 50); // Set pixel size for the image
            gtk_box_append(GTK_BOX(cell_container), piece_image);

            // Store references
            cell_containers[x][y] = cell_container;
            cell_images[x][y] = piece_image; // Store reference to GtkImage

            // Add diagonal number label as an overlay on the cell_overlay
            int diag_num = xy_to_diagonal(x, y);
            if (diag_num > 0) { // Only dark squares have diagonal numbers
                GtkWidget *diag_label = gtk_label_new(NULL);
                gtk_label_set_text(GTK_LABEL(diag_label), g_strdup_printf("%d", diag_num));
                gtk_widget_add_css_class(diag_label, "diagonal-number-label");
                
                gtk_overlay_add_overlay(GTK_OVERLAY(cell_overlay), diag_label); // Add label to cell_overlay
                gtk_widget_set_halign(diag_label, GTK_ALIGN_END);   // Align to end (right)
                gtk_widget_set_valign(diag_label, GTK_ALIGN_START); // Align to start (top)
                gtk_widget_set_margin_end(diag_label, 2);  // Small margin from right edge
                gtk_widget_set_margin_top(diag_label, 2);  // Small margin from top edge
                diagonal_labels[x][y] = diag_label; // Store reference
            } else {
                diagonal_labels[x][y] = NULL; // No label for light squares
            }

            gtk_grid_attach(GTK_GRID(grid), cell_overlay, x, y, 1, 1); // Attach the cell_overlay to the grid
        }
    }

    // Input fields and buttons for human player
    human_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(human_input_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_vbox), human_input_box);

    from_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(from_entry), "From (1-32)");
    gtk_box_append(GTK_BOX(human_input_box), from_entry);

    to_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(to_entry), "To (1-32)");
    gtk_box_append(GTK_BOX(human_input_box), to_entry);

    human_make_move_button = gtk_button_new_with_label("Make Move");
    g_signal_connect(human_make_move_button, "clicked", G_CALLBACK(on_human_make_move_button_clicked), NULL);
    gtk_box_append(GTK_BOX(human_input_box), human_make_move_button);

    // Button for AI vs AI mode or to trigger AI move in human vs AI
    control_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(control_hbox, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_vbox), control_hbox);

    next_ai_move_button = gtk_button_new_with_label("Next AI Move");
    g_signal_connect(next_ai_move_button, "clicked", G_CALLBACK(on_next_ai_move_button_clicked), NULL);
    gtk_box_append(GTK_BOX(control_hbox), next_ai_move_button);

    reset_button = gtk_button_new_with_label("Reset Game");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_game), NULL);
    gtk_box_append(GTK_BOX(main_vbox), reset_button);

    // Initial game setup
    init_zobrist_keys(); // Initialize Zobrist keys once
    zobrist_history = g_array_new(FALSE, FALSE, sizeof(guint64)); // Initialize history array
    
    init_board();
    set_game_mode(MODE_HUMAN_VS_AI); // Start in Human vs AI mode by default
    print_board();

    gtk_window_present(GTK_WINDOW(window));
}

/**
 * @brief Main function to run the GTK application.
 */
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.checkersai", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Free the zobrist_history array when the application exits
    if (zobrist_history != NULL) {
        g_array_free(zobrist_history, TRUE);
    }

    return status;
}
