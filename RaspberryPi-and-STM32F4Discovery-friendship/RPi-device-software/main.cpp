// THIS CODE-SAMPLE WORKS IN REAL Raspberry Pi based IoT AUTOMATION SYSTEMS
// It helps us to understand how IoT automation is developed and to understand its vulnerabilities and weaknesses
// Code-sample provided for educational needs specially for DMZCON-2023
// Attend DMZCON-2023, international cyber security conference workshops to get new skills and insights
// www.dmzcon.com
// live.dmzcon.com

#include  <stdio.h>
#include  <unistd.h>

#ifndef	IS_TEMP
#include  <wiringPi.h>
#else
#include "temp.h"
#endif

#include  <sys/time.h>
#include  <time.h>
#include  <termios.h>
#include  <errno.h>
#include  <fcntl.h>
#include <pthread.h>
#include  "main.h"
#include "consts.h"
#include  "netsupport.h"
#include  "mqttcore.h"

#define	IS_DAEMON		1

#define	SHOWDEBUG		1
#define SHOWINFO		1

#define MAXPINCOUNT  				64
#define	MAX_READ_TIMEOUT			100

// Follow RECOMMENDED PINS of the STM32DISCOVERY-side in comments
#define CLK_PIN         22 // C7
#define CLK_ACK_PIN     24 // A8
#define ENABLE_PIN      26 //
#define READY_PIN       15 //
#define SEND_DATA_PIN   28 // C9
#define RECV_DATA_PIN   29 // C8

/////////////////////////


#define	RPI_GPIO_OUTPUT_1			0
#define	RPI_GPIO_OUTPUT_2			2


/////////////////////////////////

char *name_array[MAXPINCOUNT];

void	InitNameArray() {
name_array[0] = "EMPTY-0";
name_array[1] = "I_INPUT_1";
name_array[2] = "I_INPUT_2";
name_array[3] = "I_INPUT_3";
name_array[4] = "I_INPUT_4";
name_array[5] = "I_INPUT_5";
name_array[6] = "I_INPUT_6";
// ..........
name_array[28] = "I_INPUT_28";

name_array[29] = "O_OUTPUT_1";
// ...........
name_array[53] = "O_OUTPUT_N";

}

static unsigned int pinarr[MAXPINCOUNT] = {0x00};
static unsigned int prevpinarr[MAXPINCOUNT] = {0x00};
static unsigned int clicked[MAXPINCOUNT] = {0x00};
static unsigned int toggled[MAXPINCOUNT] = {0x00};
static unsigned int modearr[MAXPINCOUNT] = {0x00};
static unsigned int savestatesarr[MAXPINCOUNT] = {0x00};


/////////////////////////



/////////////////////////

void    LogTheMessage(unsigned int d1,unsigned int d2,char *message) {
FILE *f;
time_t tp;
struct tm *curtime;
struct timeval  timeval;
struct timezone tzp;
int mday,mon,hour,min;
f = fopen("/home/pi/SomeLogFilename.log","a");
if(f) {
        gettimeofday(&timeval,&tzp);
        tp = timeval.tv_sec;
        curtime=localtime(&tp);
        hour = curtime->tm_hour;
        min = curtime->tm_min;
        mday = curtime->tm_mday;
        mon = curtime->tm_mon;
        mon++;
        fprintf(f,"%d-%d_%d:%d - [%d] -\t%s - %d\n",mon,mday,hour,min,d2,message,d1);
        fclose(f);
}
}

//////////// SCENES ///////////////

void	RememberPreviousState() {
	unsigned int	k = 0;
	for(k=OUTPUT_FIRST_INDEX;k<=OUTPUT_LAST_INDEX;k++) {
		savestatesarr[k] = pinarr[k];
	}
}


void	TurnOff_All_Pins() {
	unsigned int	k = 0;
	for(k=OUTPUT_FIRST_INDEX;k<=OUTPUT_LAST_INDEX;k++) {
		savestatesarr[k] = pinarr[k];
		pinarr[k] = 0;
	}
  digitalWrite (RPI_GPIO_OUTPUT_1, LOW);
  digitalWrite (RPI_GPIO_OUTPUT_2, LOW);
}

void	Toggle_One(unsigned int *t,unsigned int num) {
	savestatesarr[num] = pinarr[num];
	if(*t == 0) *t = 1;
	else	*t = 0;
}

int generalcommandstimer = 0;

void	RunTimerBasedCommands() {

//	LogTheMessage(1,0,"RunTimerBasedCommands() launched");

/////////////// TIMER BASED COMMANDS HERE ////////////////

// The code placed here will be executed every 10-15 seconds in avarage

	

/////////////// FINISH OF TIMER BASED SCENES ////////////////	
}


///////////////////////////////////
/////////////////////////

