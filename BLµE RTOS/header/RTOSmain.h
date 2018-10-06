#ifndef RTOSmain_H
#define RTOSmain_H

extern Tasks TaskBlocks[MaxTasks];//Not static because needed in Semaphore, Mailbox and Flags
extern unsigned int TaskNum;	//Total number of tasks created
extern unsigned int RunningNum;
extern unsigned int OSTaskSwitches;

#define TaskNextRun TaskBlocks[RunningNum].Delay
#define TaskCSwitches TaskBlocks[RunningNum].TaskSwitches
#define TaskPriority TaskBlocks[RunningNum].Priority
#define TaskNumber RunningNum
#define OSTime ReturnTicker()
#define OSCSwitches OSTaskSwitches

#endif
