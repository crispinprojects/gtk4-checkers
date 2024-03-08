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

int m_board2d[8][8];
int m_board[64];
int m_xpos=0;
int m_ypos=0;
int m_idx=0;

// declarations
static void game_board_button_press(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
gboolean game_board_can_capture_copy_board(int x, int y);
// signals
enum
{
	CELL_SELECTED_SIGNAL,	
	LAST_SIGNAL
};

//GameBoard

static guint game_board_signals[LAST_SIGNAL] = {0};

typedef struct _GameBoardClass GameBoardClass;
typedef struct _GameBoardPrivate GameBoardPrivate;

struct _GameBoard
{	
	GtkWidget widget; //parent instance
	GtkWidget *grid;	
	GtkWidget *cell_number_labels[8][8];	
	GtkWidget *game_label;	
	int xpos;
	int ypos;
	int cell_grid[8][8];
	int cells[8][8];		
};

struct _GameBoardClass
{
	GtkWidgetClass parent_class;
	void (*cell_selected)(GtkWidget *board);
	
};

G_DEFINE_TYPE(GameBoard, game_board, GTK_TYPE_WIDGET)

static void game_board_dispose(GObject *object)
{
	GameBoard *board = GAME_BOARD(object);
	g_clear_pointer(&board->grid, gtk_widget_unparent);
	G_OBJECT_CLASS(game_board_parent_class)->dispose(object);
}

static void game_board_class_init(GameBoardClass *klass)
{
	 GObjectClass *object_class =G_OBJECT_CLASS(klass);
	
	GtkWidgetClass *widget_class;
	widget_class = (GtkWidgetClass *)klass;

	object_class->dispose = game_board_dispose;
		
	//Signals emitted on user actions	
	game_board_signals[CELL_SELECTED_SIGNAL] =
		g_signal_new("cell-selected",
					 G_OBJECT_CLASS_TYPE(object_class),
					 G_SIGNAL_RUN_FIRST,
					 G_STRUCT_OFFSET(GameBoardClass, cell_selected),
					 NULL, NULL,
					 NULL,
					 G_TYPE_NONE, 0);

	
	gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}


GtkWidget *game_board_new(void)
{
	return g_object_new(GAME_TYPE_BOARD, NULL);
}
//=====================================================================
// setup board
//=====================================================================

void game_board_reset()
{

	
	for (int x = 0; x < 8; x++){
		for (int y = 0; y < 8; y++){
				m_board2d[x][y] = EMPTY;
			}
	}

  m_board2d[1][0]=WMAN;
  m_board2d[3][0]=WMAN;
  m_board2d[5][0]=WMAN;
  m_board2d[7][0]=WMAN;
  
  m_board2d[0][1]=WMAN;
  m_board2d[2][1]=WMAN;
  m_board2d[4][1]=WMAN;
  m_board2d[6][1]=WMAN;
  
  m_board2d[1][2]=WMAN;
  m_board2d[3][2]=WMAN;
  m_board2d[5][2]=WMAN;
  m_board2d[7][2]=WMAN;
  
  m_board2d[0][3]=EMPTY;
  m_board2d[2][3]=EMPTY;
  m_board2d[4][3]=EMPTY;
  m_board2d[6][3]=EMPTY;
  
  m_board2d[1][4]=EMPTY;
  m_board2d[3][4]=EMPTY;
  m_board2d[5][4]=EMPTY;
  m_board2d[7][4]=EMPTY;
  
  m_board2d[0][5]=BMAN;
  m_board2d[2][5]=BMAN;
  m_board2d[4][5]=BMAN;
  m_board2d[6][5]=BMAN;
  
  m_board2d[1][6]=BMAN;
  m_board2d[3][6]=BMAN; 
  m_board2d[5][6]=BMAN; 
  m_board2d[7][6]=BMAN;
  
  m_board2d[0][7]=BMAN;
  m_board2d[2][7]=BMAN;
  m_board2d[4][7]=BMAN;
  m_board2d[6][7]=BMAN; 
	
}
//====================================================================

int game_board_number_white_pieces()
{
	int number_white_pieces=0;
	
	for(int y=0; y<8;y++)
	{
	for(int x=0; x<8; x++)
	{
	if(m_board2d[x][y] == WMAN || m_board2d[x][y] == WKING)
	{
		number_white_pieces=number_white_pieces+1;
	}	
	} //x
	} //y
	//g_print("number_white_pieces = %d\n",number_white_pieces);
	return number_white_pieces;	
}
//====================================================================

int game_board_number_black_pieces()
{
	int number_black_pieces=0;
	
	for(int y=0; y<8;y++)
	{
	for(int x=0; x<8; x++)
	{
	if (m_board2d[x][y]==BMAN || m_board2d[x][y]==BKING)
	{
		number_black_pieces=number_black_pieces+1;
	}	
	} //x
	} //y
	
	return number_black_pieces;	
}
//======================================================================

gboolean game_board_check_black_win()
{
	int white_count =game_board_number_white_pieces();
    if(white_count==0) return TRUE;
    else return FALSE;
}

//======================================================================
gboolean game_board_check_white_win()
{
	int black_count=game_board_number_black_pieces();
    if(black_count==0)  return TRUE;
    else return FALSE;
}

//====================================================================
gboolean game_board_move_player(int x1, int y1, int x2, int y2, int player)
{
	//m_board2d[x][y] = player;
	gboolean second_capture =FALSE;	
	//White on top
	int white_king_line =7;
	int black_king_line =0;
	
	if (abs(x2 - x1) == 1) //standard move
    {
	m_board2d[x1][y1]=EMPTY;
	m_board2d[x2][y2]=player;
	
	//Check for Kings with standard move	
	if (y2 == white_king_line && player == WMAN) 
	{	
		m_board2d[x2][y2] =WKING;	
	} 
	
	else if (y2== black_king_line && player==BMAN) {	
		m_board2d[x2][y2] =BKING;           
	}
	
	} //if standard move
	
	else if(abs(x2 - x1) == 2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
		
		m_board2d[x_cap][y_cap] = EMPTY;
        m_board2d[x2][y2] = player;
        m_board2d[x1][y1] = EMPTY;	
        
        //Check for Kings after capture
        if (y2 ==white_king_line && player == WMAN) {               
        m_board2d[x2][y2] =WKING;	        
        }
        else if (y2== black_king_line && player==BMAN) { 
        m_board2d[x2][y2] =BKING; 
        }
        //check for another capture
        second_capture =game_board_can_capture (x2,y2);
                	
	} //else capture move
	
	return second_capture;	
}
//====================================================================
Move game_board_get_AI_move()
{
	
  GArray *possible_black_moves_arry; //arraylist	
  possible_black_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
  game_board_get_possible_black_moves(possible_black_moves_arry);
  
  //g_print("possible_black_moves_arry: size =%d\n",possible_black_moves_arry->len);
     
  if(possible_black_moves_arry->len ==0)
  {
	Move m;
	m.x1 =0;
	m.y1=0;
	m.x2 =0;
	m.y2 =0;
	m.capture=0;	
	return m;
	  
  } 
  if(possible_black_moves_arry->len >0) 
  {
	  //select a move
	  Move m = g_array_index (possible_black_moves_arry, Move, 0); //first move 
	  return m;
  }
    
  g_array_free(possible_black_moves_arry, FALSE); //clear the array 
 	
}

int random_number(int min_num, int max_num)
    {
        int result = 0, low_num = 0, hi_num = 0;

        if (min_num < max_num)
        {
            low_num = min_num;
            hi_num = max_num + 1; // include max_num in output
        } else {
            low_num = max_num + 1; // include max_num in output
            hi_num = min_num;
        }

        srand(time(NULL));
        result = (rand() % (hi_num - low_num)) + low_num;
        return result;
 }

Move game_board_make_AI_move(){
	
  
  int m_board2d_tmp[8][8];
  
  for (int x = 0; x < 8; x++){
		for (int y = 0; y < 8; y++){
				m_board2d_tmp[x][y] =0;
			}
	}
   
  GArray *possible_black_moves_arry; //arraylist	
  possible_black_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
  game_board_get_possible_black_moves(possible_black_moves_arry);
  
  //g_print("AI move: possible_black_moves_arry: size =%d\n",possible_black_moves_arry->len);
    
  Move ai_move;
  
  int ai_move_x1=0;
  int ai_move_y1=0;
  int ai_move_x2=0;
  int ai_move_y2=0;
  int ai_move_capture=0;
  int player; //BMAN or BKING
  
  ai_move.x1=ai_move_x1;
  ai_move.y1=ai_move_y1;
  ai_move.x2=ai_move_x2;
  ai_move.y2=ai_move_y2;
  ai_move.capture=ai_move_capture;
  
  if(possible_black_moves_arry->len == 0) return ai_move;
  
  if(possible_black_moves_arry->len ==1) {
	  Move m = g_array_index (possible_black_moves_arry, Move, 0); //only one move
	  ai_move_x1=m.x1;
	  ai_move_y1=m.y1;
	  ai_move_x2=m.x2;
	  ai_move_y2=m.y2;
	  ai_move_capture =m.capture;
  }  
  else if(possible_black_moves_arry->len >1) 
  {
	  //select a random move
	  int ai_max_number =possible_black_moves_arry->len;  
	  int index =random_number(0, ai_max_number-1);
	  //g_print("AI move: random move index =%d\n",index);
	  Move m = g_array_index (possible_black_moves_arry, Move, index);
	  
	  ai_move_x1=m.x1;
	  ai_move_y1=m.y1;
	  ai_move_x2=m.x2;
	  ai_move_y2=m.y2;
	  ai_move_capture =m.capture;   
	    	 
  } //else if
   
  ai_move.x1=ai_move_x1;
  ai_move.y1=ai_move_y1;
  ai_move.x2=ai_move_x2;
  ai_move.y2=ai_move_y2;
  ai_move.capture=ai_move_capture;
    
  player =m_board2d[ai_move_x1][ai_move_y1];
  
  //make the AI move
  if (abs(ai_move_x2 - ai_move_x1) == 1) //standard move
  {			
  m_board2d[ai_move_x1][ai_move_y1]=EMPTY;
  m_board2d[ai_move_x2][ai_move_y2]=player;
  
  if (ai_move_y2== 0 && player==BMAN) {
  m_board2d[ai_move_x2][ai_move_y2] =BKING;  	        
  }
  }//standard move
  else if(abs(ai_move_x2 - ai_move_x1) == 2) //capture move	
  {		
  int x_cap =abs((ai_move_x1 + ai_move_x2)/2);
  int y_cap =abs((ai_move_y1 + ai_move_y2)/2);
  
  m_board2d[x_cap][y_cap] = EMPTY;
  m_board2d[ai_move_x2][ai_move_y2] = player;
  m_board2d[ai_move_x1][ai_move_y1] = EMPTY;	
  
  //Check for Kings after capture
  if (ai_move_y2== 0 && player==BMAN) {
  m_board2d[ai_move_x2][ai_move_y2] =BKING;           
  }        	
  } //else if capture 
 
  return ai_move;
}

//=====================================================================

int game_board_get_player(int x, int y)
{
	return m_board2d[x][y];
}
//======================================================================
void game_board_remove_player(int x, int y)
{
	//m_board2d[x][y] = EMPTY;	
	m_board2d[x][y]=EMPTY;
}
//======================================================================

void game_board_show_console_board()
{
	g_print("======================================================\n");
	g_print("console board\n");
	g_print("======================================================\n");
	 	//print board
		for (int y=0;y<8;y++)
		{
		for (int x=0;x<8;x++)
		{ 
			int piece =m_board2d[x][y];
			g_print("%d ",piece);
		
		}//
		g_print("\n");
		}//y
		
	   for (int y=0;y<8;y++)
		{
		for (int x=0;x<8;x++)
		{ 
			int piece =m_board2d[x][y];
			g_print("(%d,%d)= %d ",x,y, piece);
		
		}//
		g_print("\n");
		}//y
	g_print("======================================================\n");
	 
}
//=====================================================================
void game_board_get_possible_white_moves(GArray *possible_moves)
 {
	 //white moving down board
	 int x_mov=0;
	 int y_mov=0;
	 int x_jump=0;
     int y_jump=0;
     int x_mid=0;
     int y_mid=0;
     
     GArray *tmp_arry; //arraylist	
     tmp_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
	 
	 for (int y=0;y<8;y++)
		{
		for (int x=0;x<8;x++)
		{   
			int player =m_board2d[x][y];			
			//g_print("%d ",player);
			
			if (player ==WMAN)				
			{
			//test jump1
			x_jump =x+2;
			y_jump =y+2;
			if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump<8))
			{                
			x_mid=x+1;
			y_mid=y+1;                
			if ((m_board2d[x][y] == WMAN) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING) &&
			m_board2d[x_jump][y_jump] == EMPTY)
			{			
			Move m;
			m.x1=x;
			m.y1=y;
			m.x2=x_jump;
			m.y2=y_jump;
			m.capture=1;
			//g_print("WMAN capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
			g_array_append_val(tmp_arry, m);
			}
			}//jump1  
			//test jump2
			x_jump=x-2;
			y_jump=y+2;
			if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump<8) ) {
			x_mid=x-1;
			y_mid=y+1;			
			if ((m_board2d[x][y] == WMAN) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING) &&
			m_board2d[x_jump][y_jump] == EMPTY) {
			Move m;
			m.x1=x;
			m.y1=y;
			m.x2=x_jump;
			m.y2=y_jump;
			m.capture=1;
			//g_print("BMAN capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
			g_array_append_val(tmp_arry, m);
			}
			}//if jump2	
			
			//standard moves
			//standard move1
			x_mov=x+1;
			y_mov=y+1; 
			if (!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov<8)) {
			if ((m_board2d[x][y] == WMAN) && (m_board2d[x_mov][y_mov] == EMPTY))
			{		
			Move m;
			m.x1=x;
			m.y1=y;
			m.x2=x_mov;
			m.y2=y_mov;
			m.capture=0;
			//g_print("WMAN standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
			g_array_append_val(tmp_arry, m); 
			}
			}//if x+1>-1 && y+1>-1
			//standard move2
			x_mov =x-1;
			y_mov=y+1; 
			if (!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov<8)){
			if ((m_board2d[x][y] == WMAN) &&(m_board2d[x_mov][y_mov] == EMPTY))
			{
			Move m;
			m.x1=x;
			m.y1=y;
			m.x2=x_mov;
			m.y2=y_mov;
			m.capture=0;
			//g_print("WMAN standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
			g_array_append_val(tmp_arry, m); 
			}
			} //if xmov ymov  			   
		   }//player BMAN
		   
		   else if (player==WKING)
	       {			   
		   //Kings have four possible jump moves
	       //jump1 
	       x_jump=x-2;
	       y_jump=y-2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump>-1)) {
	       x_mid=x-1;
	       y_mid=y-1;
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("WKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	      				 
	       }
	       }//if x_jump y_jump
	       //jump2
	       x_jump=x+2;
	       y_jump =y-2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump>-1)) {
	       x_mid =x+1;
	       y_mid =y-1;              
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING) 
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	       
	       }                
	       }//if jump		
	       //jump3
	       x_jump=x+2;
	       y_jump=y+2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump<8)) {
	       x_mid=x+1;
	       y_mid=y+1;
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	      					 
	       }
	       }//if x_jump y_jump
	       //jump4
	       x_jump=x-2;
	       y_jump=y+2;	      
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump<8)) 
	       {
	       x_mid=x-1;
	       y_mid=y+1;
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mid][y_mid] == BMAN || m_board2d[x_mid][y_mid] == BKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	     				 
	       }
	       }//if x_jump y_jump
	       
	       //kings have 4 standard moves
	       //standard move1
	       x_mov=x-1;
	       y_mov=y-1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov>-1)) {			
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("BKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov
	       //standard move2		
	       x_mov=x+1;
	       y_mov=y-1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov>-1)) {			
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("BKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov	       
	       //standard move3		
	       x_mov=x+1;
	       y_mov=y+1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov<8)) {			
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("BKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov	       
	       //standard move4
	       x_mov=x-1;
	       y_mov=y+1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov<8)) {			
	       if ((m_board2d[x][y] == WKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("BKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov 			   
		   }//player BKING			
		}//x
	}//y
	
	//tmp_arry contains all moves
	// if jumps possible remove other moves
	
	GArray *capture_moves_arry; //arraylist	
    capture_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
    
    for (int i = 0; i < tmp_arry->len; i++)	{
	Move m = g_array_index (tmp_arry, Move, i);
	//g_print("possible move: (%d,%d) -> (%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	 if(m.capture==1)
	 {
		 g_array_append_val(capture_moves_arry, m);
	 }
    }//for tmp_arry
    
    //g_print("capture_moves_arry: size =%d\n",capture_moves_arry->len);
    
    if(capture_moves_arry->len >0)
    {
	for (int i = 0; i < capture_moves_arry->len; i++)	{
	Move m = g_array_index (capture_moves_arry, Move, i);
	//g_print("possible move: (%d,%d) -> (%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	g_array_append_val(possible_moves, m);
    }//for capture moves
	} //if capture moves
	else
	{
		//standard moves
		 for (int i = 0; i < tmp_arry->len; i++)	{
	     Move m = g_array_index (tmp_arry, Move, i);
	     g_array_append_val(possible_moves, m);   
	     } //for
	}
	
	g_array_free(tmp_arry, FALSE); //clear the array 
	g_array_free(capture_moves_arry, FALSE); //clear the array 
	
 }


