/* 
 *  timing.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

void timing_init(void);
void timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[]);
double timing_once_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3);
