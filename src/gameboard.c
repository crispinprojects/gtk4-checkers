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

#include "gameboard.h"

WINDOW *info_win;

#define EMPTY 0
#define WMAN 1
#define BMAN 2
#define WKING 3
#define BKING 4

#define PLAYER1 1 //white
#define PLAYER2 2  //black AI

bool depth_monitor =TRUE;

//======================================================================
void clear_board(int board[][BOARD_SIZE])
{	
	for (int x=0;x<BOARD_SIZE;x++)
	{
	for (int y=0;y<BOARD_SIZE;y++)
	{   
	board[x][y] =0;
	}
	}
}	

//======================================================================
void init_board(int board[][BOARD_SIZE]){
		
	//Black 
	board[1][7]=BMAN;
	board[3][7]=BMAN;
	board[5][7]=BMAN;
	board[7][7]=BMAN;
	
	board[0][6]=BMAN;
	board[2][6]=BMAN;
	board[4][6]=BMAN;
	board[6][6]=BMAN;
	
	board[1][5]=BMAN;
	board[3][5]=BMAN;
	board[5][5]=BMAN;
	board[7][5]=BMAN;
	
	//White(Green)
	board[0][0]=WMAN;
	board[2][0]=WMAN;
	board[4][0]=WMAN;
	board[6][0]=WMAN;
	
	board[1][1]=WMAN;
	board[3][1]=WMAN;
	board[5][1]=WMAN;
	board[7][1]=WMAN;
	
	board[0][2]=WMAN;
	board[2][2]=WMAN;
	board[4][2]=WMAN;
	board[6][2]=WMAN;	
}
//======================================================================
void init_all(void){
	initscr();
	noecho();
	cbreak();
	curs_set(FALSE);
    keypad(stdscr, TRUE); //call keypad to tell curses to interpret special keys
	start_color(); //set up colours
	//experimenting with colour pairs
	init_pair(1, COLOR_BLACK, COLOR_WHITE);	
	init_pair(2, COLOR_GREEN, COLOR_BLACK); //black with green crown	
	init_pair(3, COLOR_RED, COLOR_GREEN);//green with red crown
	init_pair(4, COLOR_MAGENTA, COLOR_BLUE);
	init_pair(5, COLOR_CYAN, COLOR_YELLOW);
	init_pair(6, COLOR_WHITE, COLOR_BLACK);
	init_pair(7, COLOR_BLACK, COLOR_CYAN);	
	bkgd(COLOR_PAIR(7)); // background 5, 6 or 7
	wrefresh(stdscr);
}
//=====================================================================
void draw_board(int board[][BOARD_SIZE], int cursor_x, int cursor_y)
{
	int i, y = 2, j;

	attron(COLOR_PAIR(1)); //attribute-on colour_pair black-white
	move(y,2);
	addch(ACS_ULCORNER); // upper left corner

	for(i=0;i<7;i++){
		addch(ACS_HLINE); //add horizontal line
		addch(ACS_HLINE);
		addch(ACS_HLINE);
		addch(ACS_TTEE); //Tee pointing down 
	}

	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);

	addch (ACS_URCORNER);// upper right corner

	for(j = 7; j >= 0 ; j--){
		y++;
		move(y,2);
		addch(ACS_VLINE); //add vertical line

		for(i=0;i<BOARD_SIZE;i++){
			if(cursor_x == i && cursor_y == j)
				attron(COLOR_PAIR(4)); //select curser
					
			
			else if(board[i][j] == 1 || board[i][j] == 3)
				attron(COLOR_PAIR(3)); //Black pieces
			else if(board[i][j] == 2 || board[i][j] == 4)
				attron(COLOR_PAIR(2)); //green pieces

			addch(' ');
			if(board[i][j] == 3 || board[i][j] == 4) {				
				addch(ACS_DIAMOND); //kings have a diamond crown
			}
			else
				addch(' ');
			addch(' ');
			attron(COLOR_PAIR(1));
			addch(ACS_VLINE); //add vertical line
		}

		y++;

		move(y,2);
		addch(ACS_LTEE); //Tee pointing right

		if(j!=0){

			for(i=0;i<7;i++){
				addch(ACS_HLINE);
				addch(ACS_HLINE);
				addch(ACS_HLINE);
				addch(ACS_PLUS); //large plus 
			}

			addch(ACS_HLINE);
			addch(ACS_HLINE);
			addch(ACS_HLINE);
			addch(ACS_RTEE);//Tee pointing left 
		}
	}

	move(y,2);

	addch(ACS_LLCORNER); // lower left corner

	for(i=0;i<7;i++){
		addch(ACS_HLINE);addch(ACS_HLINE);addch(ACS_HLINE);
		addch(ACS_BTEE);//Tee pointing up
	}

	addch(ACS_HLINE);addch(ACS_HLINE);addch(ACS_HLINE);
	addch(ACS_LRCORNER);// lower right corner

}
//======================================================================
void wCenterTitle(WINDOW *pwin, const char * title){
	int x, maxy, maxx, stringBOARD_SIZE;

	getmaxyx(pwin, maxy, maxx);

	stringBOARD_SIZE = (maxy-maxy)+4 + strlen(title);
	x = (maxx - stringBOARD_SIZE)/2;

	mvwaddch(pwin, 0, x, ACS_RTEE);
	waddch(pwin, ' ');
	waddstr(pwin, title);
	waddch(pwin, ' ');
	waddch(pwin, ACS_LTEE);
}
//======================================================================
void wclrscr(WINDOW * pwin){
	int y, x, maxy, maxx;
	getmaxyx(pwin, maxy, maxx);
	for(y=0; y < maxy; y++)
		for(x=0; x < maxx; x++)
			mvwaddch(pwin, y, x, ' ');
}

