#include "../header/Task.h"

extern Tasks TaskBlocks[MaxTasks];
extern unsigned int TaskNum;
extern unsigned int RunningNum;
//External Globals that are Needed.


unsigned int PendFlag(Flag * EventFlag,unsigned int Mask,unsigned short Consume)
{
	EnterCS();
	if((EventFlag->BitEnabled & Mask) != Mask)
	{
		ExitCS();
		return 404;//If Bits are not all enabled Error
	}
	//check to see if the flags are set for the Mask
	while((EventFlag->FlagValue & Mask) != Mask)		//Loop while Flag not set
	{
		TaskBlocks[RunningNum].Blocked |= 0x2;				//Set Flag Blocking Bit
		TaskBlocks[RunningNum].EventFlag = EventFlag;	//Set the address of the blocking flag
		ExitCS();																			//Don't needed to worry of shared data anymore
		ContextSwitch();															//Change Task
		EnterCS();																		//Start Critical Again when returned.
	}
	if(Consume == 1)
	{
		EventFlag->FlagValue &= ~Mask; //Clear the flag values
	}
	ExitCS();
	return 0;
}

unsigned int AcceptFlag(Flag * EventFlag,unsigned int Mask,unsigned short Consume)
{
	EnterCS();
	if((EventFlag->BitEnabled & Mask) != Mask)
	{
		ExitCS();
		return 404;//If Bits are not all enabled Error
	}
	if((EventFlag->FlagValue & Mask) != Mask)//If Flag is not Set
	{
		ExitCS();
		return 1;	//Flag not set error
	}
	if(Consume == 1)
	{
		EventFlag->FlagValue &= ~Mask; //Clear the flag values
	}
	ExitCS();
	return 0;	//No error
}
unsigned int PostFlag(Flag * EventFlag,unsigned int Mask)
{
	int i;
	EnterCS();
	if((EventFlag->BitEnabled & Mask) != Mask)
	{
		ExitCS();
		return 404;//If Bits are not all enabled Error
	}
	EventFlag->FlagValue |= Mask;							//Set Flags
	for(i=0;i<TaskNum;i++) 										//unblock functions that use this flag
	{
		if(TaskBlocks[i].EventFlag == EventFlag)//Check if blocking flag is the same as set flag
		{
			TaskBlocks[i].Blocked &= ~0x2;				//Unblock Tasks that are relying on this flag
		}
	}
	ExitCS();
	return 0;
}
void CreateFlag(Flag * EventFlag,unsigned int Mask)
{
	EventFlag->BitEnabled = Mask; 						//Enable the bits
	EventFlag->FlagValue = 0x0;								//Clear All Flags
}
