#ifndef __IMAN_H
#define __IMAN_H

#define FALSE 0
#define TRUE 1


struct my_proc{
	pid_t id;
	long priority;
	long child_no;
	struct list_head list;
};


#endif
