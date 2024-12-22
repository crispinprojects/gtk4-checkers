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
// Mx Checkers version 0.1
// Author: Alan Crispin <crispinalan@gmail.com> 
// Date: Christmas December 2024
// use make file to compile
//====================================================================

#include <ncurses.h>
#include <stdlib.h>
#include "gameboard.h"

#define HUMAN 1
#define AI 2
#define UNKNOWN 3


extern WINDOW  *info_win;
//extern tells the compiler that pointer to info_win, 
//is defined elsewhere (i.e. in gameboard) and will be 
//connected with the linker. An extern variable is used when a 
//particular file needs to access a variable from another file.

void clear_info_window()
{
	mvwprintw(info_win, 2, 2, "                                        ");
	mvwprintw(info_win, 3, 2, "                                        ");
	mvwprintw(info_win, 4, 2, "                                        ");
	mvwprintw(info_win, 5, 2, "                                         ");
	mvwprintw(info_win, 6, 2, "                                         ");
	mvwprintw(info_win, 7, 2, "                                         ");
	mvwprintw(info_win, 8, 2, "                                         ");
	mvwprintw(info_win, 9, 2, "                                         ");
	mvwprintw(info_win, 10, 2, "                                         ");
	mvwprintw(info_win, 11, 2, "                                         ");
	wrefresh(info_win);
}

void play_ding()
{
	char * command_str ="aplay -q ding.wav";       
    system(command_str);  
}

void play_win()
{
	char * command_str ="aplay -q win.wav";       
    system(command_str);  
}

