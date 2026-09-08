#include "task.h"
#include "heap.h"
#include "memory.h"
#include "paging.h"
#include "interrupts.h"
#include "kernel.h"
#include "gdt.h"
#include "tss.h"

#define USER_STACK_SLOT_SIZE 0x10000
#define USER_CODE_ADDR 0x40000000


#define USER_PROGRAM_SOURCE   0x30000
#define USER_PROGRAM_SIZE_PTR ((volatile unsigned int *)0x4FE0)
#define USER_MESSAGE_ADDR (USER_CODE_ADDR + 0x40)
static unsigned int next_task_id = 1;
static unsigned int next_stack_top = 0xC1000000;

static unsigned int next_user_stack_top = 0x80000000;

unsigned int page_directory_phys;

static Task *current_task = 0;
static Task *task_list = 0;

unsigned int user_code_start;
unsigned int user_code_pages;

Task *old_task;
Task *next_task;

static unsigned int scheduler_esp;

static Task *idle_task = 0;

Task *CreateUserTask(void (*entry)(void)){
    Task *task = (Task *)kmalloc(sizeof(Task));
    

    if(task == 0){
        return 0;
    }


    unsigned int kernel_stack_top = CreateKernelStackT();

    if (kernel_stack_top == 0) {
        kfree(task);
        return 0;
    }
    unsigned int user_pd =
    CreateUserPageDirectory();
    if(user_pd == 0){
        return 0;
    }

    unsigned int user_stack_top   = CreateUserStack(user_pd);

    if (user_stack_top == 0) {
        return 0;

    }

    unsigned int user_code = CreateUserCode(user_pd);

    if (user_code == 0) {
        return 0;
    }


    unsigned int user_size =
    *USER_PROGRAM_SIZE_PTR;

    unsigned int user_pages =
        (user_size + PAGE_SIZE - 1) / PAGE_SIZE;

    struct user_registers *regs =
    (struct user_registers *)
    (kernel_stack_top - sizeof(struct user_registers));

    regs->gs = USER_DATA_SELECTOR;
    regs->fs = USER_DATA_SELECTOR;
    regs->es = USER_DATA_SELECTOR;
    regs->ds = USER_DATA_SELECTOR;

    regs->edi = 0;
    regs->esi = 0;
    regs->ebp = 0;
    regs->esp = 0;

    regs->ebx = 0;
    regs->edx = 0;
    regs->ecx = 0;
    regs->eax = 0;

    regs->int_no = 32;
    regs->err_code = 0;

    regs->eip = user_code;
    regs->cs = USER_CODE_SELECTOR;
    regs->eflags = 0x202;

    regs->useresp = user_stack_top;
    regs->ss = USER_DATA_SELECTOR;

    task->esp = (unsigned int)regs;
    task->id = next_task_id++;
    task->state = TASK_READY;

    task->entry = entry;

    task->kernel_stack_top = kernel_stack_top;
    task->user_stack_top = user_stack_top;
    task->page_directory_phys =  user_pd;
    task->user_code_start = user_code;
    task->user_code_pages = user_pages;


    task->privilege = TASK_USER;
    task->next = 0;

    AddTask(task);

    return task;
}

static unsigned int CreateUserCode(unsigned int page_directory_phys)
{
    unsigned int size = *USER_PROGRAM_SIZE_PTR;
    unsigned int pages = (size+PAGE_SIZE-1)/PAGE_SIZE;
    

    for(int i = 0; i < pages;i++){

        unsigned int phys = AllocPage();

        if (phys == 0) {
            for(unsigned int j = 0; j < i; j++){
                unsigned int virtual = USER_CODE_ADDR + (j * PAGE_SIZE);
                unsigned int old_phys = GetPhysicalAddress(virtual);
                if(old_phys != 0){
                    UnmapPageInDirectory(page_directory_phys, virtual);
                    FreePage(old_phys & 0xFFFFF000);
                }

                
            }
            return 0;
        }
        unsigned int virt = USER_CODE_ADDR + (i * PAGE_SIZE);

        MapPageInDirectory(
            page_directory_phys,
            virt,
            phys,
            PAGE_PRESENT | PAGE_WRITE | PAGE_USER
        );
        unsigned int offset = i * PAGE_SIZE;
        unsigned int remaining = size - offset;
        unsigned int bytes_to_copy;

        if (remaining > PAGE_SIZE) {
            bytes_to_copy = PAGE_SIZE;
        } else {
            bytes_to_copy = remaining;
        }
        unsigned char *destination =   (unsigned char *)phys;

        unsigned char *source =
            (unsigned char *)(USER_PROGRAM_SOURCE + offset);

        for (unsigned int j = 0; j < bytes_to_copy; j++) {
            destination[j] = source[j];
        }
        

    }
    return USER_CODE_ADDR;
}