//======================================================================

void create_info_win()
{
	//make information window
	info_win = newwin(14, 44, 5, 38);
	wattrset(info_win, COLOR_PAIR(1));
	wclrscr(info_win);
	box(info_win, 0, 0);
	wCenterTitle(info_win, "INFO");
	touchwin(info_win);
	wrefresh(info_win);
}
//=====================================================================

void set_monitor(bool value)
{
	depth_monitor=value;
}


//=====================================================================
bool move_white(int board[][BOARD_SIZE], int x1, int y1, int x2, int y2)
{
	int piece =board[x1][y1];			
	int x3=0; //capture coordinates
	int y3=0; //capture coordinates	
	int white_king_line =7; //White moving up board
	bool another_capture =FALSE;
		
	if (abs(x2 - x1)==1 && abs(y2-y1)==1) //standard move
    {
	board[x1][y1] = 0;
	board[x2][y2] = piece;	
		//check for Kings with standard move	
		if (y2 == white_king_line && piece == WMAN) 
		{				
			board[x2][y2] = WKING;
		} 		
	} //if standard move
	
	else if(abs(x2 - x1) == 2 && abs(y2-y1) ==2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
					
        board[x_cap][y_cap] = 0;
        board[x2][y2] = piece;
        board[x1][y1] = 0;
                
        //check for Kings after pawn capture
        if (y2 ==white_king_line && piece == WMAN) {               
       
        board[x2][y2] =WKING;	    
        }       
        //check for another capture
        another_capture =can_capture(board,x2,y2, &x3,&y3);        
                	
	} //else capture move
	
	return another_capture;	
	
}
//=====================================================================
bool move_black(int board[][BOARD_SIZE], int x1, int y1, int x2, int y2)
{
	
	int piece =board[x1][y1];
	int x3=0; //capture coordinates
	int y3=0; //capture coordinates				
	
	int black_king_line =0; //black moving down board
	bool another_capture =FALSE;
		
	if (abs(x2 - x1)==1 && abs(y2-y1)==1) //standard move
    {
	board[x1][y1] = 0;
	board[x2][y2] = piece;	
		//check for Kings with standard move	
		if (y2 == black_king_line && piece== BMAN) 
		{				
			board[x2][y2] = BKING;
		} 		
	} //if standard move
	
	else if(abs(x2 - x1) == 2 && abs(y2-y1) ==2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
					
        board[x_cap][y_cap] = 0;
        board[x2][y2] = piece;
        board[x1][y1] = 0;
                
        //check for Kings after pawn capture
        if (y2 ==black_king_line && piece == BMAN) {               
       
        board[x2][y2] =BKING;	    
        }       
        //check for another capture
        another_capture =can_capture(board,x2,y2,&x3,&y3);        
                	
	} //else capture move
	
	return another_capture;	
	
}
//=====================================================================
bool is_white_move_valid(int board[BOARD_SIZE][BOARD_SIZE],int x1, int y1, int x2, int y2)
{	
	bool move_state= FALSE;
	
	int moves[100][4];
	int numMoves = 0;
	
	if (board[x1][y1] == WMAN || board[x1][y1] == WKING)
	{		
		get_possible_white_moves(board,moves,&numMoves);
		
		for (int i = 0; i < numMoves; ++i) {
		int currentx1 = moves[i][0];
		int currenty1 = moves[i][1];
		int currentx2 = moves[i][2];
		int currenty2 = moves[i][3];	    
		
		if(currentx1 ==x1 && currenty1==y1 && currentx2==x2 && currenty2==y2)
		{
		move_state=TRUE;
		}	
		} //for nummoves
		
	} //if WMAN or WKING			
	
  return move_state;
}
//======================================================================

