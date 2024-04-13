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


#define EMPTY 0
#define WMAN 1
#define BMAN 2
#define WKING 3
#define BKING 4


struct _Move {
	int x1;
	int y1;
	int x2;
	int y2;
	int capture;
	int rank;
};

typedef struct _Move Move; 

int m_board[8][8];
static int m_x1=0;
static int m_y1=0;
static int m_x2=0;
static int m_y2=0;

// declarations
void init_board(int board[8][8]);
void copy_board(int board1[8][8], int board2[8][8]);
void print_board(int board[8][8]);

static void game_board_button_press(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void game_board_button_released(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);

void game_board_get_possible_white_moves(GArray *possible_moves);
void game_board_get_possible_black_moves(GArray *possible_moves);
gboolean game_board_can_capture (int x, int y);

int game_board_number_black_pieces();
int game_board_number_white_pieces();

//AI
void get_legal_black_moves(int board[8][8], GArray *possible_moves);
void get_legal_white_moves(int board[8][8], GArray *possible_moves);
gboolean can_capture(int board[8][8], int x, int y);
void move_player(int board[8][8], int player, int x1, int y1, int x2, int y2);
Move find_best_move(int board[8][8], GArray *possible_moves);
Move find_best_move2(int board[8][8], GArray *possible_moves);
int evaluate_board(int board[8][8]);
gboolean check_for_bking(int board[8][8]);


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
	// GtkWidget parent_instance;
	GtkWidget widget; //gameboard is a widget
	GtkWidget *grid;	
	GtkWidget *cell_images[8][8];	
	char* game_str;	
	int index;
	int xpos;
	int ypos;
	int white_score;
	int black_score;
	int cell_grid[8][8];
	int cells[8][8];		
};

struct _GameBoardClass
{
	GtkWidgetClass parent_class;
	void (*cell_selected)(GtkWidget *board);
	
};

G_DEFINE_TYPE(GameBoard, game_board, GTK_TYPE_WIDGET)

//=====================================================================
static void game_board_dispose(GObject *object)
{
	GameBoard *board = GAME_BOARD(object);
	g_clear_pointer(&board->grid, gtk_widget_unparent);
	G_OBJECT_CLASS(game_board_parent_class)->dispose(object);
}
//======================================================================
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

//======================================================================
GtkWidget *game_board_new(void)
{
	return g_object_new(GAME_TYPE_BOARD, NULL);
}
//=====================================================================
// setup board
//=====================================================================

void init_board(int board[8][8])
{
	
	for (int x = 0; x < 8; x++){
		for (int y = 0; y < 8; y++){
				board[x][y] = EMPTY;
			}
	}

  board[1][0]=WMAN;
  board[3][0]=WMAN;
  board[5][0]=WMAN;
  board[7][0]=WMAN;
  
  board[0][1]=WMAN;
  board[2][1]=WMAN;
  board[4][1]=WMAN;
  board[6][1]=WMAN;
  
  board[1][2]=WMAN;
  board[3][2]=WMAN;
  board[5][2]=WMAN;
  board[7][2]=WMAN;
  
  board[0][3]=EMPTY;
  board[2][3]=EMPTY;
  board[4][3]=EMPTY;
  board[6][3]=EMPTY;
  
  board[1][4]=EMPTY;
  board[3][4]=EMPTY;
  board[5][4]=EMPTY;
  board[7][4]=EMPTY;
  
  board[0][5]=BMAN;
  board[2][5]=BMAN;
  board[4][5]=BMAN;
  board[6][5]=BMAN;
  
  board[1][6]=BMAN;
  board[3][6]=BMAN; 
  board[5][6]=BMAN; 
  board[7][6]=BMAN;
  
  board[0][7]=BMAN;
  board[2][7]=BMAN;
  board[4][7]=BMAN;
  board[6][7]=BMAN;   	
}

//=====================================================================
void print_board(int board[8][8])
{
	g_print("======================================================\n");
	g_print("board state\n");
	g_print("======================================================\n");
	 	//print board
		for (int y=0;y<8;y++)
		{
		for (int x=0;x<8;x++)
		{ 
			int piece =board[x][y];
			g_print("%d ",piece);
		
		}//x
		g_print("\n");
		}//y

	g_print("======================================================\n");
		 
}
//=====================================================================

void copy_board(int board1[8][8], int board2[8][8])
{
	//copy board1 into board 2
	for(int y=0; y<8;y++){
	for(int x=0; x<8; x++) {
	board2[x][y]=board1[x][y]; 
	}//x
	}//y
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
			int player =m_board[x][y];			
			//g_print("%d ",player);
			if (player ==BMAN)	
	        {
		    //test for jump1
		    x_jump=x-2;
		    y_jump=y-2;
		    if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump>-1)) {
		    x_mid=x-1;
		    y_mid=y-1;
		    if ((m_board[x][y] == BMAN) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING)
		    && (m_board[x_jump][y_jump] == EMPTY))
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
		    if ((m_board[x][y] == BMAN) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING)
		    && (m_board[x_jump][y_jump] == EMPTY))
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
		    if ((m_board[x][y] == BMAN) && (m_board[x_mov][y_mov] == EMPTY))
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
		    if ((m_board[x][y] == BMAN) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING) 
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mid][y_mid] == WMAN || m_board[x_mid][y_mid] == WKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == BKING) && (m_board[x_mov][y_mov] == EMPTY))
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