//======================================================================

 void game_board_get_possible_black_moves(GArray *possible_moves)
 {
	 //g_print("possible white moves\n");
	 //black moving up board
	 int x_mov=0;
	 int y_mov=0;
	 int x_jump=0;
     int y_jump=0;
     int x_mid=0;
     int y_mid=0;
     
     GArray *tmp_arry; //arraylist	
     tmp_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
	 
	 for (int y=0;y<8;y++)
		{
		for (int x=0;x<8;x++)
		{   
			int player =m_board2d[x][y];			
			//g_print("%d ",player);
			if (player ==BMAN)	
	        {
		    //test for jump1
		    x_jump=x-2;
		    y_jump=y-2;
		    if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump>-1)) {
		    x_mid=x-1;
		    y_mid=y-1;
		    if ((m_board2d[x][y] == BMAN) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING)
		    && (m_board2d[x_jump][y_jump] == EMPTY))
		    {		    					 
		    Move m;
		    m.x1=x;
		    m.y1=y;
		    m.x2=x_jump;
		    m.y2=y_jump;
		    m.capture=1;
		    //g_print("WMAN capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
		    g_array_append_val(tmp_arry, m);		    					 
		    }
		    }//if jump1
		    //test for jump2
		    x_jump=x+2;
		    y_jump =y-2;
		    if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump>-1)) {
		    x_mid =x+1;
		    y_mid =y-1;  
		    if ((m_board2d[x][y] == BMAN) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING)
		    && (m_board2d[x_jump][y_jump] == EMPTY))
		    {		   					 
		    Move m;
		    m.x1=x;
		    m.y1=y;
		    m.x2=x_jump;
		    m.y2=y_jump;
		    m.capture=1;
		    //g_print("WMAN capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
		    g_array_append_val(tmp_arry, m);		    					 
		    }
		    }//if jump2
		    //standard move 1
		    x_mov=x-1;
		    y_mov=y-1;
		    if(!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov>-1)) {			
		    if ((m_board2d[x][y] == BMAN) && (m_board2d[x_mov][y_mov] == EMPTY))
		    {		   
		    Move m;
		    m.x1=x;
		    m.y1=y;
		    m.x2=x_mov;
		    m.y2=y_mov;
		    m.capture=0;
		    //g_print("WMAN standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
		    g_array_append_val(tmp_arry, m); 
		    }
			}
		    //standard move 2
			x_mov=x+1;
		    y_mov=y-1;  		   
		    if(!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov>-1)){ 		    
		    if ((m_board2d[x][y] == BMAN) && (m_board2d[x_mov][y_mov] == EMPTY))
		    {		
		    Move m;
		    m.x1=x;
		    m.y1=y;
		    m.x2=x_mov;
		    m.y2=y_mov;
		    m.capture=0;
		    //g_print("WMAN standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
		    g_array_append_val(tmp_arry, m); 
		    }//if board
		    } //if standard move2	
		    
		    }//player WMAN
		   
		   else if (player==BKING)
	       {
	       //Kings have four possible jump moves
	       //jump1 
	       x_jump=x-2;
	       y_jump=y-2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump>-1)) {
	       x_mid=x-1;
	       y_mid=y-1;
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	      				 
	       }
	       }//if x_jump y_jump
	       //jump2
	       x_jump=x+2;
	       y_jump =y-2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump>-1)) {
	       x_mid =x+1;
	       y_mid =y-1;              
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING) 
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	       
	       }                
	       }//if jump		
	       //jump3
	       x_jump=x+2;
	       y_jump=y+2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump<8 && y_jump<8))
	       {
	       x_mid=x+1;
	       y_mid=y+1;
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	      					 
	       }
	       }//if x_jump y_jump
	       //jump4
	       x_jump=x-2;
	       y_jump=y+2;
	       if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump<8)) 
	       {
	       x_mid=x-1;
	       y_mid=y+1;
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mid][y_mid] == WMAN || m_board2d[x_mid][y_mid] == WKING)
	       && (m_board2d[x_jump][y_jump] == EMPTY))
	       {	       					 
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_jump;
	       m.y2=y_jump;
	       m.capture=1;
	       //g_print("BKING capture move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m);	     				 
	       }
	       }//if x_jump y_jump
	       
	       //kings have 4 standard moves
	       //standard move1
	       x_mov=x-1;
	       y_mov=y-1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov>-1)) {			
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("WKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov
	       //standard move2		
	       x_mov=x+1;
	       y_mov=y-1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov>-1)) {			
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("WKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov
	       
	       //standard move3		
	       x_mov=x+1;
	       y_mov=y+1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov<8 && y_mov<8)) {			
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("WKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov
	       
	       //standard move4
	       x_mov=x-1;
	       y_mov=y+1;
	       if(!((x_mov+y_mov)%2 ==0) && (x_mov>-1 && y_mov<8)) {			
	       if ((m_board2d[x][y] == BKING) && (m_board2d[x_mov][y_mov] == EMPTY))
	       {	       
	       Move m;
	       m.x1=x;
	       m.y1=y;
	       m.x2=x_mov;
	       m.y2=y_mov;
	       m.capture=0;
	       //g_print("WKING standard move (%d,%d)->(%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	       g_array_append_val(tmp_arry, m); 
	       }
	       }//if xmov ymov
	       
		 }//player WKING
			
		}//x
	}//y
	
	//tmp_arry contains all moves
	// if jumps possible remove other moves
	
	GArray *capture_moves_arry; //arraylist	
    capture_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
    
    for (int i = 0; i < tmp_arry->len; i++)	{
	Move m = g_array_index (tmp_arry, Move, i);
	//g_print("possible move: (%d,%d) -> (%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	 if(m.capture==1)
	 {
		 g_array_append_val(capture_moves_arry, m);
	 }
    }//for tmp_arry
    
    //g_print("capture_moves_arry: size =%d\n",capture_moves_arry->len);
    
    if(capture_moves_arry->len >0)
    {
	for (int i = 0; i < capture_moves_arry->len; i++)	{
	Move m = g_array_index (capture_moves_arry, Move, i);
	//g_print("possible move: (%d,%d) -> (%d,%d)\n", m.x1,m.y1,m.x2,m.y2);
	g_array_append_val(possible_moves, m);
    }//for capture moves
	} //if capture moves
	else
	{
		//standard moves
		 for (int i = 0; i < tmp_arry->len; i++)	{
	     Move m = g_array_index (tmp_arry, Move, i);
	     g_array_append_val(possible_moves, m);   
	     } //for
	}
	
	g_array_free(tmp_arry, FALSE); //clear the array 
	g_array_free(capture_moves_arry, FALSE); //clear the array 
	 
 }

