#ifndef TASK_H
#define TASK_H
#define TASK_STACK_PAGES 4

#define TASK_READY    0
#define TASK_RUNNING  1
#define TASK_BLOCKED  2
#define TASK_DEAD     3

#define TASK_KERNEL 0
#define TASK_USER   1

#define USER_STACK_TOP 0x80000000
#define USER_STACK_PAGES 4

typedef struct Task{
    unsigned int id;
    unsigned int esp;
    unsigned int kernel_stack_top;
    unsigned int user_stack_top;
    unsigned int state;
    unsigned int privilege;
    unsigned int page_directory_phys;
    unsigned int user_code_start;
    unsigned int user_code_pages;
    void (*entry)(void);
    

    struct Task *next;
} Task;

Task *CreateTask(void (*entry)(void));
void SwitchTask(unsigned int *old_esp, unsigned int new_esp);
void AddTask(Task *task);
void Yield(void);
void StartScheduler(void);
void RestoreTask(unsigned int task_esp);
unsigned int Schedule(unsigned int current_esp);
void TaskExit(void);
static void TaskTrampoline(void);
void DestroyTask(Task *task);
void CleanupDeadTasks(void);
void BlockCurrentTask(void);
void WakeTask(Task *task);
static unsigned int CreateUserStack(unsigned int page_directory_phys);
Task *CreateUserTask(void (*entry)(void));
unsigned int CreateKernelStackT(void);
static unsigned int CreateUserCode(unsigned int page_directory_phys);
void MarkCurrentTaskBlocked(void);

void MarkCurrentTaskDead(void);
Task *GetCurrentTask(void);


#endif 