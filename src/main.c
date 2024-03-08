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

//====================================================================
// GTK4 Checkers 
// Author: Alan Crispin <crispinalan@gmail.com> 
// Date: March 2024
// use make file to compile
//====================================================================


#include <gtk/gtk.h>

#include "gameboard.h"
#include "pieces.h"
#include "move.h"

static int m_index=0;
static int m_xpos1=0;
static int m_ypos1=0;
static int m_xpos2=0;
static int m_ypos2=0;
static int m_player=WMAN;
static gboolean m_human_done=FALSE; 
static gboolean m_ai_done=TRUE; 

static gboolean m_first_click=TRUE;

//declarations
//callbks
static void callbk_board_cell_selected(GameBoard *board, gpointer user_data);
static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void update_label_game(GameBoard *board, gpointer user_data);

//=====================================================================

static void callbk_about(GSimpleAction * action, GVariant *parameter, gpointer user_data){


	GtkWidget *window = user_data;

	const gchar *authors[] = {"Alan Crispin", NULL};
	GtkWidget *about_dialog;
	about_dialog = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(about_dialog),GTK_WINDOW(window));
	gtk_widget_set_size_request(about_dialog, 200,200);
    gtk_window_set_modal(GTK_WINDOW(about_dialog),TRUE);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "GTK4 Checkers");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), "Version 0.1.0");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),"Copyright © 2024");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),"Board Game");
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_LGPL_2_1);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),"https://github.com/crispinprojects/");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog),"Project Website");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);	
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), "help-contents-symbolic");	
	gtk_widget_set_visible (about_dialog, TRUE);
}

//======================================================================
static void callbk_board_cell_selected(GameBoard* board, gpointer user_data)
{
	GtkWindow *window =user_data;
	GtkWidget *label_game =g_object_get_data(G_OBJECT(window), "window-label-game-key");	
	char *label_str="";
	char *start_str="";
	char *to_str ="";
	char *player_str="";
	char *ai_str="";
	
	if(m_ai_done)
	{
	
	if(m_first_click) 
	{		
	//first click white player
	m_xpos1 = game_board_get_xpos(GAME_BOARD(board));
	m_ypos1 = game_board_get_ypos(GAME_BOARD(board));	
	m_player = game_board_get_player(m_xpos1,m_ypos1);
	start_str =g_strdup_printf("%s%s%s%s%s","(", g_strdup_printf("%i", m_xpos1),",",g_strdup_printf("%i", m_ypos1), ")");
	player_str =g_strdup_printf("%s%s", "player = ",g_strdup_printf("%i", m_player));
		
	label_str="Selected Piece: ";
	label_str = g_strconcat(label_str,player_str, " ", start_str, NULL);	
	gtk_label_set_text(GTK_LABEL(label_game), label_str);
				
	m_first_click=FALSE;	
	return;	
    }
    else  //not first click
    {
	
	m_xpos2 = game_board_get_xpos(GAME_BOARD(board));
	m_ypos2 = game_board_get_ypos(GAME_BOARD(board));
	
	if(game_board_is_white_move_valid(m_xpos1,m_ypos1,m_xpos2,m_ypos2))	
	{
	
	gboolean second_capture = game_board_move_player(m_xpos1,m_ypos1,m_xpos2,m_ypos2,m_player);	
	game_board_redraw(GAME_BOARD(board));
	
	if(second_capture ==TRUE) 
	{
	label_str="Another Capture Possible ";		
	gtk_label_set_text(GTK_LABEL(label_game), label_str);
	m_first_click=TRUE;
	m_human_done=FALSE;			
	return;
	}
	else
	{		
		m_first_click=TRUE;
	    m_human_done=TRUE;
	    m_ai_done=FALSE;	
			    
	    gboolean white_win= game_board_check_white_win();
	    
	    if(white_win) 
	    {
	    label_str="Human (White) Wins";	 
	    }		
	    else
	    {
	    label_str="AI to Move ";
	    }
	    
	    gtk_label_set_text(GTK_LABEL(label_game), label_str);			    
		
	    return;
		
	} //else no white second capture
    
    } //white move valid
    else
    {
    //white move not valid
    label_str="White Move Illegal ";		
	gtk_label_set_text(GTK_LABEL(label_game), label_str);
	m_first_click=TRUE;
	m_human_done=FALSE;
	game_board_redraw(GAME_BOARD(board));
	return;
	}
    
    } //else not first clcik
	
    }//m_ai_done		
}

//====================================================================
static void restart_game (GtkButton *button,  gpointer user_data)
{
  //g_print("Restart Game\n");
  
  GtkWidget *label_game = g_object_get_data(G_OBJECT(button), "button-restart-label-key"); //object-key association 
  
  m_player=WMAN;
  m_first_click=TRUE;
  m_human_done=FALSE; 
  m_ai_done=TRUE;
  
  GameBoard *board =user_data;
  game_board_reset();
  game_board_redraw(GAME_BOARD(board));
    
  char *label_str="WHITE to move. Click piece start position. Then click move position.";
  gtk_label_set_text(GTK_LABEL(label_game), label_str);
  
}
//=====================================================================