//====================================================================

gboolean game_board_is_white_move_valid(int x1, int y1, int x2, int y2)
{
	
	gboolean move_state= FALSE;
	
	GArray *possible_white_moves_arry; //arraylist	
    possible_white_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));     
	game_board_get_possible_white_moves(possible_white_moves_arry);
	
	for (int i = 0; i < possible_white_moves_arry->len; i++)	
	{
	
	Move m = g_array_index (possible_white_moves_arry, Move, i);
	
	if(x1==m.x1 && y1==m.y1 && x2==m.x2 && y2==m.y2)
	{
	move_state=TRUE;
	}	
	} //for
	
  return move_state;
}
//=====================================================================

gboolean game_board_is_black_move_valid(int x1, int y1, int x2, int y2)
{
	
	gboolean move_state= FALSE;
	
	GArray *possible_black_moves_arry; //arraylist	
    possible_black_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));     
	game_board_get_possible_black_moves(possible_black_moves_arry);
	
	for (int i = 0; i < possible_black_moves_arry->len; i++)	
	{
	
	Move m = g_array_index (possible_black_moves_arry, Move, i);
	
	if(x1==m.x1 && y1==m.y1 && x2==m.x2 && y2==m.y2)
	{
	move_state=TRUE;
	}	
	} //for
	
  return move_state;
}

