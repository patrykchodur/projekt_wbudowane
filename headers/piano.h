/*
 * functions supporting onscreen piano
 * and its buttons (play, record/stop recording, erase recording)
 * 
 * it takes action if button was hit and returns 1
 * if action took place (otherwise 0)
 */

#ifndef PIANO_H
#define PIANO_H

// for Point struct and Note enum
#include "common.h"


// no dependency
void piano_init(void);
// no need to call it twice
void draw_piano(void);
// if touchscreen was pressed pass 1
// otherwise 0
void piano_action(Point pt, char pressed);




#endif // PIANO_H