static unsigned int CreateUserStack(unsigned int page_directory_phys){
    unsigned int stack_top = next_user_stack_top;

    unsigned int stack_bottom =
        stack_top - (USER_STACK_PAGES * PAGE_SIZE);

    for(int i = 0; i < USER_STACK_PAGES; i++){
        unsigned int phys = AllocPage();
        if(phys == 0){
        for (unsigned int j = 0; j < i; j++) {

            unsigned int virt =
                stack_bottom + (j * PAGE_SIZE);

            unsigned int old_phys =
                GetPhysicalAddressInDirectory(
                    page_directory_phys,
                    virt
                );

            if (old_phys != 0) {

                UnmapPageInDirectory(
                    page_directory_phys,
                    virt
                );

                FreePage(old_phys & 0xFFFFF000);
            }
        }
            return 0;
        }
        unsigned int virt = stack_bottom + (i * PAGE_SIZE);
        MapPageInDirectory(page_directory_phys, virt, phys, PAGE_PRESENT|PAGE_WRITE|PAGE_USER);
    }
    next_user_stack_top -= USER_STACK_SLOT_SIZE;
    return stack_top;
}

void MarkCurrentTaskDead(void){
    if (current_task != 0) {
        current_task->state = TASK_DEAD;
    }
}

unsigned int CreateKernelStackT(void)
{
    unsigned int stack_top = next_stack_top;
    unsigned int stack_bottom =
        stack_top - (TASK_STACK_PAGES * PAGE_SIZE);

    for (unsigned int i = 0; i < TASK_STACK_PAGES; i++) {

        unsigned int phys = AllocPage();

        if (phys == 0) {
        for (unsigned int j = 0; j < i; j++) {

            unsigned int virt =
                stack_bottom + (j * PAGE_SIZE);

            unsigned int old_phys =
                GetPhysicalAddress(virt);

            if (old_phys != 0) {
                UnmapPage(virt);
                FreePage(old_phys & 0xFFFFF000);
            }
        }
        }

        unsigned int virt =
            stack_bottom + (i * PAGE_SIZE);

        MapPage(virt, phys, PAGE_WRITE);
    }

    next_stack_top += 0x10000;

    return stack_top;
}


Task *CreateTask(void (*entry)(void)){

    Task *task = (Task *)kmalloc(sizeof(Task));



    if (task == 0) {
        return 0;
    }

    unsigned int stack_top = CreateKernelStackT();

    if (stack_top == 0) {
        kfree(task);
        return 0;
    }

    struct registers *regs =
    (struct registers *)(stack_top - sizeof(struct registers));
    regs->gs = 0x10;
    regs->fs = 0x10;
    regs->es = 0x10;
    regs->ds = 0x10;

    regs->edi = 0;
    regs->esi = 0;
    regs->ebp = 0;
    regs->esp = 0;

    regs->ebx = 0;
    regs->edx = 0;
    regs->ecx = 0;
    regs->eax = 0;

    regs->int_no = 32;
    regs->err_code = 0;

    regs->eip = (unsigned int)TaskTrampoline;
    regs->cs = 0x08;
    regs->eflags = 0x202;

    task->esp = (unsigned int)regs;
    task->id = next_task_id++;
    task->state = TASK_READY;
    task->entry = entry;
    task->kernel_stack_top = stack_top;
    task->user_stack_top = 0;
    task->next = 0;
    task->privilege = TASK_KERNEL;
    task->page_directory_phys = GetKernelPageDirectory();
    task->user_code_start = 0;
    task->user_code_pages = 0;

    AddTask(task);



    return task;
  
}

void AddTask(Task *task){
    if(task_list == 0){
       task_list = task;
       task->next = task; 
    } else{
        Task *current = task_list;
        while(current->next != task_list){
            current = current->next;
        }
        current->next = task;
        task->next = task_list;
    }
}