int main(int argc, char **argv)
{

	int board[BOARD_SIZE][BOARD_SIZE] = {{0,0}};
		
	int human_x1=0;
	int human_y1=0;
	int human_x2=0;
	int human_y2=0;
	
	int ai_x1=0;
	int ai_y1=0;
	int ai_x2=0;
	int ai_y2=0;
	
	int x=1;
	int y=0;
	bool m_deep_search=FALSE;
	bool watch_monitor=TRUE;
	bool m_first_press=TRUE;
	int m_side =HUMAN;	
	bool play_sound=TRUE;
	int ch=0;	
	
	int max_depth=10;

	init_board(board);
	init_all();
	
	create_info_win();
	mvwprintw(info_win, 2, 2, "Mx Checkers v0.1.0                      ");
	mvwprintw(info_win, 3, 2, "A key: AI move                          ");
	mvwprintw(info_win, 4, 2, "D key: Deep Search                      ");
	mvwprintw(info_win, 5, 2, "H key: Help                             ");
	mvwprintw(info_win, 6, 2, "I key: Redraw INFO window               ");
	mvwprintw(info_win, 7, 2, "M key: Monitor                          ");
	mvwprintw(info_win, 8, 2, "R key: Restart Game                      ");
	mvwprintw(info_win, 9, 2, "S key: Sound On/Off                      ");
	mvwprintw(info_win, 10, 2, "Spacebar: Select Piece                   ");
	mvwprintw(info_win, 11, 2, "HUMAN TO MOVE                           ");		
	wrefresh(info_win);
	
	//gameloop
	while(1){
		draw_board(board, x,y);
				
		ch = getch();		
				
		if(ch == KEY_LEFT){
			if(x-1 >= 0) x--;			
		}

		else if(ch == KEY_RIGHT){
			if(x+1 <= 7) x++;		
		}

		else if(ch == KEY_UP){
			if(y+1 <= 7) y++;			
		}

		else if(ch == KEY_DOWN){
			if(y-1 >= 0) y--;			
		}
		
		else if(ch == 100){ //d key
			 
			 if(m_deep_search ==FALSE)
			 {
				 max_depth=14;
				 m_deep_search=TRUE; //toggle
			 }
			 else {
				 max_depth=10;
				 m_deep_search=FALSE; //toggle				 
			 }
			 
			clear_info_window();			
			if(m_deep_search){	
			mvwprintw(info_win, 2, 2, "DEEP SEARCH ACTIVATED: WAIT FOR AI");
			mvwprintw(info_win, 3, 2, "SEARCH DEPTH: %d  ",max_depth);
			if(m_side ==HUMAN) 
			mvwprintw(info_win, 4, 2, "HUMAN TO MOVE                  ");
			else mvwprintw(info_win, 4, 2, "AI TO MOVE (PRESS A KEY)       ");	
		    } else
		    {
			mvwprintw(info_win, 2, 2, "DEEP SEARCH DEACTIVATED");
			mvwprintw(info_win, 3, 2, "SEARCH DEPTH: %d  ",max_depth);
			if(m_side ==HUMAN) 
			mvwprintw(info_win, 4, 2, "HUMAN TO MOVE                  ");
			else mvwprintw(info_win, 4, 2, "AI TO MOVE (PRESS A KEY)       ");
			}
			wrefresh(info_win);
		}
		
		else if(ch == 109){ //m key
			//monitor -todo
			if(watch_monitor ==FALSE)
			 {	
				 set_monitor(TRUE);
				 watch_monitor=TRUE;
				 clear_info_window();
				 mvwprintw(info_win, 2, 2, "DEPTH MONITOR ON ");
				 if(m_side ==HUMAN) 				 
			     mvwprintw(info_win, 3, 2, "HUMAN TO MOVE                  ");			
			     else 
			     mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY)       ");
			     wrefresh(info_win);
			 }
			 else {
				  
				 //deep_monitor=FALSE; //toggle
				 set_monitor(FALSE);
				 watch_monitor=FALSE;
				 clear_info_window();
				 mvwprintw(info_win, 2, 2, "DEPTH MONITOR OFF ");
				 if(m_side ==HUMAN) 				 
			     mvwprintw(info_win, 3, 2, "HUMAN TO MOVE                  ");			
			     else 
			     mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY)       ");
			     wrefresh(info_win);				 
			 }
			
		}
		
		else if(ch == 104){ //h key
			
			clear_info_window();
			mvwprintw(info_win, 2, 2, "Help");
			mvwprintw(info_win, 3, 2, "A key: AI move                          ");
			mvwprintw(info_win, 4, 2, "D key: Deep Search                      ");
			mvwprintw(info_win, 5, 2, "H key: Help                             ");
			mvwprintw(info_win, 6, 2, "I key: Redraw INFO window               ");
			mvwprintw(info_win, 7, 2, "M key: Monitor                          ");
			mvwprintw(info_win, 8, 2, "R key: Restart Game                      ");
			mvwprintw(info_win, 9, 2, "S key: Sound On/Off                      ");
			mvwprintw(info_win, 9, 2, "Spacebar: Select Piece                   ");
			
			if(m_side ==HUMAN) 
			mvwprintw(info_win, 10, 2, "HUMAN TO MOVE                  ");
			else mvwprintw(info_win, 10, 2, "AI TO MOVE (PRESS A KEY)       ");
			
			wrefresh(info_win);		
		}
		
		else if(ch == 105){ //i key
			//redraw info window
			create_info_win();
			mvwprintw(info_win, 2, 2, "INFO WINDOW RESTARTED");
			mvwprintw(info_win, 3, 2, "A key: AI move                          ");
			mvwprintw(info_win, 4, 2, "D key: Deep Search                      ");
			mvwprintw(info_win, 5, 2, "H key: Help                             ");
			mvwprintw(info_win, 6, 2, "I key: Redraw INFO window               ");
			mvwprintw(info_win, 7, 2, "M key: Monitor                          ");
			mvwprintw(info_win, 8, 2, "R key: Restart Game                      ");
			mvwprintw(info_win, 8, 2, "S key: Sound On/Off                      ");
			mvwprintw(info_win, 9, 2, "Spacebar: Select Piece                   ");
						
			if(m_side ==HUMAN) 
			     mvwprintw(info_win, 10, 2, "HUMAN TO MOVE                  ");
			else mvwprintw(info_win, 10, 2, "AI TO MOVE (PRESS A KEY)       ");
			
			wrefresh(info_win);		
		}
		
		else if(ch == 114){ //r key
		//restart game
		clear_board(board);
		init_board(board);	
		max_depth=8;	
		human_x1=0;
		human_y1=0;
		human_x2=0;
		human_y2=0;		
		ai_x1=0;
		ai_y1=0;
		ai_x2=0;
		ai_y2=0;		
		x=1;
		y=0;
		m_first_press=TRUE;	
		m_side =HUMAN;
		create_info_win();
		mvwprintw(info_win, 2, 2, "GAME RESTARTED                 ");
		mvwprintw(info_win, 3, 2, "A key: AI move                          ");
		mvwprintw(info_win, 4, 2, "D key: Deep Search                      ");
		mvwprintw(info_win, 5, 2, "H key: Help                             ");
		mvwprintw(info_win, 6, 2, "I key: Redraw INFO window               ");
		mvwprintw(info_win, 7, 2, "M key: Monitor                          ");
		mvwprintw(info_win, 8, 2, "R key: Restart Game                      ");
		mvwprintw(info_win, 9, 2, "S key: Sound On/Off                      ");
		mvwprintw(info_win, 10, 2, "Spacebar: select piece                   ");		
		mvwprintw(info_win, 11, 2, "HUMAN TO MOVE                  ");
		wrefresh(info_win);		
		}
		
		else if(ch == 115){ //s key
			
			if(play_sound ==FALSE)
			{
			play_sound=TRUE;		
			clear_info_window();
			mvwprintw(info_win, 2, 2, "SOUND ON ");
			if(m_side ==HUMAN) 				 
			mvwprintw(info_win, 3, 2, "HUMAN TO MOVE                  ");			
			else 
			mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY)       ");
			wrefresh(info_win);
			}
			else {			
			play_sound=FALSE; //toggle			
			clear_info_window();
			mvwprintw(info_win, 2, 2, "SOUND OFF ");
			if(m_side ==HUMAN) 				 
			mvwprintw(info_win, 3, 2, "HUMAN TO MOVE                  ");			
			else 
			mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY)       ");
			wrefresh(info_win);				 
			}
			} //s_key
		
		//AI player
		
		else if(ch == 97){ //a key (lower case) AI PLAYER
			
			if(m_side ==AI) 
			{
				
			if(is_white_win(board))
			{
			clear_info_window();
			mvwprintw(info_win, 2, 2, "HUMAN (WHITE) WINS    ");			
			mvwprintw(info_win, 3, 2, "GAME OVER. RESTART.        ");
			wrefresh(info_win);	
			if (play_sound) play_win();
			break;	
			}
			
			get_best_move_AI(board, 2, max_depth, &ai_x1, &ai_y1, &ai_x2, &ai_y2);
					
			
			if(!is_coord_valid(ai_x1,ai_y1) || !is_coord_valid(ai_x2,ai_y2)){
				//AI returned non-valid move				
				clear_info_window();	
				mvwprintw(info_win, 2, 2, "BLACK AI MOVE: (%d,%d)->(%d,%d)    ",ai_x1,ai_y1,ai_x2,ai_y2);			
				mvwprintw(info_win, 3, 2, " AI FAILED: SORRY GAME OVER.          ");
				//mvwprintw(info_win, 4, 2, "AI FAILED: SORRY GAME OVER.          ");
				wrefresh(info_win);	
				
			}
				
			bool ai_second_cap=move_black(board, ai_x1, ai_y1, ai_x2, ai_y2);
			refresh();
			
			if(is_black_win(board))
			{
			clear_info_window();
			mvwprintw(info_win, 2, 2, "BLACK AI MOVE: (%d,%d)->(%d,%d)    ",ai_x1,ai_y1,ai_x2,ai_y2);			
			mvwprintw(info_win, 3, 2, "AI (BLACK) WINS: GAME OVER.         ");
			wrefresh(info_win);	
			if (play_sound) play_win();	
			}
			else {			
			clear_info_window();
			mvwprintw(info_win, 2, 2, "BLACK AI MOVE: (%d,%d)->(%d,%d)   ",ai_x1,ai_y1,ai_x2,ai_y2);			
			mvwprintw(info_win, 3, 2, "HUMAN TO MOVE                      ");
			mvwprintw(info_win, 4, 2, "-------- CURRENT SCORE --------- ");
			mvwprintw(info_win, 5, 2, "HUMAN PAWN NUMBER: %d               ",get_number_WMAN(board));
			mvwprintw(info_win, 6, 2, "HUMAN KING NUMBER: %d               ",get_number_WKING(board));
		    mvwprintw(info_win, 7, 2, "AI PAWN NUMBER: %d                  ",get_number_BMAN(board));
			mvwprintw(info_win, 8, 2, "AI KING NUMBER: %d                  ",get_number_BKING(board));
			
			//mvwprintw(info_win, 4, 2, "OPPONENT CAN CAPTURE: %d          ",opponent_can_capture(board,ai_x2,ai_y2));
			wrefresh(info_win);	
		
		    }
			
			if(ai_second_cap)
			{
			clear_info_window();
			mvwprintw(info_win, 2, 2, "BLACK (AI) SECOND CAPTURE POSSIBLE");
			mvwprintw(info_win, 3, 2, "PRESS A KEY AGAIN                   ");						
			wrefresh(info_win);			
			m_side=AI;
			ai_x1=0; //x
			ai_y1=0; //y
			ai_x2=0;
			ai_y2=0;
				
			}//second capture
			else {
			m_first_press=TRUE;
			m_side=HUMAN;
			human_x1=0; //x
			human_y1=0; //y
			human_x2=0;
			human_y2=0;	
			x=1;
			y=0;
			if (play_sound) play_ding();	
			}
		  }//m_side=AI						
		} //A AI key
		
		else if(ch == 32){ //spacebar
				
		if(m_first_press && m_side ==HUMAN) //first click HUMAN player
		{	
		human_x1 = x;
		human_y1 = y;				
		m_first_press=FALSE;
		m_side=HUMAN;
		}//if first click
					
		else if(!m_first_press && m_side ==HUMAN) //second click HUMAN player
		{	
		human_x2 = x;
		human_y2 = y;
				
		if(is_white_move_valid(board,human_x1, human_y1, human_x2, human_y2))	
	    {
	     
	    bool human_second_cap=move_white(board, human_x1, human_y1, human_x2, human_y2);
		
		if(is_white_win(board))
		{
		clear_info_window();
		mvwprintw(info_win, 2, 2, "HUMAN MOVE (WHITE): (%d,%d)->(%d,%d)   ",human_x1,human_y1,human_x2,human_y2);		
		mvwprintw(info_win, 3, 2, "HUMAN WINS. GAME OVER.        ");
		wrefresh(info_win);
		if (play_sound) play_win();	
		}
		else {
		clear_info_window();
		mvwprintw(info_win, 2, 2, "HUMAN MOVE (WHITE): (%d,%d)->(%d,%d)     ",human_x1,human_y1,human_x2,human_y2);		
		
		if(m_deep_search){
		mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY & WAIT)      ");
		mvwprintw(info_win, 4, 2, "DEEP SEARCH (LONG TIME): MAXDEPTH = %d",max_depth);
		}
		else {
		mvwprintw(info_win, 3, 2, "AI TO MOVE (PRESS A KEY)              ");
		mvwprintw(info_win, 4, 2, "STANDARD SEARCH: MAXDEPTH = %d         ",max_depth);
	    }
		wrefresh(info_win);
	    }
		
		//check for second capture

		if(human_second_cap)
		{
		clear_info_window();
		mvwprintw(info_win, 2, 2, "HUMAN (WHITE) SECOND CAPTURE POSSIBLE. ");
		mvwprintw(info_win, 3, 2, "TAKE NEXT PIECE                        ");		
		wrefresh(info_win);
		m_first_press=TRUE;
		m_side=HUMAN;
		human_x1=0; //x
		human_y1=0; //y
		human_x2=0;
		human_y2=0;		
		}//second capture
		else //AI move
		{
		m_first_press=TRUE;
		m_side=AI; //AI to move
		human_x1=0; //x
		human_y1=0; //y
		human_x2=0;
		human_y2=0;	
		x=1;
		y=0;
		}//else WHITE second capture		
	    }
	    else
	    {
	    //white move not valid (start over)
	    clear_info_window();
	    mvwprintw(info_win, 2, 2, "HUMAN (WHITE) MOVE: (%d,%d)->(%d,%d)  ",human_x1,human_y1,human_x2,human_y2);
		mvwprintw(info_win, 3, 2, "HUMAN (WHITE) ILLEGAL MOVE. TRY AGAIN!               ");	
		wrefresh(info_win);
	    m_first_press=TRUE;
	    m_side=HUMAN;
	    human_x1=0; //x
	    human_y1=0; //y
	    human_x2=0;
	    human_y2=0;
	    x=1;
	    y=0;
		}
	} //else if !firstclick and human
						
	} //if spacebar

		refresh();
	} //gameloop
	
	refresh();
	wrefresh(info_win);	

	return 0;

}
//======================================================================