void get_possible_white_moves(int board[BOARD_SIZE][BOARD_SIZE],int possibleMoves[100][4], int* numMoves)
{
	 int x_mov=0;
	 int y_mov=0;
	 int x_jump=0;
     int y_jump=0;
     int x_mid=0;
     int y_mid=0;
   
     int standard_moves[100][4]= {0};
     int num_standard_moves=0;
     
     int jump_moves[100][4]= {0};
     int num_jump_moves=0;
     
     int num_moves=0;
	 
	 for (int x=0;x<BOARD_SIZE;x++)
		{
		for (int y=0;y<BOARD_SIZE;y++)
		{   
			int player =board[x][y];			
			
			if (player ==WMAN)	
	        {
		    //jump1 case
		    x_jump=x-2;
		    y_jump=y+2;
		    if(x_jump>-1 && y_jump<BOARD_SIZE) {
		    x_mid=x-1;
		    y_mid=y+1;
		    if ((board[x][y] == WMAN) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
		    && (board[x_jump][y_jump] == EMPTY))
		    {	
		    //WMAN capture move
		    jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;		    					 
		    }
		    }//if jump1
		    //jump2 case
		    x_jump=x+2;
		    y_jump =y+2;
		    if((x_jump<BOARD_SIZE && y_jump<BOARD_SIZE)) {
		    x_mid =x+1;
		    y_mid =y+1;  
		    if ((board[x][y] == WMAN) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
		    && (board[x_jump][y_jump] == EMPTY))
		    {	
		    //WMAN capture move 
		    jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	    					 
		    }
		    }//if jump2
		    
		    //standard move 1
		    x_mov=x-1;
		    y_mov=y+1;
		    if(x_mov>-1 && y_mov<BOARD_SIZE) {			
		    if ((board[x][y] ==WMAN) && (board[x_mov][y_mov] == EMPTY))
		    {		   
		    standard_moves[num_standard_moves][0] =x;
			standard_moves[num_standard_moves][1] =y;
			standard_moves[num_standard_moves][2]=x_mov;
			standard_moves[num_standard_moves][3]=y_mov;
			num_standard_moves=num_standard_moves+1;
		    }
			}
		    //standard move 2
			x_mov=x+1;
		    y_mov=y+1;  		   
		    if((x_mov<BOARD_SIZE && y_mov<BOARD_SIZE)){ 		    
		    if ((board[x][y] == WMAN) && (board[x_mov][y_mov] == EMPTY))
		    {		
		    standard_moves[num_standard_moves][0] =x;
			standard_moves[num_standard_moves][1] =y;
			standard_moves[num_standard_moves][2]=x_mov;
			standard_moves[num_standard_moves][3]=y_mov;
			num_standard_moves=num_standard_moves+1;
		    }//if board
		    } //if standard move2	
		    
		    }//player WMAN
		   
		   else if (player==WKING)
	       {
	       //Kings have four possible jump moves
	       //jump1 
	       x_jump=x-2;
	       y_jump=y+2;
	       if((x_jump>-1 && y_jump<BOARD_SIZE)) {
	       x_mid=x-1;
	       y_mid=y+1;
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	
	       }
	       }//if x_jump y_jump
	       //jump2
	       x_jump=x+2;
	       y_jump =y+2;
	       if((x_jump<BOARD_SIZE && y_jump<BOARD_SIZE)) {
	       x_mid =x+1;
	       y_mid =y+1;              
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING) 
	       && (board[x_jump][y_jump] == EMPTY))
	       {
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	       
	          
	       }                
	       }//if jump		
	       //jump3
	       x_jump=x+2;
	       y_jump=y-2;
	       if((x_jump<BOARD_SIZE && y_jump>-1))
	       {
	       x_mid=x+1;
	       y_mid=y-1;
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {	
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;   	
	       }
	       }//if x_jump y_jump
	       //jump4
	       x_jump=x-2;
	       y_jump=y-2;
	       if((x_jump>-1 && y_jump>-1)) 
	       {
	       x_mid=x-1;
	       y_mid=y-1;
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {	       					 
	        jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;  				 
	       }
	       }//if x_jump y_jump
	       
	       //kings have 4 standard moves
	       //standard move1
	       x_mov=x-1;
	       y_mov=y+1;
	       if((x_mov>-1 && y_mov<BOARD_SIZE)) {			
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       //standard move2		
	       x_mov=x+1;
	       y_mov=y+1;
	       if((x_mov<BOARD_SIZE && y_mov<BOARD_SIZE)) {			
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       
	       //standard move3		
	       x_mov=x+1;
	       y_mov=y-1;
	       if((x_mov<BOARD_SIZE && y_mov>-1)) {			
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       
	       //standard move4
	       x_mov=x-1;
	       y_mov=y-1;
	       if((x_mov>-1 && y_mov>-1)) {			
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
	       { 
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov	       
		 }//player WKING			
		}//x
	}//y
	
	// if jumps possible just return these moves
	
	if (num_jump_moves > 0)
	{
		//load up jum moves
		//*numMoves =num_jump_moves;
		num_moves=num_jump_moves;		
		for (int i = 0; i < num_jump_moves; ++i) {
		int currentx1 = jump_moves[i][0];
		int currenty1 = jump_moves[i][1];
		int currentx2 = jump_moves[i][2];
		int currenty2 = jump_moves[i][3];
		
		possibleMoves[i][0]=currentx1;
		possibleMoves[i][1]=currenty1; 
		possibleMoves[i][2]=currentx2;
		possibleMoves[i][3]=currenty2;
		}//for		
	} //if jump move
	else {
		//load up standard move		
		num_moves=num_standard_moves;		
		for (int i = 0; i < num_standard_moves; ++i) {
		int currentx1 = standard_moves[i][0];
		int currenty1 = standard_moves[i][1];
		int currentx2 = standard_moves[i][2];
		int currenty2 = standard_moves[i][3];
		
		possibleMoves[i][0]=currentx1;
		possibleMoves[i][1]=currenty1; 
		possibleMoves[i][2]=currentx2;
		possibleMoves[i][3]=currenty2;
		}//for
	}//else standard move	
	
	*numMoves=num_moves;	
}

//======================================================================
bool is_black_move_valid(int board[BOARD_SIZE][BOARD_SIZE],int x1, int y1, int x2, int y2)
{	
	bool move_state= FALSE;
	
	int moves[100][4];
	int numMoves = 0;
	
	if (board[x1][y1] == BMAN || board[x1][y1] == BKING)
	{		
		get_possible_black_moves(board,moves,&numMoves);
		
		for (int i = 0; i < numMoves; ++i) {
		int currentx1 = moves[i][0];
		int currenty1 = moves[i][1];
		int currentx2 = moves[i][2];
		int currenty2 = moves[i][3];	    
		
		if(currentx1 ==x1 && currenty1==y1 && currentx2==x2 && currenty2==y2)
		{
		move_state=TRUE;
		}	
		} //for nummoves
		
	} //if BMAN or BKING
			
	
  return move_state;
}
//=====================================================================

void get_possible_black_moves(int board[BOARD_SIZE][BOARD_SIZE],int possibleMoves[100][4], int* numMoves)
{
	 int x_mov=0;
	 int y_mov=0;
	 int x_jump=0;
     int y_jump=0;
     int x_mid=0;
     int y_mid=0;
    
     int standard_moves[100][4]= {0};
     int num_standard_moves=0;
     
     int jump_moves[100][4]= {0};
     int num_jump_moves=0;
     
     int num_moves=0;
	 
	 for (int x=0;x<BOARD_SIZE;x++)
		{
		for (int y=0;y<BOARD_SIZE;y++)
		{   
			int player =board[x][y];		
			
			if (player ==BMAN)	
	        {
		    //jump1 case
		    x_jump=x+2;
		    y_jump=y-2;
		    if(x_jump<BOARD_SIZE && y_jump>-1) {
		    x_mid=x+1;
		    y_mid=y-1;
		    if ((board[x][y] == BMAN) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
		    && (board[x_jump][y_jump] == EMPTY))
		    {	
		    //BMAN capture move
		    jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;		    					 
		    }
		    }//if jump1
		    //jump2 case
		    x_jump=x-2;
		    y_jump =y-2;
		    if((x_jump>-1 && y_jump>-1)) {
		    x_mid =x-1;
		    y_mid =y-1;  
		    if ((board[x][y] == BMAN) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
		    && (board[x_jump][y_jump] == EMPTY))
		    {	
		    //WMAN capture move 
		    jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	    					 
		    }
		    }//if jump2
		    
		    //standard move 1
		    x_mov=x+1;
		    y_mov=y-1;
		    if(x_mov<BOARD_SIZE && y_mov>-1) {			
		    if ((board[x][y] ==BMAN) && (board[x_mov][y_mov] == EMPTY))
		    {		   
		    standard_moves[num_standard_moves][0] =x;
			standard_moves[num_standard_moves][1] =y;
			standard_moves[num_standard_moves][2]=x_mov;
			standard_moves[num_standard_moves][3]=y_mov;
			num_standard_moves=num_standard_moves+1;
		    }
			}
		    //standard move 2
			x_mov=x-1;
		    y_mov=y-1;  		   
		    if((x_mov>-1 && y_mov>-1)){ 		    
		    if ((board[x][y] == BMAN) && (board[x_mov][y_mov] == EMPTY))
		    {		
		    standard_moves[num_standard_moves][0] =x;
			standard_moves[num_standard_moves][1] =y;
			standard_moves[num_standard_moves][2]=x_mov;
			standard_moves[num_standard_moves][3]=y_mov;
			num_standard_moves=num_standard_moves+1;
		    }//if board
		    } //if standard move2	
		    
		    }//player BMAN
		   
		   else if (player==BKING)
	       {
	       //Kings have four possible jump moves
	       //jump1 
	       x_jump=x-2;
	       y_jump=y+2;
	       if((x_jump>-1 && y_jump<BOARD_SIZE)) {
	       x_mid=x-1;
	       y_mid=y+1;
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	
	       }
	       }//if x_jump y_jump
	       //jump2
	       x_jump=x+2;
	       y_jump =y+2;
	       if((x_jump<BOARD_SIZE && y_jump<BOARD_SIZE)) {
	       x_mid =x+1;
	       y_mid =y+1;              
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING) 
	       && (board[x_jump][y_jump] == EMPTY))
	       {
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;	       
	          
	       }                
	       }//if jump		
	       //jump3
	       x_jump=x+2;
	       y_jump=y-2;
	       if((x_jump<BOARD_SIZE && y_jump>-1))
	       {
	       x_mid=x+1;
	       y_mid=y-1;
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {	
			jump_moves[num_jump_moves][0] =x;
			jump_moves[num_jump_moves][1] =y;
			jump_moves[num_jump_moves][2]=x_jump;
			jump_moves[num_jump_moves][3]=y_jump;
			num_jump_moves=num_jump_moves+1;   	
	       }
	       }//if x_jump y_jump
	       //jump4
	       x_jump=x-2;
	       y_jump=y-2;
	       if((x_jump>-1 && y_jump>-1)) 
	       {
	       x_mid=x-1;
	       y_mid=y-1;
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
	       {	       					 
	       jump_moves[num_jump_moves][0] =x;
	       jump_moves[num_jump_moves][1] =y;
	       jump_moves[num_jump_moves][2]=x_jump;
	       jump_moves[num_jump_moves][3]=y_jump;
	       num_jump_moves=num_jump_moves+1;  				 
	       }
	       }//if x_jump y_jump
	       
	       //kings have 4 standard moves
	       //standard move1
	       x_mov=x-1;
	       y_mov=y+1;
	       if((x_mov>-1 && y_mov<BOARD_SIZE)) {			
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       //standard move2		
	       x_mov=x+1;
	       y_mov=y+1;
	       if((x_mov<BOARD_SIZE && y_mov<BOARD_SIZE)) {			
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       
	       //standard move3		
	       x_mov=x+1;
	       y_mov=y-1;
	       if((x_mov<BOARD_SIZE && y_mov>-1)) {			
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
	       {	       
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       
	       //standard move4
	       x_mov=x-1;
	       y_mov=y-1;
	       if((x_mov>-1 && y_mov>-1)) {			
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
	       { 
	       standard_moves[num_standard_moves][0] =x;
	       standard_moves[num_standard_moves][1] =y;
	       standard_moves[num_standard_moves][2]=x_mov;
	       standard_moves[num_standard_moves][3]=y_mov;
	       num_standard_moves=num_standard_moves+1;
	       }
	       }//if xmov ymov
	       
		 }//player WKING
			
		}//x
	}//y
		
	// if jumps possible just return these moves
	
	if (num_jump_moves > 0)
	{
		//load up jump moves	
		num_moves=num_jump_moves;		
		for (int i = 0; i < num_jump_moves; ++i) {
		int currentx1 = jump_moves[i][0];
		int currenty1 = jump_moves[i][1];
		int currentx2 = jump_moves[i][2];
		int currenty2 = jump_moves[i][3];
		
		possibleMoves[i][0]=currentx1;
		possibleMoves[i][1]=currenty1; 
		possibleMoves[i][2]=currentx2;
		possibleMoves[i][3]=currenty2;
		}//for		
	} //if jump move
	else {
		//load up standard move			
		num_moves=num_standard_moves;		
		for (int i = 0; i < num_standard_moves; ++i) {
		int currentx1 = standard_moves[i][0];
		int currenty1 = standard_moves[i][1];
		int currentx2 = standard_moves[i][2];
		int currenty2 = standard_moves[i][3];
		
		possibleMoves[i][0]=currentx1;
		possibleMoves[i][1]=currenty1; 
		possibleMoves[i][2]=currentx2;
		possibleMoves[i][3]=currenty2;
		}//for
	}//else standard move
		
	*numMoves=num_moves;	
	
}
//======================================================================

bool can_capture(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int *x_cap, int *y_cap)
{    
     if (board[x][y] ==WMAN)
     {        
        if (x-2>-1 && y+2<BOARD_SIZE){
	        if((board[x-1][y+1] == BMAN ||
	        board[x-1][y+1] == BKING) &&
	        (board[x-2][y+2] == EMPTY)){                
	        *x_cap =x-2;
	        *y_cap =y+2;	       
	        return TRUE;
	        }//if
			}
            
            if (x+2<BOARD_SIZE && y+2<BOARD_SIZE){
	            if ((board[x+1][y+1] == BMAN ||
	            board[x+1][y+1] == BKING)&&
	            board[x+2][y+2] == EMPTY){                 
	            *x_cap =x+2;
	            *y_cap =y+2;	            
	            return TRUE;
				}                
			}       
    } //WMAN
     
        
    if (board[x][y] ==BMAN)
    {
		 if (x+2<BOARD_SIZE && y-2>-1){
			 if ( (board[x+1][y-1] == WMAN ||
			 board[x+1][y-1] == WKING)&&
			 board[x+2][y-2] == EMPTY){                  
			 *x_cap =x+2;
			 *y_cap =y-2;                
			 return TRUE;
			 }//if
             }
             
        if (x-2>-1 && y-2>-1){
	        if ( (board[x-1][y-1] == WMAN ||
	        board[x-1][y-1] == WKING)&&
	        board[x-2][y-2] == EMPTY){
	        *x_cap =x-2;
	        *y_cap =y-2;
	        return TRUE;
	        }//if 
			}
			      
    }//BMAN
    
    if (board[x][y] ==WKING)
    {
        if (x-2>-1 && y+2<BOARD_SIZE){
	        if ( (board[x-1][y+1] == BMAN ||
	        board[x-1][y+1] == BKING)&&
	        board[x-2][y+2] == EMPTY){               
	        *x_cap =x-2;
	        *y_cap =y+2;	        
	        return TRUE;
	        }
			}
			
        if (x+2<BOARD_SIZE && y+2<BOARD_SIZE){
            if ( (board[x+1][y+1] == BMAN ||
                  board[x+1][y+1] == BKING)&&
                  board[x+2][y+2] == EMPTY){               
                 *x_cap =x+2;
                 *y_cap =y+2;                
                return TRUE;
			}//if
		}

        if (x+2<BOARD_SIZE && y-2>-1){
	        if ((board[x+1][y-1] == BMAN ||
	        board[x+1][y-1] == BKING)&&
	        board[x+2][y-2] == EMPTY){                
	        *x_cap =x+2;
	        *y_cap =y-2;              
	        return TRUE;
	        } //if
			}
                
        if (x-2>-1 && y-2>-1) {
	        if ( (board[x-1][y-1] == BMAN ||
	        board[x-1][y-1] == BKING)&&
	        board[x-2][y-2] == EMPTY){	        
	        *x_cap =x-2;
	        *y_cap =y-2;	        
	        return TRUE;
	        }
			}
    }//WKING
        
    if (board[x][y] ==BKING)
    {
        if (x-2>-1 && y+2<BOARD_SIZE){
        if ((board[x-1][y+1] == WMAN ||
        board[x-1][y+1] == WKING) &&
        (board[x-2][y+2] == EMPTY)){        
        *x_cap =x-2;
        *y_cap =y+2;        
        return TRUE;
        }
		}
        
        if (x+2<BOARD_SIZE && y+2<BOARD_SIZE){
	        if ( (board[x+1][y+1] == WMAN ||
	        board[x+1][y+1] == WKING)&&
	        board[x+2][y+2] == EMPTY){	        
	        *x_cap =x+2;
	        *y_cap =y+2;	        
	        return TRUE;
	        }
			}
        
        if (x+2<BOARD_SIZE && y-2>-1){
	        if ((board[x+1][y-1] == WMAN ||
	        board[x+1][y-1] == WKING) &&
	        (board[x+2][y-2] == EMPTY)){               
	        *x_cap =x+2;
	        *y_cap =y-2;               
	        return TRUE;
	        }
			}
        
        if (x-2>-1 && y-2>-1){
	        if ((board[x-1][y-1] == WMAN ||
	        board[x-1][y-1] == WKING)&&
	        board[x-2][y-2] == EMPTY){                
	        *x_cap =x-2;
	        *y_cap =y-2;                
	        return TRUE;
	        }
			}
    }  //BKING
      
    return FALSE;	
		
}
//=====================================================================

// create a copy of the board
void copy_board(int src[BOARD_SIZE][BOARD_SIZE], int dest[BOARD_SIZE][BOARD_SIZE]) 
{
	
	for (int x = 0; x < BOARD_SIZE; ++x) {
	for (int y = 0; y < BOARD_SIZE; ++y) {
	dest[x][y] = src[x][y];
	}
	}
}
//=========================================================================

bool is_coord_valid(int x, int y)
{
	bool valid =FALSE;
	
	if((y==0 || y==2 || y==4 || y==6) && (x==0 || x==2 || x==4 || x==6)) valid =TRUE;
	
	if((y==1 || y==3 || y==5 || y==7) && (x==1 || x==3 || x==5 || x==7)) valid =TRUE;
	
	return valid;
}

//======================================================================
void get_possible_moves(int board[BOARD_SIZE][BOARD_SIZE], int turn, int possibleMoves[100][4], int* numMoves)
{
	
	if(turn ==1 || turn ==3) //white
	{		
	int whitemoves[100][4];
	int numWhiteMoves = 0;	
	get_possible_white_moves(board, whitemoves, &numWhiteMoves);
	
	for (int i = 0; i < numWhiteMoves; ++i) {
	int currentx1 = whitemoves[i][0];
	int currenty1 = whitemoves[i][1];
	int currentx2 = whitemoves[i][2];
	int currenty2 = whitemoves[i][3];
	
	possibleMoves[i][0]=currentx1;
	possibleMoves[i][1]=currenty1 ;
	possibleMoves[i][2]=currentx2;
	possibleMoves[i][3]=currenty2;	
	} //for	
	*numMoves = numWhiteMoves;
	//return;
	} //if turn 1 or 3
	
	if(turn ==2 || turn ==4) //black
	{		
	int blackmoves[100][4];
	int numBlackMoves = 0;	
	get_possible_black_moves(board, blackmoves, &numBlackMoves);	
	
	for (int i = 0; i < numBlackMoves; ++i) {
	int currentx1 = blackmoves[i][0];
	int currenty1 = blackmoves[i][1];
	int currentx2 = blackmoves[i][2];
	int currenty2 = blackmoves[i][3];
		    
	possibleMoves[i][0]=currentx1;
	possibleMoves[i][1]=currenty1 ;
	possibleMoves[i][2]=currentx2;
	possibleMoves[i][3]=currenty2;	
	} //for	
	*numMoves = numBlackMoves;
	//return;
	} //if turn 1 or 3	
}

//======================================================================
// get the best move for AI 
void get_best_move_AI(int board[BOARD_SIZE][BOARD_SIZE], int turn, int maxDepth, int* x1, int* y1, int* x2, int* y2) 
{	
	int moves[100][4];
	int numMoves = 0;
	get_possible_moves(board, PLAYER2, moves, &numMoves);
	
	int bestScore = -9999;	
	int bestMoveIndex = 0;
	
	// call minimax and get the index of the best move
	
	for (int i = 0; i < numMoves; ++i) {
	int currentx1 = moves[i][0];
	int currenty1 = moves[i][1];
	int currentx2 = moves[i][2];
	int currenty2 = moves[i][3];	
	int boardCopy[BOARD_SIZE][BOARD_SIZE];
	copy_board(board, boardCopy);
	
	make_move(boardCopy, currentx1, currenty1, currentx2, currenty2);
	int score = minimax_jumps(boardCopy, maxDepth, 0, PLAYER1, -9999, 9999); //player2 AI
	
	if (score > bestScore) {
	bestScore = score;
	bestMoveIndex = i;
	}//if score	
	} //for numMoves
	
	// update coordinate variables
	*x1 = moves[bestMoveIndex][0];
	*y1 = moves[bestMoveIndex][1];
	*x2 = moves[bestMoveIndex][2];
	*y2 = moves[bestMoveIndex][3];
}
//======================================================================

int get_number_black_pieces(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_black_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++){
	for(int y=0; y<BOARD_SIZE;y++) {
	
	if (board[x][y]==BMAN || board[x][y]==BKING)
	{
	number_black_pieces=number_black_pieces+1;
	}	
	} //y
	} //x
	
	return number_black_pieces;	
}
//=====================================================================
int get_number_BMAN(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_BMAN_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++){
	for(int y=0; y<BOARD_SIZE;y++) {
	
	if (board[x][y]==BMAN)
	{
	number_BMAN_pieces=number_BMAN_pieces+1;
	}	
	} //y
	} //x
	
	return number_BMAN_pieces;	
}
//====================================================================
int get_number_BKING(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_BKING_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++){
	for(int y=0; y<BOARD_SIZE;y++) {
	
	if (board[x][y]==BKING)
	{
	number_BKING_pieces=number_BKING_pieces+1;
	}	
	} //y
	} //x
	
	return number_BKING_pieces;	
}
//=====================================================================
int get_number_WMAN(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_WMAN_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++){
	for(int y=0; y<BOARD_SIZE;y++) {
	
	if (board[x][y]==WMAN)
	{
	number_WMAN_pieces=number_WMAN_pieces+1;
	}	
	} //y
	} //x
	
	return number_WMAN_pieces;	
}