//=====================================================================
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
			int player =m_board[x][y];			
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
			if ((m_board[x][y] == WMAN) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING) &&
			m_board[x_jump][y_jump] == EMPTY)
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
			if ((m_board[x][y] == WMAN) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING) &&
			m_board[x_jump][y_jump] == EMPTY) {
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
			if ((m_board[x][y] == WMAN) && (m_board[x_mov][y_mov] == EMPTY))
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
			if ((m_board[x][y] == WMAN) &&(m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING) 
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mid][y_mid] == BMAN || m_board[x_mid][y_mid] == BKING)
	       && (m_board[x_jump][y_jump] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mov][y_mov] == EMPTY))
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
	       if ((m_board[x][y] == WKING) && (m_board[x_mov][y_mov] == EMPTY))
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
void game_board_reset()
{	
	init_board(m_board);
	//print_board(m_board);
	
}
//=====================================================================
void load_mboard(int board[8][8])
{
  for(int y=0; y<8;y++){
  for(int x=0; x<8; x++) {
  m_board[x][y] =board[x][y]; 
  }//x
  }//y
}
//=====================================================================
void game_board_redraw(GameBoard *gameboard)
{
	GtkWidget *image=NULL;
	GtkWidget *widget = GTK_WIDGET(gameboard);		
	gtk_widget_set_focusable(widget, TRUE);	
		   
    
   GtkCssProvider *empty_provider;
  const char css[] =
    ".empty {"
    " background: white;"
    "  border: 2px solid lightgray;"
    "}";

  empty_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (empty_provider, css, -1); //debain12 has gtk4.8
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (empty_provider),800);
  
     
  GtkCssProvider *wman_provider;
  const char css_wman[] =
    ".wman {"
    " background: lemonchiffon;"
    " border-radius: 50%;"    
    " border: 2px solid gray;"
    "}";                                          
  wman_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (wman_provider, css_wman, -1); //debian12 has gtk4.8
  //gtk_css_provider_load_from_string (wman_provider, css_wman);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (wman_provider), 800);
                                              
                                              
  GtkCssProvider *wking_provider;
  const char css_wking[] =
    ".wking {"
    " background: greenyellow;"
    " border-radius: 50%;"  
    " border: 2px solid gray;"
    "}";                                          
  wking_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (wking_provider, css_wking, -1); //debian12 has gtk4.8
  //gtk_css_provider_load_from_string (wking_provider, css_wking);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (wking_provider), 800);
   
  GtkCssProvider *bman_provider;
  const char css_bman[] =
    ".bman {"
    " background: darkgray;"
    " border-radius: 50%;"  
    " border: 2px solid gray;"
    "}";                                          
   bman_provider = gtk_css_provider_new ();
   gtk_css_provider_load_from_data (bman_provider, css_bman, -1); //debian12 has gtk4.8
  //gtk_css_provider_load_from_string (bman_provider css_bman);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (bman_provider), 800);
                                              
  GtkCssProvider *bking_provider;
  const char css_bking[] =
    ".bking {"
    " background: darkslategray;"
    " border-radius: 50%;" 
    " border: 2px solid gray;"
    "}"; 
                                                
   bking_provider = gtk_css_provider_new ();
   gtk_css_provider_load_from_data (bking_provider, css_bking, -1); //debain12 has gtk4.8
  //gtk_css_provider_load_from_string (bking_provider, css_bking);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (bking_provider), 800);     
	
	// initialise

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{						
			int player =m_board[x][y];
		    
		    if(player ==WMAN) 
		    {
		    image = gtk_image_new ();
		    gtk_widget_add_css_class (image, "wman");
		    //gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
		    gtk_image_set_pixel_size(GTK_IMAGE (image),50); 		    		    
			}
			else if(player ==WKING)
			{	
			image = gtk_image_new ();
			gtk_widget_add_css_class (image, "wking");
			//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE);
			 gtk_image_set_pixel_size(GTK_IMAGE (image),50);  			
			}
			
			else if(player ==BMAN)
			{			
			image = gtk_image_new ();
			gtk_widget_add_css_class (image, "bman");
			//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
			 gtk_image_set_pixel_size(GTK_IMAGE (image),50); 					
			}
			else if(player ==BKING)
			{			
			image = gtk_image_new ();
			gtk_widget_add_css_class (image, "bking");
			//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE);	
			 gtk_image_set_pixel_size(GTK_IMAGE (image),50); 					
			}
			else {
				image = gtk_image_new ();
				gtk_widget_add_css_class (image, "empty");
				//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
				 gtk_image_set_pixel_size(GTK_IMAGE (image),50); 
				 
			}
			
			//gtk_widget_set_hexpand(image, TRUE);
	       // gtk_widget_set_vexpand(image, TRUE);
			gtk_grid_attach(GTK_GRID(gameboard->grid), image, x, y , 1, 1);
			gameboard->cell_images[y][x] = image; // initialise	
		}//x
	}//y
}