//=====================================================================

static void game_board_init(GameBoard *board)
{

	GtkWidget *widget = GTK_WIDGET(board);		
	gtk_widget_set_focusable(widget, TRUE);		
	
	game_board_reset();	

	GtkWidget *button;
	GtkGesture *gesture;

	gesture = gtk_gesture_click_new();
	g_signal_connect(gesture, "pressed", G_CALLBACK(game_board_button_press), board);
	gtk_widget_add_controller(GTK_WIDGET(board), GTK_EVENT_CONTROLLER(gesture));
	
	GtkWidget *game_label = gtk_label_new("Game");
	gtk_widget_set_hexpand(game_label, TRUE);
	gtk_widget_set_vexpand(game_label, TRUE);
	board->game_label = game_label;

	board->grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(board->grid), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(board->grid), TRUE);

	// initialise
	gchar *acell_str = "";
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			GtkWidget *label = gtk_label_new("");
			
			gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
			gtk_widget_add_css_class (GTK_WIDGET (label), "frame");	
									
			GtkCssProvider *provider;
			provider = gtk_css_provider_new ();
			gtk_css_provider_load_from_data (provider, "label.background {background-image: none; background-color: lightblue;}",-1);
                                            
            gtk_style_context_add_provider_for_display (gdk_display_get_default (), GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			
			if(y==0 && (x==1 || x==3 || x==5 || x==7)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }
			
			if(y==1 && (x==0 || x==2 || x==4 || x==6)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }
			
			if(y==2 && (x==1 || x==3 || x==5 || x==7)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }
			
			if(y==3 && (x==0 || x==2 || x==4 || x==6)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }
		    
		    if(y==4 && (x==1 || x==3 || x==5 || x==7)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }
			
			if(y==5 && (x==0 || x==2 || x==4 || x==6)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }	
		    
		    if(y==6 && (x==1 || x==3 || x==5 || x==7)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }	
					
			if(y==7 && (x==0 || x==2 || x==4 || x==6)){			
			gtk_widget_add_css_class (GTK_WIDGET (label), "background");
		    }	
		    
		    int player =m_board2d[x][y];	
		    
		    if(player ==1)
			{			
			acell_str = g_strdup_printf("%s", "W"); //white piece
			gtk_label_set_text(GTK_LABEL(label), acell_str);				 
			}		
			
			if(player ==2)
			{			
			acell_str = g_strdup_printf("%s", "B"); //black piece
			gtk_label_set_text(GTK_LABEL(label), acell_str);				 
			}

			gtk_widget_set_hexpand(label, TRUE);
			gtk_widget_set_vexpand(label, TRUE);			
			
  
			gtk_grid_attach(GTK_GRID(board->grid), label, x, y + 2, 1, 1);
			board->cell_number_labels[y][x] = label; // initialise
		}
	}
	
	gtk_widget_set_hexpand(board->grid, TRUE);
	gtk_widget_set_vexpand(board->grid, TRUE);
	gtk_widget_set_parent(board->grid, GTK_WIDGET(board));
}