//=====================================================================
int get_number_WKING(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_WKING_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++){
	for(int y=0; y<BOARD_SIZE;y++) {
	
	if (board[x][y]==WKING)
	{
	number_WKING_pieces=number_WKING_pieces+1;
	}	
	} //y
	} //x
	
	return number_WKING_pieces;	
}

//=====================================================================

int get_number_white_pieces(int board[BOARD_SIZE][BOARD_SIZE])
{
	int number_white_pieces=0;
	
	for(int x=0; x<BOARD_SIZE; x++)	{
	for(int y=0; y<BOARD_SIZE;y++)	{
	if(board[x][y] == WMAN || board[x][y] == WKING)
	{
	number_white_pieces=number_white_pieces+1;
	}	
	} //y
	} //x	
	return number_white_pieces;	
}

//======================================================================
bool is_black_win(int board[BOARD_SIZE][BOARD_SIZE])
{
	int white_count =get_number_white_pieces(board);
    if(white_count==0) return TRUE;
    else return FALSE;
}

//======================================================================
bool is_white_win(int board[BOARD_SIZE][BOARD_SIZE])
{
	int black_count=get_number_black_pieces(board);
    if(black_count==0)  return TRUE;
    else return FALSE;
}

//====================================================================== 
void make_move(int board[BOARD_SIZE][BOARD_SIZE],int x1, int y1, int x2, int y2) 
{
	int player=board[x1][y1];	
		
	//Black at top. White at bottom
	int white_king_line =7;
	int black_king_line =0;
	
	if (abs(x2 - x1) == 1) //standard move
    {	
	board[x2][y2] = player;
	board[x1][y1] = EMPTY;	
	//Check for Kings with standard move	
	if (y2 == white_king_line && player == WMAN) 
	{			
		board[x2][y2] = WKING;
	} 
	
	else if (y2== black_king_line && player==BMAN) {	
		
		board[x2][y2] = BKING;           
	}
	
	} //if standard move
	
	else if(abs(x2 - x1) == 2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
					
        board[x_cap][y_cap] = EMPTY;
        board[x2][y2] = player;
        board[x1][y1] = EMPTY;
        //Check for Kings after capture
        if (y2 ==white_king_line && player == WMAN) {
        board[x2][y2] =WKING;	     
        }
        else if (y2== black_king_line && player==BMAN) {        
        board[x2][y2] =BKING;
        }       
                       	
	} //else capture move
	
	//return second_capture;	
}