//=====================================================================
gboolean game_board_move_player(int x1, int y1, int x2, int y2, int player)
{
	gboolean second_capture =FALSE;	
	//White on top
	int white_king_line =7;
	int black_king_line =0;
	
	if (abs(x2 - x1) == 1) //standard move
    {
	m_board[x1][y1]=EMPTY;
	m_board[x2][y2]=player;
	
	//Check for Kings with standard move	
	if (y2 == white_king_line && player == WMAN) 
	{	
		m_board[x2][y2] =WKING;	
	} 
	
	else if (y2== black_king_line && player==BMAN) {	
		m_board[x2][y2] =BKING;           
	}
	
	} //if standard move
	
	else if(abs(x2 - x1) == 2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
		
		m_board[x_cap][y_cap] = EMPTY;
        m_board[x2][y2] = player;
        m_board[x1][y1] = EMPTY;	
        
        //Check for Kings after capture
        if (y2 ==white_king_line && player == WMAN) {               
        m_board[x2][y2] =WKING;	        
        }
        else if (y2== black_king_line && player==BMAN) { 
        m_board[x2][y2] =BKING; 
        }
        //check for another capture
        second_capture =game_board_can_capture (x2,y2);
                	
	} //else capture move
	
	return second_capture;	
}

//======================================================================
gboolean game_board_can_capture (int x, int y)
{
    //white on top    
    
    gboolean canjump =FALSE;
    
    if (m_board[x][y] ==WMAN)
    {
		 if (x+2<8 && y+2<8)
            if ( (m_board[x+1][y+1] == BMAN ||
                  m_board[x+1][y+1] == BKING)&&
                 m_board[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (m_board[x-1][y+1] == BMAN ||
                  m_board[x-1][y+1] == BKING)&&
                 m_board[x-2][y+2] == EMPTY)
                canjump=TRUE;
        
    }
    
    if (m_board[x][y] ==BMAN)
    {
        
        if (x-2>-1 && y-2>-1)
            if ( (m_board[x-1][y-1] == WMAN ||
                  m_board[x-1][y-1] == WKING) &&
                 (m_board[x-2][y-2] == EMPTY))
                canjump=TRUE;
        if (x+2<8 && y-2>-1)
            if ( (m_board[x+1][y-1] == WMAN ||
                  m_board[x+1][y-1] == WKING)&&
                 m_board[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
       
    }
    
    if (m_board[x][y] ==WKING)
    {
        if (x-2>-1 && y-2>-1)
            if ( (m_board[x-1][y-1] == BMAN ||
                  m_board[x-1][y-1] == BKING) &&
                 (m_board[x-2][y-2] == EMPTY))
                canjump=TRUE;
        
        if (x+2<8 && y-2>-1)
            if ( (m_board[x+1][y-1] == BMAN ||
                  m_board[x+1][y-1] == BKING)&&
                 m_board[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
        if (x+2<8 && y+2<8)
            if ( (m_board[x+1][y+1] == BMAN ||
                  m_board[x+1][y+1] == BKING) &&
                 (m_board[x+2][y+2] == EMPTY))
                canjump=TRUE;
        
        if (x-2>-1 && y+2<8)
            if ( (m_board[x-1][y+1] == BMAN ||
                  m_board[x-1][y+1] == BKING)&&
                 m_board[x-2][y+2] == EMPTY)
                canjump=TRUE;
    }

    if (m_board[x][y] ==BKING)
    {
        if (x+2<8 && y+2<8)
            if ( (m_board[x+1][y+1] == WMAN ||
                  m_board[x+1][y+1] == WKING)&&
                 m_board[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (m_board[x-1][y+1] == WMAN ||
                  m_board[x-1][y+1] == WKING)&&
                 m_board[x-2][y+2] == EMPTY)
                canjump=TRUE;

        if (x-2>-1 && y-2>-1)
            if ( (m_board[x-1][y-1] == WMAN ||
                  m_board[x-1][y-1] == WKING)&&
                 m_board[x-2][y-2] == EMPTY)
                canjump=TRUE;
                
        if (x+2<8 && y-2>-1)
            if ( (m_board[x+1][y-1] == WMAN ||
                  m_board[x+1][y-1] == WKING)&&
                 m_board[x+2][y-2] == EMPTY)
                canjump=TRUE;
    }
    return canjump;
}

//=====================================================================
static void game_board_init(GameBoard *gameboard)
{

	GtkWidget *image=NULL;
	GtkWidget *widget = GTK_WIDGET(gameboard);		
	gtk_widget_set_focusable(widget, TRUE);		
		
	init_board(m_board);
	//print_board(m_board);	
	GtkGesture *gesture;
	gesture = gtk_gesture_click_new();
	g_signal_connect(gesture, "pressed", G_CALLBACK(game_board_button_press), gameboard);
	g_signal_connect(gesture, "released", G_CALLBACK(game_board_button_released), gameboard);
	gtk_widget_add_controller(GTK_WIDGET(gameboard), GTK_EVENT_CONTROLLER(gesture));
	

	gameboard->grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(gameboard->grid), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(gameboard->grid), TRUE);
	gtk_grid_set_row_spacing (GTK_GRID (gameboard->grid), 2);
    gtk_grid_set_column_spacing (GTK_GRID (gameboard->grid), 2);    
    
     GtkCssProvider *empty_provider;
  const char css[] =
    ".empty {"
    " background: white;"
    " border: 2px solid lightgray;"
    "}";

  empty_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (empty_provider, css, -1); //debian12 has gtk4.8
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (empty_provider),800);
    
    
    GtkCssProvider *wman_provider;
     const char css_wman[] =
    ".wman {"
    " background: lemonchiffon;"
    " border-radius: 50%;"   
    " border: 2px solid gray;"
    "}";  
                                      
  wman_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (wman_provider, css_wman, -1); //debian12 has gtk4.8
  //gtk_css_provider_load_from_string (wman_provider_green, css_wman);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (wman_provider), 800);
                                              
  
  GtkCssProvider *bman_provider;
  const char css_bman[] =
    ".bman {"
    " background: darkgray;"
    " border-radius: 50%;" 
    " border: 2px solid gray;"
    "}";                                          
   bman_provider = gtk_css_provider_new ();
   gtk_css_provider_load_from_data (bman_provider, css_bman, -1); //debian12 has gtk4.8
  //gtk_css_provider_load_from_string (bman_provider, css_bman);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (bman_provider), 800);
	// initialise
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{		
			int player =m_board[x][y];
		    
		    if(player ==WMAN) 
		    {
		    image = gtk_image_new ();
		    gtk_widget_add_css_class (image, "wman");
		    //gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
		     gtk_image_set_pixel_size(GTK_IMAGE (image),50); 		   	    
			}			
			else if(player ==BMAN)
			{			
			image = gtk_image_new ();
			gtk_widget_add_css_class (image, "bman");
			//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
			 gtk_image_set_pixel_size(GTK_IMAGE (image),50); 						
			}
			else {
				image = gtk_image_new ();
				gtk_widget_add_css_class (image, "empty");
				//gtk_image_set_icon_size (GTK_IMAGE (image), GTK_ICON_SIZE_LARGE); 
				 gtk_image_set_pixel_size(GTK_IMAGE (image),50); 				 
			}
			
			
			gtk_widget_set_hexpand(image, TRUE);
	        gtk_widget_set_vexpand(image, TRUE);
			gtk_grid_attach(GTK_GRID(gameboard->grid), image, x, y , 1, 1);
			gameboard->cell_images[y][x] = image; // initialise				
		}//x
	}//y

	//gtk_widget_set_hexpand(gameboard->grid, TRUE);
	//gtk_widget_set_vexpand(gameboard->grid, TRUE);
	
	gtk_widget_set_parent(gameboard->grid, GTK_WIDGET(gameboard));
}

//======================================================================

static void game_board_button_press(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
	//g_print("game board button pressed\n");	
	GameBoard *gameboard = user_data;
	GtkWidget *widget = GTK_WIDGET(gameboard);
	GtkWidget *image;
	int y_pos = -1, x_pos = -1;
	int ix, iy;	
	int acell;
	
	GtkCssProvider *provider_selected;
	const char css_selected[] =
    ".selected {"
    " background: blue;"
    " border-radius: 50%;"
    " border: 2px solid lightgray;"
    "}";                                          
	provider_selected = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider_selected, css_selected, -1); //debian12 has gtk4.8
	//gtk_css_provider_load_from_string (provider_selected, css_selected);
	gtk_style_context_add_provider_for_display (gdk_display_get_default (),
	GTK_STYLE_PROVIDER (provider_selected),800);
			
	image = gtk_widget_pick(widget, x, y, GTK_PICK_DEFAULT); //create image as pick widget	
	gtk_widget_add_css_class (image, "selected");
	
	acell =0; 
	
	for (iy = 0; iy < 8; iy++) {
		for (ix = 0; ix < 8; ix++)
		{
			if (acell >=0 && acell <64)	{				
			if (image == gameboard->cell_images[iy][ix])
			{
				y_pos = iy;
				x_pos = ix;
			} //picker label			
		    } //if aday	
		    acell = acell + 1;
		}//ix		
	}//iy
		
	//g_print("game_board_button_press: x = %i y = %i\n",x_pos,y_pos);
	if (y_pos == -1 || x_pos == -1) return; //something is wrong return
	//int index = y_pos*8+x_pos;
	//g_print("game_board_button_press: index = %d\n",index);
	
	gameboard->xpos=x_pos;
	gameboard->ypos=y_pos;
	
	if (!gtk_widget_has_focus(widget))
		gtk_widget_grab_focus(widget);
   	
	//game_board_redraw(gameboard);
	g_signal_emit(gameboard, game_board_signals[CELL_SELECTED_SIGNAL], 0);
		
}
//======================================================================
static void game_board_button_released(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
	//g_print("mouse button released (%f,%f)\n",x,y);
    GameBoard *gameboard = user_data;
	GtkWidget *widget = GTK_WIDGET(gameboard);
	GtkWidget *image;	
	
	GtkCssProvider *provider_selected;
	const char css_selected[] =
    ".selected {"
    " background: blue;"
    " border-radius: 50%;"
    " border: 2px solid lightgray;"
    "}";                                          
	provider_selected = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider_selected, css_selected, -1); //debian12 has gtk4.8
	//gtk_css_provider_load_from_string (provider_selected, css_selected);
	gtk_style_context_add_provider_for_display (gdk_display_get_default (),
	GTK_STYLE_PROVIDER (provider_selected),800);
			
	image = gtk_widget_pick(widget, x, y, GTK_PICK_DEFAULT); //create image as pick widget
	gtk_widget_remove_css_class (image, "selected");	
	
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
//=====================================================================
int game_board_white_score(GameBoard *board)
{
	//g_print("gameboard: get_xpos = %d\n",board->xpos);
	return board->white_score;
}
//=====================================================================
int game_board_black_score(GameBoard *board)
{
	//g_print("gameboard: get_xpos = %d\n",board->xpos);
	return board->black_score;
}

//======================================================================
int game_board_get_player(int x, int y)
{
	return m_board[x][y];
}

//====================================================================

int game_board_number_white_pieces()
{
	int number_white_pieces=0;
	
	for(int y=0; y<8;y++)
	{
	for(int x=0; x<8; x++)
	{
	if(m_board[x][y] == WMAN || m_board[x][y] == WKING)
	{
		number_white_pieces=number_white_pieces+1;
	}	
	} //x
	} //y
	//g_print("number_white_pieces = %d\n",number_white_pieces);
	return number_white_pieces;	
}
//======================================================================

int game_board_number_black_pieces()
{
	int number_black_pieces=0;
	
	for(int y=0; y<8;y++)
	{
	for(int x=0; x<8; x++)
	{
	if (m_board[x][y]==BMAN || m_board[x][y]==BKING)
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
//======================================================================
// AI
//======================================================================

void get_legal_black_moves(int board[8][8], GArray *possible_moves)
{
	 //g_print("possible black moves\n");
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
			int player =board[x][y];			
			//g_print("%d ",player);
			if (player ==BMAN)	
	        {
		    //test for jump1
		    x_jump=x-2;
		    y_jump=y-2;
		    if(!((x_jump+y_jump)%2 ==0) && (x_jump>-1 && y_jump>-1)) {
		    x_mid=x-1;
		    y_mid=y-1;
		    if ((board[x][y] == BMAN) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
		    && (board[x_jump][y_jump] == EMPTY))
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
		    if ((board[x][y] == BMAN) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
		    && (board[x_jump][y_jump] == EMPTY))
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
		    if ((board[x][y] == BMAN) && (board[x_mov][y_mov] == EMPTY))
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
		    if ((board[x][y] == BMAN) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING) 
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mid][y_mid] == WMAN || board[x_mid][y_mid] == WKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == BKING) && (board[x_mov][y_mov] == EMPTY))
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
void get_legal_white_moves(int board[8][8], GArray *possible_moves)
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
			int player =board[x][y];			
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
			if ((board[x][y] == WMAN) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING) &&
			board[x_jump][y_jump] == EMPTY)
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
			if ((board[x][y] == WMAN) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING) &&
			board[x_jump][y_jump] == EMPTY) {
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
			if ((board[x][y] == WMAN) && (board[x_mov][y_mov] == EMPTY))
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
			if ((board[x][y] == WMAN) &&(board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING) 
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mid][y_mid] == BMAN || board[x_mid][y_mid] == BKING)
	       && (board[x_jump][y_jump] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
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
	       if ((board[x][y] == WKING) && (board[x_mov][y_mov] == EMPTY))
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
 
//====================================================================
gboolean can_capture(int board[8][8], int x, int y)
{
    //white on top    
    
    gboolean canjump =FALSE;
    
    if (board[x][y] ==WMAN)
    {
		 if (x+2<8 && y+2<8)
            if ( (board[x+1][y+1] == BMAN ||
                  board[x+1][y+1] == BKING)&&
                 board[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (board[x-1][y+1] == BMAN ||
                  board[x-1][y+1] == BKING)&&
                 board[x-2][y+2] == EMPTY)
                canjump=TRUE;
        
    }
    
    if (board[x][y] ==BMAN)
    {
        
        if (x-2>-1 && y-2>-1)
            if ( (board[x-1][y-1] == WMAN ||
                  board[x-1][y-1] == WKING) &&
                 (board[x-2][y-2] == EMPTY))
                canjump=TRUE;
        if (x+2<8 && y-2>-1)
            if ( (board[x+1][y-1] == WMAN ||
                  board[x+1][y-1] == WKING)&&
                 board[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
       
    }
    
    if (board[x][y] ==WKING)
    {
        if (x-2>-1 && y-2>-1)
            if ( (board[x-1][y-1] == BMAN ||
                  board[x-1][y-1] == BKING) &&
                 (board[x-2][y-2] == EMPTY))
                canjump=TRUE;
        
        if (x+2<8 && y-2>-1)
            if ( (board[x+1][y-1] == BMAN ||
                  board[x+1][y-1] == BKING)&&
                 board[x+2][y-2] == EMPTY)
                canjump=TRUE;
        
        if (x+2<8 && y+2<8)
            if ( (board[x+1][y+1] == BMAN ||
                  board[x+1][y+1] == BKING) &&
                 (board[x+2][y+2] == EMPTY))
                canjump=TRUE;
        
        if (x-2>-1 && y+2<8)
            if ( (board[x-1][y+1] == BMAN ||
                  board[x-1][y+1] == BKING)&&
                 board[x-2][y+2] == EMPTY)
                canjump=TRUE;
    }

    if (board[x][y] ==BKING)
    {
        if (x+2<8 && y+2<8)
            if ( (board[x+1][y+1] == WMAN ||
                  board[x+1][y+1] == WKING)&&
                 board[x+2][y+2] == EMPTY)
                canjump=TRUE;
        if (x-2>-1 && y+2<8)
            if ( (board[x-1][y+1] == WMAN ||
                  board[x-1][y+1] == WKING)&&
                 board[x-2][y+2] == EMPTY)
                canjump=TRUE;

        if (x-2>-1 && y-2>-1)
            if ( (board[x-1][y-1] == WMAN ||
                  board[x-1][y-1] == WKING)&&
                 board[x-2][y-2] == EMPTY)
                canjump=TRUE;
                
        if (x+2<8 && y-2>-1)
            if ( (board[x+1][y-1] == WMAN ||
                  board[x+1][y-1] == WKING)&&
                 board[x+2][y-2] == EMPTY)
                canjump=TRUE;
    }
    return canjump;

}

//====================================================================
void move_player(int board[8][8], int player, int x1, int y1, int x2, int y2)
{
	gboolean second_capture =FALSE;	
	//White on top and no king check
		
	if (abs(x2 - x1) == 1) //standard move
    {
	board[x1][y1]=EMPTY;
	board[x2][y2]=player;
	} //if standard move
	
	else if(abs(x2 - x1) == 2) //capture move	
	{		
		int x_cap =abs((x1 + x2)/2);
		int y_cap =abs((y1 + y2)/2);
		
		board[x_cap][y_cap] = EMPTY;
        board[x2][y2] = player;
        board[x1][y1] = EMPTY;	
                	
	} //else capture move
	
}

//======================================================================

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
//=====================================================================

//=====================================================================

gboolean check_for_bking(int board[8][8])
{
	gboolean make_bking=FALSE;
	int y=0;
	for (int x = 0; x < 8; x++){
	if (board[x][y] ==BMAN) make_bking=TRUE;
	}
	return make_bking;
}


//=====================================================================
int evaluate_board(int board[8][8])
{
	int score =0;
	
	int bmen=0;
	int wmen=0;
	int wkings=0;
	int bkings=0;
		
	for(int y=0; y<8;y++)
	{
	for(int x=0; x<8; x++)
	{
	if (board[x][y]==BMAN) bmen=bmen+1;
	if (board[x][y]==BKING) bkings=bkings+1;
	if (board[x][y]==WMAN) wmen=wmen+1;
	if (board[x][y]==WKING) wkings=wkings+1;		
	} //x
	} //y	
	//black wants positive score while white wants negative score
	score =(bmen+2*bkings) -(wmen+2*wkings);
	//g_print("eval board: blackmen=%d blackkings=%d whitemen=%d whitekings=%d score =%d\n", bmen,bkings,wmen,wkings,score);
	
	return score;
}

//Move find_best_move(int board[8][8], GArray *possible_black_moves_arry)
//{
	 ////select a random move
	  //int max_number =possible_moves->len;  
	  //int index =random_number(0, max_number-1);
	  ////g_print("AI move: random move index =%d\n",index);
	  //Move m = g_array_index (possible_moves, Move, index);	  
	  //return m;

//}

Move find_best_move2(int board[8][8], GArray *possible_black_moves_arry)
{
	
	
	//now process non-capture moves
	 
	 int white_score=0;
	 int black_score=0;
	 int board_testing[8][8];
	 copy_board(board, board_testing); //copy board1 into board2
	
	 GArray *ranked_moves_arry; //arraylist
     ranked_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));
     GArray *possible_white_moves_arry; //arraylist
     possible_white_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));
	  
	  //make black move and see if white can capture
	  for (int i = 0; i < possible_black_moves_arry->len; i++)	
	  {	
	    Move move_black = g_array_index (possible_black_moves_arry, Move, i);
	    move_black.rank=0;
	
	    int player = board_testing[move_black.x1][move_black.y1];	
	   
	    //do black move
	    move_player(board_testing, player, move_black.x1,move_black.y1, move_black.x2,move_black.y2);
	    
	    if(check_for_bking(board_testing))
	    {
			//g_print("king move detected\n");
			return move_black;
		}
	       
	    get_legal_white_moves(board_testing, possible_white_moves_arry);
	    	    
	    //check if white can capture moved black piece	    
	   for (int i = 0; i < possible_white_moves_arry->len; i++)	
	   {	
	  
	   Move move_white = g_array_index (possible_white_moves_arry, Move, i);	
	   int player = board_testing[move_white.x1][move_white.y1];	
	   move_player(board_testing, player, move_white.x1,move_white.y1, move_white.x2,move_white.y2); //do white move	
	   white_score = evaluate_board(board_testing);	
	   //g_print("white score = %d\n",white_score);
	    if(white_score<0)
	    {
			//bad for black, good for white
			move_black.rank =-1; //low rank
			g_array_append_val(ranked_moves_arry, move_black);
		}
		else {
			move_black.rank =1;
			g_array_append_val(ranked_moves_arry, move_black);
		}  
	   }//possible white moves
	
	   }//for black moves
	  
	  Move best_move = g_array_index (ranked_moves_arry, Move, 0);	 
	  int best_rank=0;
	  //g_print("----------------------------------------------------\n");
	  for (int i = 0; i < ranked_moves_arry->len; i++)	
	  {
		   Move ranked_move = g_array_index (ranked_moves_arry, Move, i);
		   //g_print("ranked move: (%d,%d)->(%d,%d) rank = %d\n", 
		      //ranked_move.x1,ranked_move.y1,ranked_move.x2,ranked_move.y2,ranked_move.rank);		   
		   if(ranked_move.rank>best_rank)
		   {
			   best_move = g_array_index (ranked_moves_arry, Move, i);
		   }		   
	  }	  
	 g_array_free(ranked_moves_arry, FALSE); //clear the array
	 g_array_free(possible_white_moves_arry, FALSE); //clear the array
	 return best_move; 	
	
	
}




//=====================================================================
Move find_best_move(int board[8][8], GArray *possible_black_moves_arry)
{
	 //need to check if we have multiple black capture moves
	 // treat these differently -favour capture moves with second capture
	 
	 GArray *capture_moves_arry; //arraylist
     capture_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));
    
	 
	 for (int i = 0; i < possible_black_moves_arry->len; i++)	
	 {	
	    Move move_b = g_array_index (possible_black_moves_arry, Move, i);
	    move_b.rank=0;
	    if(move_b.capture ==1)
	    {
			//return move_black; //return first capture for now
			g_array_append_val(capture_moves_arry, move_b);
		}
	 }
	 
	 if(capture_moves_arry->len >0)
	 {
		 //g_print("number of capture moves = %d\n",capture_moves_arry->len);
		 //process capture moves and return
		 Move best_capture_move = g_array_index (capture_moves_arry, Move, 0);
		 
		 for (int i = 0; i < capture_moves_arry->len; i++)	
		 {	
		 Move move_capture = g_array_index (capture_moves_arry, Move, i);
		 move_capture.rank=0;
		 //make capture move and check for second jump
		  int board_capture[8][8];
	      copy_board(board, board_capture); //copy board1 into board
	      int player = board_capture[move_capture.x1][move_capture.y1];	
	      move_player(board_capture, player, move_capture.x1,move_capture.y1, move_capture.x2,move_capture.y2); 
	      //check for second jump
	      gboolean second_capture = can_capture(board_capture,move_capture.x2,move_capture.y2);
	      //g_print("capture: second capture =%d\n",second_capture);
  
			if(second_capture)
			{
				//move_capture.rank=2;
				//g_print("setting new best capture move\n");
				best_capture_move = move_capture;
			}	      
		 }//for capture_moves_arry
		 	   
	  g_array_free(capture_moves_arry, FALSE); //clear the array	
	  return best_capture_move; 
		 
	 } //if capture_moves_arry->len >0
	 
	 //now process non-capture moves
	 
	 int white_score=0;
	 int black_score=0;
	 int board_testing[8][8];
	 copy_board(board, board_testing); //copy board1 into board2
	
	 GArray *ranked_moves_arry; //arraylist
     ranked_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));
     GArray *possible_white_moves_arry; //arraylist
     possible_white_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move));
	  
	  //make black move and see if white can capture
	  for (int i = 0; i < possible_black_moves_arry->len; i++)	
	  {	
	    Move move_black = g_array_index (possible_black_moves_arry, Move, i);
	    move_black.rank=0;
	
	    int player = board_testing[move_black.x1][move_black.y1];	
	   
	    //do black move
	    move_player(board_testing, player, move_black.x1,move_black.y1, move_black.x2,move_black.y2);
	    
	    if(check_for_bking(board_testing))
	    {
			//g_print("king move detected\n");
			return move_black;
		}
	       
	    get_legal_white_moves(board_testing, possible_white_moves_arry);
	    	    
	    //check if white can capture moved black piece	    
	   for (int i = 0; i < possible_white_moves_arry->len; i++)	
	   {	
	  
	   Move move_white = g_array_index (possible_white_moves_arry, Move, i);	
	   int player = board_testing[move_white.x1][move_white.y1];	
	   move_player(board_testing, player, move_white.x1,move_white.y1, move_white.x2,move_white.y2); //do white move	
	   white_score = evaluate_board(board_testing);	
	   //g_print("white score = %d\n",white_score);
	    if(white_score<0)
	    {
			//bad for black, good for white
			move_black.rank =-1; //low rank
			g_array_append_val(ranked_moves_arry, move_black);
		}
		else {
			move_black.rank =1;
			g_array_append_val(ranked_moves_arry, move_black);
		}  
	   }//possible white moves
	
	   }//for black moves
	  
	  Move best_move = g_array_index (ranked_moves_arry, Move, 0);	 
	  int best_rank=0;
	  //g_print("----------------------------------------------------\n");
	  for (int i = 0; i < ranked_moves_arry->len; i++)	
	  {
		   Move ranked_move = g_array_index (ranked_moves_arry, Move, i);
		   //g_print("ranked move: (%d,%d)->(%d,%d) rank = %d\n", 
		      //ranked_move.x1,ranked_move.y1,ranked_move.x2,ranked_move.y2,ranked_move.rank);		   
		   if(ranked_move.rank>best_rank)
		   {
			   best_move = g_array_index (ranked_moves_arry, Move, i);
		   }		   
	  }	  
	 g_array_free(ranked_moves_arry, FALSE); //clear the array
	 g_array_free(possible_white_moves_arry, FALSE); //clear the array
	 return best_move; 	
}
 