void make_ai_move(GameBoard *board, gpointer user_data)
{
	GtkWidget *label_game=user_data;
	
	if(m_human_done) //only run AI when human move done
	{
	
	char *label_str="";
	char *ai_move_str="";
	char *ai_capture_str="";
	char *next_move_str="";
	
	Move ai_move = game_board_make_AI_move();
	game_board_redraw(GAME_BOARD(board));
			
	if(ai_move.capture ==1)
	{
		gboolean second_capture_ai =game_board_can_capture(ai_move.x2,ai_move.y2);
		//g_print("make_ai_move: second capture = %d\n", second_capture_ai);
		
		if(second_capture_ai)
		{		
		label_str="AI can make another capture. Press AI move button again.";	  
	    gtk_label_set_text(GTK_LABEL(label_game), label_str);	
	    m_ai_done=FALSE;    
	    return;		
		}
		else {		
		ai_move_str =g_strdup_printf("%s%s%s%s%s%s%s%s%s"," AI Move: (", 
	    g_strdup_printf("%i", ai_move.x1),",",g_strdup_printf("%i", ai_move.y1), ") -> (",
	    g_strdup_printf("%i", ai_move.x2),",",g_strdup_printf("%i", ai_move.y2), ")" );	
		next_move_str=" White to move.";
		label_str = g_strconcat(label_str, ai_move_str, next_move_str, NULL);	
	    gtk_label_set_text(GTK_LABEL(label_game), label_str);
		} //no next capture		
	} //if ai capture
	else
	{
		ai_move_str =g_strdup_printf("%s%s%s%s%s%s%s%s%s"," AI Move: (", 
	    g_strdup_printf("%i", ai_move.x1),",",g_strdup_printf("%i", ai_move.y1), ") -> (",
	    g_strdup_printf("%i", ai_move.x2),",",g_strdup_printf("%i", ai_move.y2), ")" );	
		next_move_str=" White to move.";
		label_str = g_strconcat(label_str, ai_move_str, next_move_str, NULL);	
	    gtk_label_set_text(GTK_LABEL(label_game), label_str);
	}
	
	gboolean black_win= game_board_check_black_win();
	
	if(black_win) 
	{
		label_str="Computer AI (Black) Wins";	   
	    gtk_label_set_text(GTK_LABEL(label_game), label_str);
	}
		
	m_ai_done=TRUE;
	m_human_done=FALSE;
	m_first_click=TRUE;
		
   } //human done check
	
}

//======================================================================
static void callbk_ai_move (GtkButton *button, gpointer user_data)
{
		
	GameBoard *board =user_data;	
	GtkWidget *label_game =g_object_get_data(G_OBJECT(button), "button-ai-label-key");	
	make_ai_move(board, label_game);		
}

//======================================================================

static void startup(GtkApplication *app)
{
	 //g_print("startup  called\n");	
}

//======================================================================
static void activate (GtkApplication *app, gpointer  user_data)
{
	GtkWidget *window;
	GtkWidget *header;	
	GtkWidget *board; 
	GtkWidget *box;	
	GtkWidget *label_game; // game information
	GtkWidget * button_restart;	
	GtkWidget * button_ai_move;
		
	GtkWidget *menu_button;
	
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Checkers");
	gtk_window_set_default_size(GTK_WINDOW (window),600,350);
	
	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (window), box);
	
	label_game = gtk_label_new("<b>WHITE to move.</b> Click piece start position. Then click move position.");
	gtk_label_set_use_markup(GTK_LABEL(label_game), TRUE);
	
	gtk_label_set_xalign(GTK_LABEL(label_game), 0.5);
	
	//Create game board
	board = game_board_new();		
	g_signal_connect(GAME_BOARD(board), "cell-selected", G_CALLBACK(callbk_board_cell_selected), window);
		
	button_restart = gtk_button_new_from_icon_name ("view-refresh-symbolic");
	gtk_widget_set_tooltip_text(button_restart, "Restart Game");
	//button_restart = gtk_button_new_with_label ("Restart");
	g_signal_connect (button_restart, "clicked", G_CALLBACK (restart_game), board);
		
	button_ai_move = gtk_button_new_with_label ("AI Move");
	gtk_widget_set_tooltip_text(button_restart, "Computer Move");
	g_signal_connect (button_ai_move, "clicked", G_CALLBACK (callbk_ai_move), board);
	
	
	g_object_set_data(G_OBJECT(window), "window-label-game-key",label_game);
			
	g_object_set_data(G_OBJECT(button_restart), "button-restart-label-key",label_game);	
	g_object_set_data(G_OBJECT(button_ai_move), "button-ai-label-key",label_game);
		
	gtk_box_append(GTK_BOX(box), label_game);
	gtk_box_append(GTK_BOX(box), board);
	
	//Actions
	GSimpleAction *about_action;
	about_action=g_simple_action_new("about",NULL); //app.about
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about_action)); //make visible
	g_signal_connect(about_action, "activate",  G_CALLBACK(callbk_about), window);
	
	// Menu model
	GMenu *menu, *section;
	menu = g_menu_new();
	
	section = g_menu_new ();
	g_menu_append (section, "About", "app.about");
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);
	
	//Now hamburger style menu button
	menu_button = gtk_menu_button_new();
	gtk_widget_set_tooltip_text(menu_button, "Menu");
	gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (menu_button),"open-menu-symbolic"); 		
	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button), G_MENU_MODEL(menu));
	
	header = gtk_header_bar_new ();
	gtk_header_bar_pack_start (GTK_HEADER_BAR (header), button_restart);

	gtk_header_bar_pack_start (GTK_HEADER_BAR (header), button_ai_move);	
	gtk_header_bar_pack_end(GTK_HEADER_BAR (header), menu_button);
	gtk_window_set_titlebar(GTK_WINDOW(window), header);
	
	gtk_window_present (GTK_WINDOW (window));  //use present with gtk4 
	
}
//======================================================================
int main (int  argc, char **argv)
{		
	GtkApplication *app;
	int status;
	app = gtk_application_new ("org.gtk.checkers", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect_swapped(app, "startup", G_CALLBACK (startup),app);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}