//======================================================================
int evaluate_position(int board[BOARD_SIZE][BOARD_SIZE]) 
{
		
	int WMAN_num = 0, BMAN_num = 0;
	int WKING_num = 0, BKING_num = 0;

	int score = 0;
	int BBL1=0; //black back line
	int BBL2=0;
	int BBL3=0;
	int BBL4=0;
	int WBL1 =0; //white back line
	int WBL2 =0;
	int WBL3 =0;
	int WBL4 =0;
	
	int BFL1=0; //black front line
	int BFL2=0;
	int BFL3=0;
	int BFL4=0;
	int WFL1 =0;//white front line
	int WFL2 =0;
	int WFL3 =0;
	int WFL4 =0;
					
	// count pieces and calculate score	
		for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
			//score for standard pawn pieces
			if (board[x][y] == BMAN) {
				BMAN_num=BMAN_num+2;
				
			} else if (board[x][y] == WMAN) {
				WMAN_num=WMAN_num+2;
				
				//score for king pieces
			} else if (board[x][y] == BKING) {
				BKING_num=BKING_num+4;
			
			} else if (board[x][y] == WKING) {
				WKING_num=WKING_num+4;
				
			}			
		}//y
	} //x
	
	//reward for back and front lines			
	if (board[1][7] == BMAN) BBL1=1;
	if (board[3][7] == BMAN) BBL2=1;
	if (board[5][7] == BMAN) BBL3=1;
	if (board[7][7] == BMAN) BBL4=1;
	
	if (board[1][7] == WMAN) WFL1=1;
	if (board[3][7] == WMAN) WFL2=1;
	if (board[5][7] == WMAN) WFL3=1;
	if (board[7][7] == WMAN) WFL4=1;
	
	if (board[0][0] == WMAN) WBL1=1;
	if (board[2][0] == WMAN) WBL2=1;
	if (board[4][0] == WMAN) WBL3=1;
	if (board[6][0] == WMAN) WBL4=1;
	
	if (board[0][0] == BMAN) BFL1=1;
	if (board[2][0] == BMAN) BFL2=1;
	if (board[4][0] == BMAN) BFL3=1;
	if (board[6][0] == BMAN) BFL4=1;
	
	score = (BMAN_num + BKING_num+BBL1+BBL2+BBL3+BBL4+BFL1+BFL2+BFL3+BFL4)
	         - (WMAN_num + WKING_num +WBL1+WBL2+WBL3+WBL4+WFL1+WFL2+WFL3+WFL4);
	
		
	return score;
}
//======================================================================
int evaluate_position2(int board[BOARD_SIZE][BOARD_SIZE]) 
{
		
	//As search depth increases piece count becomes the
	//dominant factor
	
	int WMAN_num = 0, BMAN_num = 0;
	int WKING_num = 0, BKING_num = 0;

	int score = 0;
					
	// count pieces and calculate score	
		for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
			//score for standard pawn pieces
			if (board[x][y] == BMAN) {
				BMAN_num=BMAN_num+1;
				
			} else if (board[x][y] == WMAN) {
				WMAN_num=WMAN_num+1;
				
				//score for king pieces
			} else if (board[x][y] == BKING) {
				BKING_num=BKING_num+2;
			
			} else if (board[x][y] == WKING) {
				WKING_num=WKING_num+2;
				
			}			
		}//y
	} //x
	
	score = (BMAN_num + BKING_num)- (WMAN_num + WKING_num);
	
	return score;
}