int     daemonize(void) {
  char *ptty0;
  char *ptty1;
  char *ptty2;
  int fd;
  LogTheMessage(1,0,"Trying to Daemonize...");

/*  
  if (((ptty0 = ttyname(0)) == NULL) || ((ptty1 = ttyname(1)) == NULL) ||
      ((ptty2 = ttyname(2)) == NULL)) {
        LogTheMessage(1,0,"Daemonize FAILTURE-0");
        return -1;
  }
*/
  
  if (fork() != 0) {
        LogTheMessage(1,0,"Daemonize: fork passed");
    return -1;
  }
  close(0);
  close(1);
  close(2);
  setsid();
if ((fd = open("/dev/null", O_RDONLY)) == -1)  {
        LogTheMessage(1,0,"Daemonize FAILTURE");
  return -1;
}
if (dup2(fd, 0) == -1)  {
        LogTheMessage(2,0,"Daemonize FAILTURE");
  return -1;
}
/*
if ((fd = open(ptty1, O_WRONLY)) == -1)  {
        LogTheMessage(3,0,"Daemonize FAILTURE");
  return -1;
}
if (dup2(fd, 1) == -1)  {
        LogTheMessage(4,0,"Daemonize FAILTURE");
  return -1;
}
if (close(fd) == -1)  {
        LogTheMessage(5,0,"Daemonize FAILTURE");
  return -1;
}
if ((fd = open(ptty2, O_WRONLY)) == -1)  {
        LogTheMessage(6,0,"Daemonize FAILTURE");
  return -1;
}
if (dup2(fd, 2) == -1)  {
        LogTheMessage(7,0,"Daemonize FAILTURE");
  return -1;
}
if (close(fd) == -1)  {
        LogTheMessage(8,0,"Daemonize FAILTURE");
  return -1;
}
*/
return 0;
}

/////////////////////////

