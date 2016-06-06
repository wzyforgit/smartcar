#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#ifdef BLACK
#undef BLACK
#endif
#define BLACK 0

#ifdef WHITE
#undef WHITE
#endif
#define WHITE 255

extern double least_square(const local_t start,const local_t end,const local_t map_start,const local_t map_end,local_t *mids);

#endif