//=====================================================================
int game_board_get_xpos(GameBoard *board)
{
	//g_print("gameboard: get_xpos = %d\n",board->xpos);
	return board->xpos;
}
//=====================================================================
int game_board_get_ypos(GameBoard *board)
{
	//g_print("gameboard: get_ypos = %d\n",board->ypos);
	return board->ypos;
}

//======================================================================
void game_board_redraw(GameBoard *board)
{
	//g_print("game board redraw cells\n");
	
	gchar *acell_str = "";
	int acell = 0;	
	// update cell labels
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{			
			int player =m_board2d[x][y];
		    
		    if(player ==WMAN)
			{			
			acell_str = g_strdup_printf("%s", "W"); //white piece
			gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);				 
			}	
			else if(player ==BMAN)
			{			
			acell_str = g_strdup_printf("%s", "B"); //black piece
			gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);				 
			}
			else if(player ==WKING)
			{			
			acell_str = g_strdup_printf("%s", "W*"); //white king
			gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);				 
			}
			else if(player ==BKING)
			{			
			acell_str = g_strdup_printf("%s", "B*"); //black king
			gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);				 
			}
			else if(player ==EMPTY)
			{			
			acell_str = g_strdup_printf("%s", ""); //empty
			gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);				 
			}			
			else {
				acell_str = g_strdup_printf("%s", ""); //some is wrong if reaches this
			    gtk_label_set_text(GTK_LABEL(board->cell_number_labels[y][x]), acell_str);
			}
			   			
		}//for y
			acell = acell + 1;
		}//for x
	
}
//=====================================================================

