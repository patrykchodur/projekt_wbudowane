#ifndef COMMON_H
#define COMMON_H

// notes used by the piano
typedef enum{
	NO_NOTE = -1,
	C = 1,
	Cs = 2,
	D,
	Ds,
	E,
	F,
	Fs,
	G,
	Gs,
	A,
	As,
	B,
	C2
} Note;

typedef struct {
	int x;
	int y;
} Point;

static int inside_rect(Point pt, Point p1, Point p2) {
	if (p1.x < p2.x)
		if (pt.x < p1.x || pt.x > p2.x)
			return 0;
	else
		if (pt.x < p2.x || pt.x > p1.x)
			return 0;
	if (p1.y < p2.y)
		if (pt.y < p1.y || pt.y > p2.y)
			return 0;
	else
		if (pt.y < p2.y || pt.y > p1.y)
			return 0;
	return 1;
}


#endif // COMMON_H
