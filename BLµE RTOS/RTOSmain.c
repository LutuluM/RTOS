//Includes
#include <stdbool.h>
#include <stdint.h>

#include "header/genInit.h"	//Functions for Setting Up Buttons, Leds, UART, and Systick
#include "header/Task.h"	//Contains the Heads to Sema and Flags and Mailboxes
#include "header/RTOSmain.h"//Global and Task Debugging Variables

//Prototypes For tasks
static void UpdateBlinkRate(void);
static void Blinky1(void);
static void Blinky2(void);
static void HeartBeat(void);
static void SerialTerm(void);
static void Monitor(void);

//Globals For Sema, Flags, Mailbox, ect...
Sema MySema;
Flag MyFlag;
Mailbox MyBox[2];
unsigned int delay = 1;


int main(void)
{
		SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //Setups up system clock

		Gen_Init(); 							//Setups The Leds and Buttons
		InitConsole(921600);			//Setup USB UART, Pass in Baud Rate
		Systick_Init(100000);				/*Setup Systick, value passed in is the number of Ticks per second
																Number of Ticks Determines how long the delay is. For example with
																the default value 1000, a delay of 1 is 1ms. Value can be changed but 
																1000 is recommend.
																*/
		//Initalize Semaphores, Flags, and Mailboxes
		CreateSema(&MySema,1);
		CreateFlag(&MyFlag,0x1|0x2);//Create Flag for 1 and 2
		CreateMailbox(&MyBox[0]);
		CreateMailbox(&MyBox[1]);
	
		//Add Functions to the RTOS, inputs are Function Address and Priority with 0 being the Highest
		AddFunc(UpdateBlinkRate,1);
		AddFunc(Blinky1,2);
		AddFunc(Blinky2,105);
		AddFunc(HeartBeat,100);
		AddFunc(SerialTerm,50);
		///Start Running the Tasks
		StartRTOS();
}

static void UpdateBlinkRate(void)
{
	int32_t test;
    while (1) {
			test = GPIOPinRead(GPIO_PORTF_BASE,0x11);
			if((test&0x11) == 0x00) PostFlag(&MyFlag,0x3);
			else if((test&0x11) == 0x01) PostFlag(&MyFlag,0x2);
			else if((test&0x11) == 0x10) PostFlag(&MyFlag,0x1);
			TaskDelay(10000);
			}
}

static void Blinky1(void)
{
		int32_t Delay = 300;
		int32_t error;
		int32_t test;
	

    while (1) {
				error = AcceptFlag(&MyFlag,0x1,0x1);
				if(error == 0)
				{
					Delay = 15+(Delay*1337)%100000;
					PostMailbox(&MyBox[0],(void*)Delay);
				}
        test = GPIOPinRead(GPIO_PORTF_BASE,0xE);
				test ^= 0x2;
				GPIOPinWrite(GPIO_PORTF_BASE,0xE,test);
				TaskDelay(Delay);
			}
}

static void Blinky2(void)
{
		int32_t Delay = 300;
		int32_t error;
		int32_t test,i;

    while (1) {
				error = AcceptFlag(&MyFlag,0x2,0x1);
				if(error == 0)
				{
					Delay = 20+(Delay*1337)%100000;
					PostMailbox(&MyBox[1],(void*)Delay);
				}
        test = GPIOPinRead(GPIO_PORTF_BASE,0xE);
				test ^= 0x4;
				GPIOPinWrite(GPIO_PORTF_BASE,0xE,test);
				i=0;
				while(i<Delay)i++;
			}
}

static void HeartBeat(void){
	while (1) {
		PendSema(&MySema);
		UARTprintf("Pulse \n");
		UARTprintf("TaskCSwitches:%d\nTaskPriority:%d\nTaskNumber:%d\nOSTime:%d\nOSCSwitches:%d\n",TaskCSwitches,TaskPriority,TaskNumber,OSTime,OSCSwitches);
		PostSema(&MySema);
		TaskDelay(50000);
	}
}

static	void	SerialTerm(void){
	unsigned short error;
	int32_t Delay[2] = {0,0};
	

	while (1) {
		Delay[0] = (int32_t)AcceptMailbox(&MyBox[0],&error);
		Delay[1] = (int32_t)AcceptMailbox(&MyBox[1],&error);
		if(Delay[0] != 0)	{
			PendSema(&MySema);
			UARTprintf("Blinky1 %d\n",Delay[0]);
			PostSema(&MySema);
		}
		if(Delay[1] != 0)	{
			PendSema(&MySema);
			UARTprintf("Blinky2 %d\n",Delay[1]);
			PostSema(&MySema);
		}
		if(Delay[0]==0 && Delay[1]==0)
		{
			PendSema(&MySema);
			//Monitor();
			PostSema(&MySema);
			
		}
		TaskDelay(100000);
	}
}
void Monitor(void){
	int32_t i;
//	UARTprintf("\x1b[2J");
	UARTprintf("                            BLuE RTOS top\n");
	UARTprintf("System Time: %10u\n", OSTime);
	//Priority,Delay,Semaphore,EventFlag,MailBox
	UARTprintf("|------------------------------------------------------------------|\n");
	UARTprintf("|Task #| Priority |Task Delay|Switches|Semaphore|Event Flag|Mailbox|\n");
	UARTprintf("|------------------------------------------------------------------|\n");
	for(i=0; i < TaskNum; i++){
		UARTprintf("|Task %u|%10u|%10u|%8u| %8X|  %8X|", 
			i, TaskBlocks[i].Priority, TaskBlocks[i].Delay, TaskBlocks[i].TaskSwitches, 
			TaskBlocks[i].Semaphore, TaskBlocks[i].EventFlag);
		if(IsEmpty(TaskBlocks[i].MailBox) == 1){
			UARTprintf("  empty");
		}
		else if(IsFull(TaskBlocks[i].MailBox) == 1){
			UARTprintf("   full");
		}
		else{
			UARTprintf("       ");
		}
		UARTprintf("|\n");
	}
	for(i=TaskNum; i < MaxTasks; i++){
		UARTprintf("\n");
	}
}