static void game_board_button_press(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
	//g_print("game board button pressed\n");
	
	GameBoard *board = user_data;
	GtkWidget *widget = GTK_WIDGET(board);
	GtkWidget *label;
	int row = -1, col = -1;
	int ix, iy;	
	int acell;
		
	label = gtk_widget_pick(widget, x, y, GTK_PICK_DEFAULT); //create label as pick widget
	
	acell =0; 
	
	for (iy = 0; iy < 8; iy++) {
		for (ix = 0; ix < 8; ix++)
		{
			if (acell >=0 && acell <64)	{				
			if (label == board->cell_number_labels[iy][ix])
			{
				row = iy;
				col = ix;
			} //picker label			
		    } //if aday	
		    acell = acell + 1;
		}//ix		
	}//iy
		
	//g_print("game_board_button_press: row = %i col = %i\n",row,col);

	if (row == -1 || col == -1) return; //something is wrong return

	//int index = row*8+col;
	//g_print("game_board_button_press: index = %d\n",index);
	
	//acell = board->cells[row][col];	
	//board->cell = acell;	
	//board->index =index;
	board->xpos=col;
	board->ypos=row;
	//g_print("game_board_button_press: index = %d\n",index);
	//g_print("game_board_button_press: xpos = %d\n",col);
	//g_print("game_board_button_press: ypos = %d\n",row);

	if (!gtk_widget_has_focus(widget))
		gtk_widget_grab_focus(widget);
   	
	game_board_redraw(board);
	g_signal_emit(board, game_board_signals[CELL_SELECTED_SIGNAL], 0);
}

