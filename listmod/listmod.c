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
#define FALSE 0
#define TRUE 1


// NOTE --> this is constant is defined in System.map in /boot dir :))
unsigned long *syscall_table = 0xc0585110; 

//###############################################

struct my_proc{
	pid_t id;
	long priority;
	long child_no;
	struct list_head list;
};

int init_flag = FALSE;
struct my_proc dummy;
struct list_head* proc_list = &(dummy.list);

void add_process(pid_t id,int prio,long child_number){
	struct my_proc *p;
	p = kmalloc(sizeof(*p),GFP_KERNEL);
	p->id = id; 
	p->priority = prio;
	p->child_no = child_number;
	list_add_tail(&p->list,proc_list);
}

void remove_process(pid_t proc_id){
	struct my_proc *p;
	list_for_each_entry(p,proc_list,list){
		if(p->id = proc_id){
			list_del(&p->list);
			kfree(p);
			return;
		}
	}
}

long dfs(struct task_struct *current_task){
	long child_no = 1;
	struct task_struct *child;
	list_for_each_entry(child,&(current_task->children),sibling){
		child_no += dfs(child);
	}
	add_process(current_task->pid,current_task->prio,child_no);
	return child_no;
}

void clear_list(){
	printk("start clear_list()\n");
	struct list_head* it;
	list_for_each(it,proc_list){
		struct my_proc* cur;
		cur = list_entry(it,struct my_proc, list);
		struct list_head* prev = it->prev;
		list_del(it);
		it = prev;
		kfree(cur);
	}
	printk("end  clear_list()\n");
}

asmlinkage long new_sys_init_data_list(pid_t p){
	printk("initing list, in MODULE\n");
	if(init_flag == FALSE ){
		INIT_LIST_HEAD(proc_list);
		init_flag = TRUE;
	}
	struct task_struct *task;
	int process_exist = 1;

	if(!list_empty(proc_list)){
		printk("List is not empty!\n");
		clear_list();
	}
	printk("now looking for pid:%d\n",p);
	for_each_process(task){		
		if(task->pid == p){ 
			process_exist = 0;
			dfs(task);
			break;
		}	
		
	}
	return process_exist;
} 

asmlinkage long new_sys_show_data_list(unsigned limit){
	printk("showed\n");
	if(init_flag == FALSE  || list_empty(proc_list))
		new_sys_init_data_list(1);
	
	struct my_proc *p;
	list_for_each_entry(p,proc_list,list){
		if( (p->list).next == (proc_list))
			return 0;
		printk("pid:%d    |    child_no:%d\n",p->id,p->child_no);
		limit--;
		if(!limit)
		   return 0;
	}
	return -1;
}
asmlinkage long new_sys_sort_data_list(void) {
	if(init_flag == FALSE  || list_empty(proc_list))
		new_sys_init_data_list(1);
	struct list_head* head;
	struct list_head* min;
	struct list_head* iterator;
	struct my_proc* current_my_p_list;
	struct my_proc* current_min;
    	list_for_each(head, proc_list){
		min=head;
		current_min = list_entry(head, struct my_proc, list);
		for(iterator=(head->next); iterator != (proc_list); iterator= (iterator->next) ){
			current_my_p_list = list_entry(iterator, struct my_proc, list);
			if((current_my_p_list->child_no) > (current_min->child_no)){
				min = iterator;
				current_min = current_my_p_list;
			}
		}
		if(min != head){
        		  list_move_tail(min, head);
          		  head = head->prev;
       		}
	}
	printk("MODULE | SORTED !\n");
	return 0;
}

//#############################################
asmlinkage int (*original_sort)();
asmlinkage int (*original_show)();
asmlinkage int new_sort() {
 
    printk(KERN_ALERT "SORT CHANGED IN MODULE");
    return new_sys_sort_data_list();
}
asmlinkage int new_show(int limit){
    printk(KERN_ALERT "SORT CHANGED IN MODULE");
    return new_sys_show_data_list(limit);
}


static int init(void) {
 
    printk(KERN_ALERT "\nMODULE INIT\n");
 
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    original_sort = (void *)syscall_table[__NR_sort_data_list];
    original_show = (void *)syscall_table[__NR_show_data_list];
    syscall_table[__NR_sort_data_list] = new_sort;  
    syscall_table[__NR_show_data_list] = new_show;
    write_cr0 (read_cr0 () | 0x10000);
 
    return 0;
}
 
static void exit(void) {
 
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    syscall_table[__NR_sort_data_list] = original_sort;
    syscall_table[__NR_show_data_list] = original_show;
    write_cr0 (read_cr0 () | 0x10000);
 
    printk(KERN_ALERT "MODULE EXIT\n");
 
    return;
}
 
module_init(init);
module_exit(exit);

