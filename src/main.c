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

/*
 * Use MAKEFILE to compile
 *
*/

#include <gtk/gtk.h>
#include <glib/gstdio.h>  //needed for g_mkdir

#include "gameboard.h"

#define WHITE 5
#define BLACK 6

static int m_index=0;
static int m_x1=0;
static int m_y1=0;
static int m_x2=0;
static int m_y2=0;

static gboolean m_first_click=TRUE;
static int m_side =WHITE;
//declarations

//callbks
static void callbk_board_cell_selected(GameBoard *board, gpointer user_data);
static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void restart_game (GtkButton *button,  gpointer user_data);

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
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), "Version 0.2.0");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),"Copyright © 2024");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),"Board Game");
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_LGPL_2_1);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),"https://github.com/crispinprojects/");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog),"Project Website");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);	
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), "help-contents-symbolic");
	
	gtk_widget_set_visible (about_dialog, TRUE);

}

//----------------------------------------------------------------------
// Board callbks
//----------------------------------------------------------------------

static void callbk_board_cell_selected(GameBoard* gameboard, gpointer user_data)
{
	char *game_str="";
	GtkWidget *window =user_data;
	GtkWidget *gamelabel = g_object_get_data(G_OBJECT(window), "window-gamelabel-key");
	
	if(m_first_click && m_side ==WHITE) 
	{		
	////first click  player
	m_x1 = game_board_get_xpos(GAME_BOARD(gameboard));
	m_y1 = game_board_get_ypos(GAME_BOARD(gameboard));
	//g_print("m_x1 =%d m_y1=%d\n",m_x1,m_y1);
	
	m_first_click=FALSE;	
	return;
    }
    else if(!m_first_click && m_side ==WHITE) 
    {
     m_x2 = game_board_get_xpos(GAME_BOARD(gameboard));
	 m_y2 = game_board_get_ypos(GAME_BOARD(gameboard));
	 //g_print("m_x2 =%d m_y2=%d\n",m_x2,m_y2);
	
	   if(game_board_is_white_move_valid(m_x1,m_y1,m_x2,m_y2))	
	   {
		//move valid make and return with first click true
		int player =game_board_get_player(m_x1, m_y1);		
		gboolean second_capture = game_board_move_player(m_x1,m_y1,m_x2,m_y2,player);	
	    game_board_redraw(GAME_BOARD(gameboard));
	
	   if(second_capture ==TRUE) 
	   {		
	    gtk_label_set_text(GTK_LABEL(gamelabel), "Another White Capture Possible");
	    m_first_click=TRUE;
	    m_side=WHITE;			
	    return;
	    }
	   else
	   {
		   //no second capture
		   m_first_click=TRUE;
		   m_side=BLACK;
		   
		  gboolean white_win= game_board_check_white_win();	    
	      if(white_win)  gtk_label_set_text(GTK_LABEL(gamelabel),"White wins");
	      else gtk_label_set_text(GTK_LABEL(gamelabel),"AI move");
	      
	      if(m_side ==BLACK) 
	      {	
	      //AI move
	      game_board_make_AI_move();			
	      game_board_redraw(GAME_BOARD(gameboard));
	      	     
	      gboolean black_win= game_board_check_black_win();	    
	      if(black_win)  gtk_label_set_text(GTK_LABEL(gamelabel),"Black(AI) wins");
	      else gtk_label_set_text(GTK_LABEL(gamelabel),"White to move");
	      
	      m_first_click=TRUE;
	      m_side=WHITE;
	      return;
		  }   
	    }//no white second capture
	    } //if white valid
	    else
        {
		//move not valid white move start all over again
		gtk_label_set_text(GTK_LABEL(gamelabel),"White move illegal -click piece to reset");
		m_first_click=TRUE;
		m_side=WHITE;
		return;		 
	    }//else not valid
        
	 } //else if second click white
	
}

//====================================================================
static void restart_game (GtkButton *button,  gpointer user_data)
{
  //g_print("Restart Game\n");
  
  GtkWidget *gamelabel = g_object_get_data(G_OBJECT(button), "button-restart-gamelabel-key");
   
  m_first_click=TRUE;
  m_side =WHITE;
     
  GameBoard *gameboard =user_data;
  game_board_reset();
  game_board_redraw(GAME_BOARD(gameboard));
  gtk_label_set_text(GTK_LABEL(gamelabel), "White to move");   
}

//----------------------------------------------------------------------

static void startup(GtkApplication *app)
{
	 //g_print("startup  called todo\n");	
}

//---------------------------------------------------------------------
static void activate (GtkApplication *app, gpointer  user_data)
{
	GtkWidget *window;
	GtkWidget *header;	
	GtkWidget *gameboard; 
	GtkWidget *box;	
	GtkWidget *gamelabel; //information
	GtkWidget * button_restart;	
	GtkWidget *menu_button;
					
	// create a new window, and set its title
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Checkers");
	gtk_window_set_default_size(GTK_WINDOW (window),610,400);
	gtk_window_set_icon_name (GTK_WINDOW (window), "face-wink-symbolic");
	
	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (window), box);

	//Create game label	
	gamelabel = gtk_label_new("White to move.");
	g_object_set_data(G_OBJECT(window), "window-gamelabel-key",gamelabel);
	
	//Create game board
	gameboard = game_board_new();		
	g_signal_connect(GAME_BOARD(gameboard), "cell-selected", G_CALLBACK(callbk_board_cell_selected), window);
		
	button_restart = gtk_button_new_from_icon_name ("view-refresh-symbolic");
	//button_restart = gtk_button_new_with_label ("Restart");
	g_signal_connect (button_restart, "clicked", G_CALLBACK (restart_game), gameboard);
	g_object_set_data(G_OBJECT(button_restart), "button-restart-gamelabel-key",gamelabel);
	
	gtk_box_append(GTK_BOX(box), gameboard);
	gtk_box_append(GTK_BOX(box), gamelabel);
	
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
	
	gtk_header_bar_pack_end(GTK_HEADER_BAR (header), menu_button);
	gtk_window_set_titlebar(GTK_WINDOW(window), header);
	
	gtk_window_present (GTK_WINDOW (window));  //use present with gtk4 
	
}

int main (int  argc, char **argv)
{		
	GtkApplication *app;
	int status;
	app = gtk_application_new ("org.gtk.gamebaord", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect_swapped(app, "startup", G_CALLBACK (startup),app);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}
