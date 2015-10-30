#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/iman.h>


int init_flag = FALSE;
EXPORT_SYMBOL(init_flag);
struct my_proc dummy;
EXPORT_SYMBOL(dummy);
struct list_head* proc_list = &(dummy.list);
EXPORT_SYMBOL(proc_list);

long add_process(pid_t id,int prio,long child_number){
	//printk("SYS | adding process with pid = %d to list\n",id);
	struct my_proc *p;
	p = kmalloc(sizeof(*p),GFP_KERNEL);
	p->id = id; 
	p->priority = prio;
	p->child_no = child_number;
	list_add(&p->list,proc_list);
	return 0;
}
EXPORT_SYMBOL(add_process);

long remove_process(pid_t proc_id){
	//printk("SYS | removing process with pid = %d to list\n",proc_id);
	struct my_proc *p;
	list_for_each_entry(p,proc_list,list){
		if(p->id = proc_id){
			list_del(&p->list);
			kfree(p);
			return 0;
		}
	}
	return 1;
}
EXPORT_SYMBOL(remove_process);

long dfs(struct task_struct *current_task){
	long child_no = 1;
	struct task_struct *child;
	list_for_each_entry(child,&(current_task->children),sibling){
		child_no += dfs(child);
	}
	add_process(current_task->pid,current_task->prio,child_no);
	return child_no;
}
EXPORT_SYMBOL(dfs);

long clear_list(){
	printk("SYS | start clear_list()\n");
	struct list_head* it;
	list_for_each(it,proc_list){
		struct my_proc* cur;
		cur = list_entry(it,struct my_proc, list);
		struct list_head* prev = it->prev;
		list_del(it);
		it = prev;
		kfree(cur);
	}
	printk("SYS | end  clear_list()\n");
	return 0;
}
EXPORT_SYMBOL(clear_list);

asmlinkage long sys_init_data_list(pid_t p){
	printk("SYS | initing list\n");
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
	printk("SYS | now looking for pid:%d\n",p);
	for_each_process(task){		
		if(task->pid == p){ 
			process_exist = 0;
			dfs(task);
			break;
		}	
		
	}
	return process_exist;
}
EXPORT_SYMBOL(sys_init_data_list);

asmlinkage long sys_show_data_list(unsigned limit){
	printk("SYS | showing list\n");
	if(init_flag == FALSE  || list_empty(proc_list))
		sys_init_data_list(1);
	
	struct my_proc *p;
	list_for_each_entry(p,proc_list,list){
		if( (p->list).next == (proc_list))
			return 0;
		printk("pid:%d    |    prio:%d\n",p->id,p->priority);
		limit--;
		if(!limit)
		   return 0;
	}
	return -1;
}
EXPORT_SYMBOL(sys_show_data_list);

asmlinkage long sys_sort_data_list(void) {
	printk("SYS | started sorting list\n");
	if(init_flag == FALSE  || list_empty(proc_list))
		sys_init_data_list(1);
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
			if((current_my_p_list->priority) < (current_min->priority)){
				min = iterator;
				current_min = current_my_p_list;
			}
		}
		if(min != head){
        		  list_move_tail(min, head);
          		  head = head->prev;
       		}
	}
	printk("SYS | done sorting !\n");
	return 0;
}
EXPORT_SYMBOL(sys_sort_data_list);