//======================================================================
void game_board_make_AI_move()
{
  gboolean second_capture=FALSE;
  int board_tmp[8][8];
  copy_board(m_board,board_tmp);
  
  int ai_move_x1=0;
  int ai_move_y1=0;
  int ai_move_x2=0;
  int ai_move_y2=0;
  int ai_move_capture=0;
  int player; //BMAN or BKING
   
  Move ai_move; 
   
  GArray *possible_black_moves_arry; //arraylist	
  possible_black_moves_arry = g_array_new(FALSE, FALSE, sizeof(Move)); 
 
  //get legal black moves, if captures possible it only returns these
  get_legal_black_moves(board_tmp, possible_black_moves_arry);  
 // g_print("AI move: possible_black_moves_arry: size =%d\n",possible_black_moves_arry->len);
  
  //select a legal move
  
  if(possible_black_moves_arry->len ==0)
  {
	g_array_free(possible_black_moves_arry, FALSE); //clear the array 	
	return;	  
  } 
  else if(possible_black_moves_arry->len ==1) {
	  Move m = g_array_index (possible_black_moves_arry, Move, 0); //only one move
	  ai_move_x1=m.x1;
	  ai_move_y1=m.y1;
	  ai_move_x2=m.x2;
	  ai_move_y2=m.y2;
	  ai_move_capture =m.capture;
  }  
  else if(possible_black_moves_arry->len >1) 
  {
	   //select a move	 
	 //need to rank moves to select best (could be multiple capture moves) 
	 Move m =find_best_move(board_tmp,possible_black_moves_arry);
	 
	  ai_move_x1=m.x1;
	  ai_move_y1=m.y1;
	  ai_move_x2=m.x2;
	  ai_move_y2=m.y2;
	  ai_move_capture =m.capture;  	    	 
  } //else if
   
  //make move      
  player =board_tmp[ai_move_x1][ai_move_y1];  
  //make the AI move
  if (abs(ai_move_x2 - ai_move_x1) == 1) //standard move
  {			
  board_tmp[ai_move_x1][ai_move_y1]=EMPTY;
  board_tmp[ai_move_x2][ai_move_y2]=player;
  //Check for King after standard move
  if (ai_move_y2== 0 && player==BMAN) {
  board_tmp[ai_move_x2][ai_move_y2] =BKING;  	        
  }
  }//standard move
  
  else if(abs(ai_move_x2 - ai_move_x1) == 2) //capture move	
  {		
  int x_cap =abs((ai_move_x1 + ai_move_x2)/2);
  int y_cap =abs((ai_move_y1 + ai_move_y2)/2);
  
  board_tmp[x_cap][y_cap] = EMPTY;
  board_tmp[ai_move_x2][ai_move_y2] = player;
  board_tmp[ai_move_x1][ai_move_y1] = EMPTY;
  
  //Check for Kings after capture
  if (ai_move_y2== 0 && player==BMAN) {
  board_tmp[ai_move_x2][ai_move_y2] =BKING;           
  }//kings
  
  //check for second capture
  gboolean second_capture = can_capture(board_tmp, ai_move_x2, ai_move_y2);
  
  if(second_capture)
  {
	  //AI second capture (recurse)
	  //g_print("another AI capture possible\n");
	  copy_board(board_tmp,m_board);
	  g_array_free(possible_black_moves_arry, FALSE);
	  game_board_make_AI_move(); 
	  return; 
  }
             	
  } //else if capture 
  
  //tmp board contains AI move. Copy to m_board and redraw
  copy_board(board_tmp,m_board);
  g_array_free(possible_black_moves_arry, FALSE);
 
 //redraw
 
}
//======================================================================
