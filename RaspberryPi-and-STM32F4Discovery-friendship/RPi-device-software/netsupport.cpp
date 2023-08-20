// EXAMPLE OF INTEGRATION WITH FAMOUS 'HOME ASSISTANT' SOFTWARE for Raspberry Pi

#include "netsupport.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "mqttcore.h"

#define HOMEASSISTANTWEBPORT	8123
#define HOMEASSISTANTMQTTPORT	1883
#define LOCALMQTTPORT		51883
#define isvalidsock(s) ((s)>=0)
#define MAXBUFFERSIZE		2048

//////////////////////////////////////////////////////////////

static void prepare_web_conn(struct sockaddr_in *sap, char *protocol) {
int port=HOMEASSISTANTWEBPORT;
bzero(sap, sizeof (*sap));
sap->sin_family = AF_INET;
sap->sin_addr.s_addr = inet_addr("127.0.0.1");
sap->sin_port = htons(port);
return;
}

//////////////////////////////////////////////////////////////

static void prepare_mqtt_conn(struct sockaddr_in *sap, char *protocol) {
int port=HOMEASSISTANTMQTTPORT;
bzero(sap, sizeof (*sap));
sap->sin_family = AF_INET;
sap->sin_addr.s_addr = inet_addr("127.0.0.1");
sap->sin_port = htons(port);
return;
}


//////////////////////////////////////////////////////////////

static void set_local_address(struct sockaddr_in *sap, char *protocol) {
int port=LOCALMQTTPORT;
bzero(sap,sizeof (*sap));
sap->sin_family = AF_INET;
sap->sin_addr.s_addr = htonl(INADDR_ANY);
sap->sin_port = htons(port);
return;
}

//////////////////////////////////////////////////////////////

void *NetHandler(void *p) {

struct sockaddr_in	local_mqtt;
struct sockaddr_in	remote_mqtt;
struct sockaddr_in	remote_web;
struct sockaddr_in	peer_mqtt;
int	local_sock;
int	si_mqtt;
const int on = 1;
unsigned int peermqttlen;
unsigned char	recv_buf[MAXBUFFERSIZE] = {0x00};

REINITNETWORK:
set_local_address(&local_mqtt,INADDR_ANY);
local_sock = socket(AF_INET,SOCK_STREAM,0);
if (!isvalidsock(local_sock)) {
	sleep(1);
	goto REINITNETWORK;
}
if (setsockopt(local_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) {
	sleep(1);
	goto REINITNETWORK;
}
if (bind(local_sock,(struct sockaddr *)&local_mqtt,sizeof(local_mqtt))) {
	sleep(1);
	goto REINITNETWORK;
}
if (listen(local_sock,100)) {
	sleep(1);
	goto REINITNETWORK;
}


while(1) {
        si_mqtt = accept(local_sock,(struct sockaddr *)&peer_mqtt,&peermqttlen);
        if (!isvalidsock(si_mqtt)) {
		sleep(1);
		goto REINITNETWORK;
        }
	recv(si_mqtt,recv_buf,MAXBUFFERSIZE,0);
	printf("%s\n",recv_buf);
	printf("new connection\n");
	shutdown(si_mqtt,2);

	sleep(1);
}

return NULL;
}