void game_board_update(GameBoard *board)
{
	g_return_if_fail(GAME_IS_BOARD(board));
	game_board_redraw(board);	
}

//======================================================================
gboolean game_board_can_capture (int x, int y)
{
    //white on top    
    
    gboolean canjump =FALSE;
    
    if (m_board2d[x][y] ==WMAN)
    {
		 if (x+2<8 && y+2<8)
            if ( (m_board2d[x+1][y+1] == BMAN ||
                  m_board2d[x+1][y+1] == BKING)&&
                 m_board2d[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (m_board2d[x-1][y+1] == BMAN ||
                  m_board2d[x-1][y+1] == BKING)&&
                 m_board2d[x-2][y+2] == EMPTY)
                canjump=TRUE;
        
    }
    
    if (m_board2d[x][y] ==BMAN)
    {
        
        if (x-2>-1 && y-2>-1)
            if ( (m_board2d[x-1][y-1] == WMAN ||
                  m_board2d[x-1][y-1] == WKING) &&
                 (m_board2d[x-2][y-2] == EMPTY))
                canjump=TRUE;
        if (x+2<8 && y-2>-1)
            if ( (m_board2d[x+1][y-1] == WMAN ||
                  m_board2d[x+1][y-1] == WKING)&&
                 m_board2d[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
       
    }
    
    if (m_board2d[x][y] ==WKING)
    {
        if (x-2>-1 && y-2>-1)
            if ( (m_board2d[x-1][y-1] == BMAN ||
                  m_board2d[x-1][y-1] == BKING) &&
                 (m_board2d[x-2][y-2] == EMPTY))
                canjump=TRUE;
        
        if (x+2<8 && y-2>-1)
            if ( (m_board2d[x+1][y-1] == BMAN ||
                  m_board2d[x+1][y-1] == BKING)&&
                 m_board2d[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
        if (x+2<8 && y+2<8)
            if ( (m_board2d[x+1][y+1] == BMAN ||
                  m_board2d[x+1][y+1] == BKING) &&
                 (m_board2d[x+2][y+2] == EMPTY))
                canjump=TRUE;
        
        if (x-2>-1 && y+2<8)
            if ( (m_board2d[x-1][y+1] == BMAN ||
                  m_board2d[x-1][y+1] == BKING)&&
                 m_board2d[x-2][y+2] == EMPTY)
                canjump=TRUE;
    }

    if (m_board2d[x][y] ==BKING)
    {
        if (x+2<8 && y+2<8)
            if ( (m_board2d[x+1][y+1] == WMAN ||
                  m_board2d[x+1][y+1] == WKING)&&
                 m_board2d[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (m_board2d[x-1][y+1] == WMAN ||
                  m_board2d[x-1][y+1] == WKING)&&
                 m_board2d[x-2][y+2] == EMPTY)
                canjump=TRUE;

        if (x-2>-1 && y-2>-1)
            if ( (m_board2d[x-1][y-1] == WMAN ||
                  m_board2d[x-1][y-1] == WKING)&&
                 m_board2d[x-2][y-2] == EMPTY)
                canjump=TRUE;
                
        if (x+2<8 && y-2>-1)
            if ( (m_board2d[x+1][y-1] == WMAN ||
                  m_board2d[x+1][y-1] == WKING)&&
                 m_board2d[x+2][y-2] == EMPTY)
                canjump=TRUE;
    }
    return canjump;

}
//=======================================================================