//======================================================================
// minimax jump algorithm with alpha-beta prunning
int minimax_jumps(int board[BOARD_SIZE][BOARD_SIZE], int maxDepth, int depth, int turn, int alpha, int beta) 
{
	
	if(depth_monitor)
	{
	mvwprintw(info_win, 8, 2, "Depth Monitor: %d                   ",depth);
	wrefresh(info_win);
     }
	// when max depth is reached, start evaluating the position
	if (depth == maxDepth) {
		if(maxDepth ==14){ 
			return evaluate_position2(board);
		}
		else {
		return evaluate_position(board);
		}	
	}

	// get the posible moves for this position
	int moves[100][4];
	int numMoves = 0;
	bool another_capture=FALSE;
	
	get_possible_moves(board, turn, moves, &numMoves);
	
	// AI turn,  max the score
	if (turn == PLAYER2) { 
		int maxScore = -9999;

		// for each of the possible moves, call minimax again
		for (int i = 0; i < numMoves; i++) {
			int x1 = moves[i][0]; 
			int y1 = moves[i][1];
			int x2 = moves[i][2]; 
			int y2 = moves[i][3];
			
			int boardCopy[BOARD_SIZE][BOARD_SIZE];
			copy_board(board, boardCopy);
		
			capture_max:
			make_move(boardCopy,x1, y1, x2, y2);
			int x3=0;
			int y3=0;
			another_capture =can_capture(boardCopy,x2,y2,&x3,&y3);
					
			if(another_capture)
			{								
				x1=x2;
				y1=y2;
				x2=x3;
				y2=y3;				
				goto capture_max;
			}
			
			int score = minimax_jumps(boardCopy, maxDepth, depth + 1, PLAYER1, alpha, beta);
					

			if (score > maxScore)
				maxScore = score;

			if (maxScore > alpha)
				alpha = maxScore; //alpha max

			// beta prunning
			if (beta <= alpha) break;
		}

		return maxScore;
		// my turn, min the score
	} else { 
		int minScore = 9999;

		// for each of the possible moves, call minimax again
		for (int i = 0; i < numMoves; i++) {
			int x1 = moves[i][0]; 
			int y1 = moves[i][1];
			int x2 = moves[i][2];
			int y2 = moves[i][3];
			
			int boardCopy[BOARD_SIZE][BOARD_SIZE];
			copy_board(board, boardCopy);

			
			capture_min:
			make_move(boardCopy, x1, y1, x2, y2);
			int x3=0;
			int y3=0;
			another_capture =can_capture(boardCopy,x2,y2,&x3,&y3);
						
			if(another_capture)
			{								
				x1=x2;
				y1=y2;
				x2=x3;
				y2=y3;
				goto capture_min;								
			}		
			
			int score = minimax_jumps(boardCopy, maxDepth, depth + 1, PLAYER2, alpha, beta);

			if (score < minScore)
				minScore = score;

			if (minScore < beta)
				beta = minScore; //beta min

			if (beta <= alpha) break;
		}

		return minScore;
	}
}
//======================================================================



