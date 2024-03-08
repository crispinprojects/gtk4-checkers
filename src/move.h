#ifndef MOVE_H
#define MOVE_H

//move structure for checker pieces

struct _Move {
	int x1;
	int y1;
	int x2;
	int y2;
	int capture;
};

typedef struct _Move Move; 

#endif
