#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <asm/page.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/iman.h>

// NOTE --> this is constant is defined in SystemXXXX.map in /boot dir :))
unsigned long *syscall_table = 0xc0585110; 

//###############################################

extern int init_flag;
extern struct my_proc dummy;
extern struct list_head* proc_list = &(dummy.list);

extern void add_process(pid_t id,int prio,long child_number);

extern void remove_process(pid_t proc_id);

extern long dfs(struct task_struct *current_task);

extern void clear_list();

extern asmlinkage long sys_init_data_list(pid_t p);


asmlinkage long new_sys_show_data_list(unsigned limit){
	printk("MODULE | changed_show() started \n");
	if(init_flag == FALSE  || list_empty(proc_list))
		sys_init_data_list(1);
	
	struct my_proc *p;
	list_for_each_entry(p,proc_list,list){
		if( (p->list).next == (proc_list))
			return 0;
		printk("pid:%d    |    child_no:%d\n",p->id,p->child_no);
		limit--;
		if(!limit)
		   return 0;
	}
	printk("MODULE | changed_show exited\n");
	return 1;
}
asmlinkage long new_sys_sort_data_list(void) {
	printk(KERN_ALERT "\nMODULE | changed_sort() started \n");
	if(init_flag == FALSE  || list_empty(proc_list))
		sys_init_data_list(1);
	struct list_head* head;
	struct list_head* max;
	struct list_head* iterator;
	struct my_proc* current_my_p_list;
	struct my_proc* current_max;
    	list_for_each(head, proc_list){
		max=head;
		current_max = list_entry(head, struct my_proc, list);
		for(iterator=(head->next); iterator != (proc_list); iterator= (iterator->next) ){
			current_my_p_list = list_entry(iterator, struct my_proc, list);
			if((current_my_p_list->child_no) > (current_max->child_no)){
				max = iterator;
				current_max = current_my_p_list;
			}
		}
		if(max != head){
        		  list_move_tail(max, head);
          		  head = head->prev;
       		}
	}
	printk("MODULE | done sorting!\n");
	return 0;
}

//#############################################
asmlinkage int (*original_sort)();
asmlinkage int (*original_show)();
asmlinkage int new_sort() {
 
    printk(KERN_ALERT "\nMODULE | changed_sort() is going to be called\n");
    return new_sys_sort_data_list();
}
asmlinkage int new_show(int limit){
    printk(KERN_ALERT "\nMODULE | changed_show() method\n");
    return new_sys_show_data_list(limit);
}


static int init(void) {
 
    printk(KERN_ALERT "\nMODULE | init() called!\n");
 
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    original_sort = (void *)syscall_table[__NR_sort_data_list];
    original_show = (void *)syscall_table[__NR_show_data_list];
    syscall_table[__NR_sort_data_list] = new_sort;  
    syscall_table[__NR_show_data_list] = new_show;
    write_cr0 (read_cr0 () | 0x10000);
    printk(KERN_ALERT "\nMODULE | Changed sys call table!\n");
    return 0;
}
 
static void exit(void) {
    printk(KERN_ALERT "\nMODULE | exit() called.\n");
    printk(KERN_ALERT "\nMODULE | Module is going to be removed.\n");
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    syscall_table[__NR_sort_data_list] = original_sort;
    syscall_table[__NR_show_data_list] = original_show;
    write_cr0 (read_cr0 () | 0x10000);
 
    printk(KERN_ALERT "\nMODULE | cleaned successfully!! \n");
 
    return;
}
 
module_init(init);
module_exit(exit);

