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

#pragma once

#include <gtk/gtk.h>
#include <stdio.h>

G_BEGIN_DECLS

#define GAME_TYPE_BOARD (game_board_get_type ())
#define GAME_BOARD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAME_TYPE_BOARD, GameBoard))
#define GAME_IS_BOARD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAME_TYPE_BOARD))

typedef struct _GameBoard	 GameBoard;

GtkWidget *game_board_new (void);

GDK_AVAILABLE_IN_ALL GType	game_board_get_type(void) G_GNUC_CONST;

GDK_AVAILABLE_IN_ALL int game_board_get_xpos(GameBoard *board);
GDK_AVAILABLE_IN_ALL int game_board_get_ypos(GameBoard *board);
GDK_AVAILABLE_IN_ALL int game_board_get_player(int x, int y);

GDK_AVAILABLE_IN_ALL gboolean game_board_check_black_win();
GDK_AVAILABLE_IN_ALL gboolean game_board_check_white_win();

GDK_AVAILABLE_IN_ALL void game_board_reset();
GDK_AVAILABLE_IN_ALL void game_board_redraw(GameBoard *board);
GDK_AVAILABLE_IN_ALL gboolean game_board_move_player(int x1, int y1, int x2, int y2, int player);
GDK_AVAILABLE_IN_ALL gboolean game_board_is_white_move_valid(int x1, int y1, int x2, int y2);
GDK_AVAILABLE_IN_ALL gboolean game_board_is_black_move_valid(int x1, int y1, int x2, int y2);

GDK_AVAILABLE_IN_ALL void game_board_make_AI_move();

//properties

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GameBoard, g_object_unref)

G_END_DECLS