int main (void) {
        unsigned int j = 0;
        unsigned int i = 0;
        unsigned int k = 0;
	unsigned int di = 0;
	unsigned int index = 0;
	unsigned int timeout_counter = 0;
	unsigned int stm_is_ready;
        unsigned int clk=1,clk_ack=0;

	pthread_t	nethandler_p;
	int	ret = 0;

#ifdef	HA_INTEGRATION

	ret = pthread_create(&nethandler_p,NULL,&NetHandler,NULL);
#endif

	
#ifdef	IS_DAEMON
	if(daemonize()!=0) return 1;
#endif
	
#ifdef	IS_TEMP
	printf("Temp runtime started, ret=%d\n",ret);
	while(1) {
		sleep(1);
	}
#endif

        if(wiringPiSetup () == -1) {
#ifdef	SHOWINFO
                printf("GPIO INIT FAILURE\n");
#endif
                return 1;
        }

        pinMode (CLK_PIN, OUTPUT);

        pinMode (RPI_GPIO_KITCHEN_LED, OUTPUT);
        pinMode (RPI_GPIO_HALLFLOOR_LED, OUTPUT);
        digitalWrite (RPI_GPIO_KITCHEN_LED, LOW);
        digitalWrite (RPI_GPIO_HALLFLOOR_LED, LOW);

	
        pinMode (CLK_ACK_PIN, INPUT);
        pullUpDnControl(CLK_ACK_PIN, PUD_DOWN);

        pinMode (ENABLE_PIN, OUTPUT);

        pinMode (SEND_DATA_PIN, OUTPUT);

        pinMode (RECV_DATA_PIN, INPUT);
        pullUpDnControl(RECV_DATA_PIN, PUD_DOWN);

        digitalWrite (ENABLE_PIN, LOW);
        digitalWrite (CLK_PIN, LOW);
	sleep(2);

	for(i = 0;i<MAXPINCOUNT;i++) {
		pinarr[i] = 0x00;
		prevpinarr[i] = 0x00;
	}
	InitNameArray();

INIT_NEW_EXCHANGE:

/*
  LogTheMessage(0,0,"Reconnection...");
  LogTheMessage(0,0,"Setting ENABLE and CLK to LOW");
*/
  digitalWrite (CLK_PIN, LOW);
  digitalWrite (ENABLE_PIN, LOW);
	usleep(250000);

	for(k=0;k<MAXPINCOUNT;k++)
		pinarr[k] = prevpinarr[k];

	//  printf("Setting ENABLE to HIGH\n");

        digitalWrite (ENABLE_PIN, HIGH);
        stm_is_ready =  digitalRead(RECV_DATA_PIN);
        if(stm_is_ready == 1) {
     //     printf("STM-controller was found. Continue. Try to connect...\n");

		timeout_counter = 0;
		while(timeout_counter < MAX_READ_TIMEOUT) {
	                stm_is_ready =  digitalRead(RECV_DATA_PIN);
			if(stm_is_ready == 0) goto INIT_CONTINUE;
			timeout_counter++;
      //  printf("RECV_PIN = %0x (Not connected) - not ready for exchange [timeout_counter=%d]\n",stm_is_ready,timeout_counter);
			usleep(10000);
		}
		if(timeout_counter >= MAX_READ_TIMEOUT) goto INIT_NEW_EXCHANGE;
INIT_CONTINUE:
       //         printf("RECV_PIN = %0x (Connected) [timeout_counter=%d], Start Exchange...\n",stm_is_ready,timeout_counter);
                clk_ack = 0;
                clk = 1;
		index = 0;
        } else {
//		printf("STM controller was not found. Hardware link is DOWN. Trying to find again...\n");
		goto INIT_NEW_EXCHANGE;
	}
        clk_ack =  digitalRead(CLK_ACK_PIN);
//        printf("Current CLK_ACK_PIN=%d\n",clk_ack);
        clk_ack = 0;

while(1) {

	if(clk_ack==0 && j<64) {
    stm_is_ready =  digitalRead(RECV_DATA_PIN);
    if(pinarr[index] == 0)
			digitalWrite (SEND_DATA_PIN, LOW);
		else
			digitalWrite (SEND_DATA_PIN, HIGH);
		index++;
		if(index == 64) {
			index = 0;
		}
	}

	if(clk == 0)
		digitalWrite (CLK_PIN, LOW);
	else
		digitalWrite (CLK_PIN, HIGH);
	stm_is_ready =  digitalRead(RECV_DATA_PIN);

	if(clk_ack == 1) {
		if(j>=64 && j<=127) {
			pinarr[index] = digitalRead(RECV_DATA_PIN);
			index++;
			if(index == 64) {
				index = 0;
			}
		}
		j++;	
		if(j == 128) {

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/*
                        printf("===========================================\n");
                        for(i = 0;i<MAXPINCOUNT;) {

                                for(k=0;k<8 && i<MAXPINCOUNT;k++,i++) {
                                        printf("%0x   ",pinarr[i]);
                                }
                                printf("\n");
                        }
                        printf("===========================================\n");
*/


			for(k=0;k<MAXPINCOUNT;k++) {
				if(pinarr[k] != prevpinarr[k] && k>=INPUT_FIRST_INDEX && k<=OUTPUT_LAST_INDEX) {
					toggled[k] = 1;
		//			printf("[%d] %s changed to %0x\n",k,name_array[k],pinarr[k]);
				}
				if(pinarr[k] != prevpinarr[k] && pinarr[k] != 0 && k>=INPUT_FIRST_INDEX && k<=INPUT_LAST_INDEX) {
					clicked[k]=1;
				}
			}
			
			
/////////////////////////////////////////////////////////////////////////////////////////
//  ************************************************************************************
/////////////////////////  COMMANDS START HERE  /////////////////////////////////////////
//  ************************************************************************************
/////////////////////////////////////////////////////////////////////////////////////////

// Place scenes and commands here
// use 'toggled' and 'clicked' states of INPUTS to define your logics here




      
/////////////////////////////////////////////////////////////////////////////////////////
//  ************************************************************************************
/////////////////////////  COMMANDS FINISH HERE  ////////////////////////////////////////
//  ************************************************************************************
/////////////////////////////////////////////////////////////////////////////////////////


			for(k=0;k<MAXPINCOUNT;k++) {
				if(pinarr[k] != prevpinarr[k] && k>=INPUT_FIRST_INDEX && k<=OUTPUT_LAST_INDEX) {
					LogTheMessage(pinarr[k],k,name_array[k]);
				}
			}

			for(k=0;k<MAXPINCOUNT;k++) {
				prevpinarr[k] = pinarr[k];
				clicked[k] = 0;
				toggled[k] = 0;
			}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////		
/////////////////////////////////////////////////////////////////////

      j=0;
			index = 0;
//			printf("Prepared to new circle: clk_ack=%d, clk=%d, j=%d, index=%d\n",clk_ack,clk,j,index);
		} else {
		}
	}
	timeout_counter = 0;
	while(clk != clk_ack && timeout_counter <= MAX_READ_TIMEOUT) {
		clk_ack =  digitalRead(CLK_ACK_PIN);
//		printf("Read clk_ack=%d, current clk=%d, j=%d, index=%d, timeout_counter=%d\n",clk_ack,clk,j,index,timeout_counter);
		timeout_counter++;
		usleep(100);
	}

	if(timeout_counter >= MAX_READ_TIMEOUT) {
//		printf("timeout_counter=%d, clk_ack=%d, clk=%d, j=%d, index=%d - Goto INIT_NEW_EXCHANGE\n",timeout_counter,clk_ack,clk,j,index);
		goto INIT_NEW_EXCHANGE;
	}	
	clk = clk?0:1;
}

return 0;
}

