#define	PUD_DOWN	0
#define	PUD_DOWN	1
#define	OUTPUT		0
#define	INPUT		0
#define	LOW		0
#define HIGH		1


void digitalWrite(int p1,int p2);
unsigned int digitalRead(int p);
void pinMode(int p1,int p2);
void pullUpDnControl(int p1,int p2);
int wiringPiSetup();