void Yield(void){
    if (current_task == 0){
        return;
    }

    old_task = current_task;
    next_task = current_task->next;

    if (next_task == old_task){
        return;
    }
  
    current_task = next_task;

    SwitchTask(&old_task->esp, next_task->esp);
}
void StartScheduler(void){
    if(task_list == 0){
        return;
    }
    idle_task = CreateTask(IdleTask);

    current_task = task_list;
    current_task->state = TASK_RUNNING;

    if (current_task->privilege == TASK_USER) {
        tss_set_kernel_stack(current_task->kernel_stack_top);
    }
    SwitchPageDirectory(
        current_task->page_directory_phys
    );
    RestoreTask(current_task->esp);
}
unsigned int Schedule(unsigned int current_esp){

    
    if(current_task == 0){
        return current_esp;
    }

    current_task->esp = current_esp;

    if(current_task->state == TASK_RUNNING){
        current_task->state = TASK_READY;
    }

    
    CleanupDeadTasks();

    Task *current = current_task->next;
    Task *start = current;


    do{
        if(current->state == TASK_READY  && current != idle_task){
            current_task = current;
            current->state = TASK_RUNNING;

            if (current_task->privilege == TASK_USER) {
                tss_set_kernel_stack(current_task->kernel_stack_top);
            }
            SwitchPageDirectory(
                current_task->page_directory_phys
            );
            return current->esp;
        }
        current = current->next;

    }while(current != start);

    current_task = idle_task;
    idle_task->state = TASK_RUNNING;

    SwitchPageDirectory(
        idle_task->page_directory_phys
    );
    return idle_task->esp;



}

void TaskExit(void)
{
    current_task->state = TASK_DEAD;
    asm volatile("int $0x30");

    while (1) {
        asm volatile("hlt");
    }
}

static void TaskTrampoline(void){
    current_task->entry();
    TaskExit();
}

void DestroyTask(Task *task){
    if(task ==0){
        return;
    }
    if(task ==current_task){
        return;
    }
    if (task->privilege == TASK_USER) {

        for (unsigned int i = 0;
            i < task->user_code_pages;
            i++) {

            unsigned int virt =
                task->user_code_start +
                (i * PAGE_SIZE);

            unsigned int phys =
                GetPhysicalAddressInDirectory(
                    task->page_directory_phys,
                    virt
                );

            if (phys != 0) {

                UnmapPageInDirectory(
                    task->page_directory_phys,
                    virt
                );

                FreePage(phys & 0xFFFFF000);
            }
        }
        unsigned int stack_bottom =
        task->user_stack_top -
        (USER_STACK_PAGES * PAGE_SIZE);

        for (unsigned int i = 0;
            i < USER_STACK_PAGES;
            i++) {

            unsigned int virt =
                stack_bottom + (i * PAGE_SIZE);

            unsigned int phys =
                GetPhysicalAddressInDirectory(
                    task->page_directory_phys,
                    virt
                );

            if (phys != 0) {

                UnmapPageInDirectory(
                    task->page_directory_phys,
                    virt
                );

                FreePage(phys & 0xFFFFF000);
            }
        }
        FreePageTableInDirectory(
            task->page_directory_phys,
            256
        );

        FreePageTableInDirectory(
            task->page_directory_phys,
            511
        );

        FreePage(
            task->page_directory_phys
        );
        
    }
    unsigned int stack_bottom = task->kernel_stack_top - (TASK_STACK_PAGES * PAGE_SIZE);

    for(int i =0; i < TASK_STACK_PAGES; i++){
        unsigned int virt = stack_bottom + i * PAGE_SIZE;
        unsigned int phys = GetPhysicalAddress(virt);

        if(phys != 0){
            UnmapPage(virt);
            FreePage(phys);

        }

    }
    kfree(task);

    
}
void RemoveTask(Task *task){
    if(task == 0 || task_list == 0){
        return;
    }

    if(task == task_list && task->next == task){
        task_list = 0;
        task->next = 0;
        
        return;
    }
    if(task == task_list){
        Task *last = task_list;
        while(last->next != task_list){
            last = last->next;
        }
        
        last->next = task->next;
        task_list = task->next;
        task->next = 0;
        return;
    }
    Task *current = task_list;
    while(current->next != task && current->next != task_list){
        current = current->next;
    }
    if (current->next != task) {
        return;
    }
    current->next = task->next;
    task->next = 0;
    return;
    
}

void CleanupDeadTasks(void)
{
    if (task_list == 0)
        return;

    int removed;

    do {
        removed = 0;

        Task *task = task_list;

        do {
            Task *next = task->next;

            if (task->state == TASK_DEAD &&
                task != current_task) {

                RemoveTask(task);
                DestroyTask(task);

                removed = 1;
                break;
            }

            task = next;

        } while (task != task_list);

    } while (removed && task_list != 0);
}

void BlockCurrentTask(void){
    if(current_task == 0){
        return;
    }
    if(current_task->state == TASK_RUNNING){
        current_task->state = TASK_BLOCKED;
        asm volatile("int $0x30");
    }

}
void WakeTask(Task *task){
    if(task == 0){
        return;
    }
    if(task->state == TASK_BLOCKED){
        task->state = TASK_READY;
    }
}
void MarkCurrentTaskBlocked(void){
    if(current_task != 0){
        current_task->state = TASK_BLOCKED;
    }

}
Task *GetCurrentTask(void){
    return current_task;
}