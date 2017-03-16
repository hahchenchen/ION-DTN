/*
 *  ION runtime image initialization and driver.
 */

#include <bsp.h>
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/error.h>
#include <rtems/shell.h>
#include "platform.h"
#include "ion.h"
#include "rfx.h"
#include "ltp.h"
#include "bp.h"

/**socket add **/
    #include <sys/types.h>  
    #include <sys/socket.h>  
    #include <stdio.h>  
    #include <stdlib.h>  
    #include <errno.h>  
    #include <string.h>  
    #include <unistd.h>  
    #include <netinet/in.h>  
    #include <pthread.h>  

#define PORT 4321  
    #define BUFFER_SIZE 1024  
    #define MAX_QUE_CONN_NM 5  
    struct sockaddr_in server_sockaddr,client_sockaddr; 
    socklen_t  sin_size;
    int recvbytes,sendbytes;  
    int sockfd, client_fd;  
    char buf[BUFFER_SIZE];  
    char message[72];  






#ifndef NASA_PROTECTED_FLIGHT_CODE
#include "cfdp.h"
#endif

#define BPDRIVER

#ifdef BPDRIVER
#define	ION_NODE_NBR	1
#endif
#ifdef BPCOUNTER
#define	ION_NODE_NBR	2
#endif

#define TEST_NAME "ION 1"


#ifdef BPDRIVER
static void	createIonConfigFiles()
{
	uvast	nodenbr = ION_NODE_NBR;
	char	filenamebuf[80];
	int	fd;
	char	*ionconfigLines[] =	{
"wmSize 5000000\n",
"configFlags 1\n",
"heapWords 15000000\n",
"pathName /ion\n",
					};
	int	ionconfigLineCount = sizeof ionconfigLines / sizeof (char *);
	char	*globalLines[] =	{
"a contact +0 +86400 1 2 125000\n",
"a contact +0 +86400 2 1 125000\n",
"a range +0 +86400 1 2 1\n",
"m production 1000000\n",
"m consumption 1000000\n",
"m horizon +0\n"
					};
	int	globalLineCount = sizeof globalLines / sizeof (char *);
	char	*ionsecrcLines[] =	{
"1\n",
"e 1\n"
					};
	int	ionsecrcLineCount = sizeof ionsecrcLines / sizeof (char *);
	char	*ltprcLines[] =		{
"1 5 1200000\n",
"a span 2 5 120000 5 120000 1400 120000 1 'udplso 10.0.0.4:1113'\n",
"s 'udplsi 10.0.0.3:1113'\n",
"m screening n\n",
"w 1\n"
					};
	int	ltprcLineCount = sizeof ltprcLines / sizeof (char *);
	char	*bprcLines[] =		{
"1\n",
"a scheme ipn 'ipnfw' 'ipnadminep'\n",
"a endpoint ipn:1.0 q\n",
"a endpoint ipn:1.1 q\n",
"a endpoint ipn:1.2 q\n",
"a endpoint ipn:1.64 x\n",
"a endpoint ipn:1.65 x\n",
"a endpoint ipn:1.126 x\n",
"a endpoint ipn:1.127 x\n",
"a protocol ltp 1400 100\n",
"a induct ltp 1 ltpcli\n",
"a outduct ltp 2 ltpclo\n"
					};
	int	bprcLineCount = sizeof bprcLines / sizeof (char *);
	char	*ipnrcLines[] =		{
"a plan 2 ltp/2\n"
					};
	int	ipnrcLineCount = sizeof ipnrcLines / sizeof (char *);
	char	linebuf[255];
	char	**line;
	int	i;

	/*	Keep all ION configuration files in one directory.	*/

	if (mkdir("/ion", 0777) < 0)
	{
		perror("Can't create directory for config files");
		return;
	}

	/*	Create ionconfig file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionconfig", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionconfig file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ionconfigLines; i < ionconfigLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ionrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionrc file '%s'.\n", filenamebuf);
		return;
	}

	isprintf(linebuf, sizeof linebuf, "1 " UVAST_FIELDSPEC " /ion/node"
			UVAST_FIELDSPEC ".ionconfig\ns\n", nodenbr, nodenbr);
	oK(iputs(fd, linebuf));
	close(fd);

	/*	Create global.ionrc file.				*/

	istrcpy(filenamebuf, "/ion/global.ionrc", sizeof filenamebuf);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create global.ionrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = globalLines; i < globalLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ionsecrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionsecrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionsecrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ionsecrcLines; i < ionsecrcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ltprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ltprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ltprc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ltprcLines; i < ltprcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ipnrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ipnrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ipnrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ipnrcLines; i < ipnrcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create bprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".bprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .bprc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = bprcLines; i < bprcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	isprintf(linebuf, sizeof linebuf, "r 'ipnadmin /ion/node"
			UVAST_FIELDSPEC ".ipnrc'\ns\n", nodenbr);
	oK(iputs(fd, linebuf));
	close(fd);

#ifndef NASA_PROTECTED_FLIGHT_CODE
	/*	Create cfdprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".cfdprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .cfdprc file '%s'.\n", filenamebuf);
		return;
	}

	oK(iputs(fd, "1\ns bputa\n"));
	close(fd);
#endif
}
#endif


#ifdef BPCOUNTER
static void	createIonConfigFiles()
{
	uvast	nodenbr = ION_NODE_NBR;
	char	filenamebuf[80];
	int	fd;
	char	*ionconfigLines[] =	{
"wmSize 500000\n",
"configFlags 1\n",
"heapWords 1500000\n",
"pathName /ion\n",
					};
	int	ionconfigLineCount = sizeof ionconfigLines / sizeof (char *);
	char	*globalLines[] =	{
"a contact +0 +86400 1 2 125000\n",
"a contact +0 +86400 2 1 125000\n",
"a range +0 +86400 1 2 1\n",
"m production 1000000\n",
"m consumption 1000000\n",
"m horizon +0\n"
					};
	int	globalLineCount = sizeof globalLines / sizeof (char *);
	char	*ionsecrcLines[] =	{
"1\n",
"e 1\n"
					};
	int	ionsecrcLineCount = sizeof ionsecrcLines / sizeof (char *);
	char	*ltprcLines[] =		{
"1 5 1200000\n",
"a span 1 5 120000 5 120000 1400 120000 1 'udplso 10.0.0.3:1113'\n",
"s 'udplsi 10.0.0.4:1113'\n",
"m screening n\n",
"w 1\n"
					};
	int	ltprcLineCount = sizeof ltprcLines / sizeof (char *);
	char	*bprcLines[] =		{
"1\n",
"a scheme ipn 'ipnfw' 'ipnadminep'\n",
"a endpoint ipn:2.0 q\n",
"a endpoint ipn:2.1 q\n",
"a endpoint ipn:2.2 q\n",
"a endpoint ipn:2.64 x\n",
"a endpoint ipn:2.65 x\n",
"a endpoint ipn:2.126 x\n",
"a endpoint ipn:2.127 x\n",
"a protocol ltp 1400 100\n",
"a induct ltp 2 ltpcli\n",
"a outduct ltp 1 ltpclo\n"
					};
	int	bprcLineCount = sizeof bprcLines / sizeof (char *);
	char	*ipnrcLines[] =		{
"a plan 1 ltp/1\n"
					};
	int	ipnrcLineCount = sizeof ipnrcLines / sizeof (char *);
	char	linebuf[255];
	char	**line;
	int	i;
//printf("BPCOUNTER\n");
	/*	Keep all ION configuration files in one directory.	*/

	if (mkdir("/ion", 0777) < 0)
	{
		perror("Can't create directory for config files");
		return;
	}

	/*	Create ionconfig file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionconfig", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionconfig file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ionconfigLines; i < ionconfigLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ionrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionrc file '%s'.\n", filenamebuf);
		return;
	}

	isprintf(linebuf, sizeof linebuf, "1 " UVAST_FIELDSPEC " /ion/node"
			UVAST_FIELDSPEC ".ionconfig\ns\n", nodenbr, nodenbr);
	oK(iputs(fd, linebuf));
	close(fd);

	/*	Create global.ionrc file.				*/

	istrcpy(filenamebuf, "/ion/global.ionrc", sizeof filenamebuf);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create global.ionrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = globalLines; i < globalLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ionsecrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ionsecrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ionsecrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ionsecrcLines; i < ionsecrcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ltprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ltprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ltprc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ltprcLines; i < ltprcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create ipnrc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".ipnrc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .ipnrc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = ipnrcLines; i < ipnrcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	close(fd);

	/*	Create bprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".bprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .bprc file '%s'.\n", filenamebuf);
		return;
	}

	for (i = 0, line = bprcLines; i < bprcLineCount; line++, i++)
	{
		oK(iputs(fd, *line));
	}

	isprintf(linebuf, sizeof linebuf, "r 'ipnadmin /ion/node"
			UVAST_FIELDSPEC ".ipnrc'\ns\n", nodenbr);
	oK(iputs(fd, linebuf));
	close(fd);

#ifndef NASA_PROTECTED_FLIGHT_CODE
	/*	Create cfdprc file.					*/

	isprintf(filenamebuf, sizeof filenamebuf, "/ion/node" UVAST_FIELDSPEC
			".cfdprc", nodenbr);
	fd = iopen(filenamebuf, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		printf("Can't create .cfdprc file '%s'.\n", filenamebuf);
		return;
	}

	oK(iputs(fd, "1\ns bputa\n"));
	close(fd);
#endif
}

#endif



static int	startDTN()
{
	uvast	nodenbr = ION_NODE_NBR;
	char	cmd[80];
	int	count;
 //  printf("startDTN func\n");
	sm_ipc_init();
	isprintf(cmd, sizeof cmd, "ionadmin /ion/node" UVAST_FIELDSPEC
			".ionrc", nodenbr);
	pseudoshell(cmd);
	//printf("test 1\n");
	count = 5;
	while (rfx_system_is_started() == 0)
	{
		snooze(1);
		count--;
		if (count == 0)
		{
			writeMemo("[?] RFX start hung up, abandoned.");
			return -1;
		}
	}
   //printf("test 2\n");
	pseudoshell("ionadmin /ion/global.ionrc");
	snooze(1);
	isprintf(cmd, sizeof cmd, "ionsecadmin /ion/node" UVAST_FIELDSPEC
			".ionsecrc", nodenbr);
	pseudoshell(cmd);
	snooze(1);

	/*	Now start the higher layers of the DTN stack.		*/
  // printf("test 3\n");
	isprintf(cmd, sizeof cmd, "ltpadmin /ion/node" UVAST_FIELDSPEC
			".ltprc", nodenbr);
	pseudoshell(cmd);
	count = 5;
	while (ltp_engine_is_started() == 0)
	{
		snooze(1);
		count--;
		if (count == 0)
		{
			writeMemo("[?] LTP start hung up, abandoned.");
			return -1;
		}
	}

	isprintf(cmd, sizeof cmd, "bpadmin /ion/node" UVAST_FIELDSPEC
			".bprc", nodenbr);
	pseudoshell(cmd);
	count = 5;
	while (bp_agent_is_started() == 0)
	{
		snooze(1);
		count--;
		if (count == 0)
		{
			writeMemo("[?] BP start hung up, abandoned.");
			return -1;
		}
	}
/*
	isprintf(cmd, sizeof cmd, "lgagent ipn:" UVAST_FIELDSPEC ".127",
			nodenbr);
	pseudoshell(cmd);
	snooze(1);
*/
#ifndef NASA_PROTECTED_FLIGHT_CODE
	/*	Now start CFDP.						*/

	isprintf(cmd, sizeof cmd, "cfdpadmin /ion/node" UVAST_FIELDSPEC
			".cfdprc", nodenbr);
	pseudoshell(cmd);
	count = 5;
	while (cfdp_entity_is_started() == 0)
	{
		snooze(1);
		count--;
		if (count == 0)
		{
			writeMemo("[?] CFDP start hung up, abandoned.");
			return -1;
		}
	}
#endif
	return 0;
}

static int	testLoopback(int argc, char *argv[])
{
	char	cmd[80];

    int i=atoi(argv[1]);
    int j=atoi(argv[2]);

   // printf("i:%d\n", i);
	puts("Starting bpdriver test.");

	printf("argv[1]:%s\n",argv[1] );

	isprintf(cmd, sizeof cmd, "bpdriver " UVAST_FIELDSPEC" ipn:1.1 ipn:2.1 -"UVAST_FIELDSPEC,i,j);
	
	pseudoshell(cmd);
	
	//snooze(1);
	
//	isprintf(cmd, sizeof cmd, "bpsource ipn:" UVAST_FIELDSPEC".1 'Hello, world.'", ION_NODE_NBR);
	
	//pseudoshell(cmd);
	//snooze(1);
	
//	puts("Loopback test ended.");
	return 0;
}

/*
static int	socket_bpdriver()
{
	char	cmd[80];

    int i=1000;
    int j=1000;

   // printf("i:%d\n", i);
	puts("Starting bpdriver test.");

	//printf("argv[1]:%s\n",argv[1] );

	isprintf(cmd, sizeof cmd, "bpdriver " UVAST_FIELDSPEC" ipn:1.1 ipn:2.1 -"UVAST_FIELDSPEC,i,j);
	
	pseudoshell(cmd);
	
	//snooze(1);
	
//	isprintf(cmd, sizeof cmd, "bpsource ipn:" UVAST_FIELDSPEC".1 'Hello, world.'", ION_NODE_NBR);
	
	//pseudoshell(cmd);
	//snooze(1);
	
//	puts("Loopback test ended.");
	return 0;
}
*/



static int	testLoopback2()
{
	char	cmd[80];
   // printf("i:%d\n", i);
	puts("Starting bpcounter test.");

	//printf("argv[1]:%s\n",argv[1] );
	isprintf(cmd, sizeof cmd, "bpcounter ipn:2.1",ION_NODE_NBR);
	
	pseudoshell(cmd);
	
	//snooze(1);
	
//	isprintf(cmd, sizeof cmd, "bpsource ipn:" UVAST_FIELDSPEC".1 'Hello, world.'", ION_NODE_NBR);
	
	//pseudoshell(cmd);
	//snooze(1);
	
//	puts("Loopback test ended.");
	return 0;
}

static int	stopDTN(int a1, int a2, int a3, int a4, int a5,
			int a6, int a7, int a8, int a9, int a10)
{
#ifndef NASA_PROTECTED_FLIGHT_CODE
	/*	Stop CFDP.						*/

	pseudoshell("cfdpadmin .");
	while (cfdp_entity_is_started())
	{
		snooze(1);
	}
#endif

	/*	Stop BP.						*/

	pseudoshell("bpadmin .");
	while (bp_agent_is_started())
	{
		snooze(1);
	}

	/*	Stop LTP.					*/

	pseudoshell("ltpadmin .");
	while (ltp_engine_is_started())
	{	
		snooze(1);
	}

	/*	Stop rfxclock.						*/

	pseudoshell("ionadmin .");
	while (rfx_system_is_started())
	{
		snooze(1);
	}

	/*	Erase all ION data in DRAM.				*/

	ionTerminate();
	return 0;
}

int ionstart()
{
puts("Inside Init(), creating configuration files.");
	createIonConfigFiles();
	puts("Inside Init(), spawning ION startup tasks.");

if (startDTN() < 0)
	{
		writeMemo("[?] Can't start ION.");
	}

return 0;

}

int ionstop()
{

 puts("Stopping ION.");
  oK(stopDTN(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	puts("ION stopped.");

return 0;

}



rtems_task	test_main()
{
	rtems_shell_env_t env;

	pthread_t thread[1];  
    
	rtems_shell_add_cmd("ionstart","misc","ionstart",ionstart);
	rtems_shell_add_cmd("ionstop","misc","ionstop",ionstop);
	rtems_shell_add_cmd("bpdriver","misc","bpdriver",testLoopback);//c add 
    rtems_shell_add_cmd("bpcounter","misc","bpcounter",testLoopback2);//c add 
	
     memset(&env, 0, sizeof(env));
  rtems_shell_main_loop( &env );


     
   /*
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1)  
               {  
                   perror("socket");  
                   exit(1);  
               }  
       
      printf("Socket id = %d\n",sockfd);  
  
      server_sockaddr.sin_family = AF_INET;  
      server_sockaddr.sin_port = htons(PORT);  
      server_sockaddr.sin_addr.s_addr = INADDR_ANY;  
      bzero(&(server_sockaddr.sin_zero), 8);  
      int i = 1;
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));  
   
        if (bind(sockfd, (struct sockaddr *)&server_sockaddr,sizeof(struct  
    sockaddr)) == -1)  
               {  
                   perror("bind");  
                   exit(1);  
               }  
        printf("Bind success!\n");  
    
       if (listen(sockfd, MAX_QUE_CONN_NM) == -1)  
              {  
                  perror("listen");  
                  exit(1);  
               }  
       printf("Listening....\n");  
   
       if ((client_fd = accept(sockfd,(struct sockaddr *)&client_sockaddr,  
    &sin_size)) == -1)  
               {  
                  perror("accept");  
                  exit(1);  
               }  
    while(1){  
      memset(buf , 0, sizeof(buf));  
    
      if ((recvbytes = recv(client_fd, buf, BUFFER_SIZE, 0)) == -1)  
             {  
                 perror("recv");  
                 exit(1);  
             }  
       else if(recvbytes==0)  
      {  
      printf("失败\n");  
      exit(1);  
       }  
      printf("Received a message: %s\n", buf);  

      if ((buf[0] & 0x01))
      	ionstart();

      else if ((buf[0] & 0x02))
      {
      	printf("222222\n");
      	socket_bpdriver();
      }
      else if ((buf[0] & 0x04))
      {
      	printf("444\n");
      	ionstop();
      }

    //  pthread_create(&thread[0],NULL,thrd_send,(void*)0);  
       
    }  


    close(sockfd);  
    
   */
	exit(0);
}

void	inferUtcDelta(char *correctUtcTimeStamp)
{
	IonVdb	*ionvdb = getIonVdb();
	time_t	correctUtcTime = readTimestampUTC(correctUtcTimeStamp, 0);
	time_t	clocktime = getUTCTime() + ionvdb->deltaFromUTC;
	int	delta = clocktime - correctUtcTime;
	char	buffer[80];

	CHKVOID(setDeltaFromUTC(delta) == 0);
	sprintf(buffer, "[i] Delta from UTC revised, is now %d.", delta);
	writeMemo(buffer);
}

void	showUtcDelta()
{
	IonVdb	*ionvdb = getIonVdb();
	char	buffer[80];

	sprintf(buffer, "[i] Delta from UTC is %d.", ionvdb->deltaFromUTC);
	writeMemo(buffer);
}

/*	*	*	RTEMS configuration	*	*	*	*/

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define	CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define	CONFIGURE_MAXIMUM_SEMAPHORES				30
#define	CONFIGURE_MAXIMUM_MESSAGE_QUEUES			10
#define	CONFIGURE_MAXIMUM_TASKS					50

#ifndef CONFIGURE_MICROSECONDS_PER_TICK
#define	CONFIGURE_MICROSECONDS_PER_TICK				10000
#endif
#ifndef CONFIGURE_TICKS_PER_TIMESLICE
#define	CONFIGURE_TICKS_PER_TIMESLICE				10
#endif
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS		40
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_MAXIMUM_POSIX_THREADS				40
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES				10
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES		10
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES			100
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES			10

#define	CONFIGURE_STACK_CHECKER_ON
#define	CONFIGURE_ZERO_WORKSPACE_AUTOMATICALLY			TRUE

#define	CONFIGURE_DISABLE_CLASSIC_NOTEPADS

#define	CONFIGURE_INIT

#undef Object
//#include </home/c/developement/rtems/rtems-libbsd/testsuite/include/rtems/bsd/test/default-network-init.h>
//#include <rtems/confdefs.h>
#include <default-network-init.h>
#include <rtems/shell.h>
#include <rtems/shell.h>

#include <rtems/netcmds-config.h>

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL

#define CONFIGURE_SHELL_USER_COMMANDS \
    &rtems_shell_HOSTNAME_Command, \
    &rtems_shell_PING_Command, \
    &rtems_shell_ROUTE_Command, \
    &rtems_shell_NETSTAT_Command, \
    &rtems_shell_IFCONFIG_Command

#include <rtems/shellconfig.h>
/* Loopback Network Configuration needed to prevent linking with dummy.o */

//extern int rtems_bsdnet_loopattach(struct rtems_bsdnet_ifconfig *, int);

//static struct rtems_bsdnet_ifconfig	loopback_config =
//{
//	"lo0",				/* name */
//	rtems_bsdnet_loopattach,	/* attach function */
//	NULL,				/* link to next interface */
//	"127.0.0.1",			/* IP address */
//	"255.0.0.0",			/* IP net mask */
//};

//struct rtems_bsdnet_config		rtems_bsdnet_config =
//{
//	NULL,//&loopback_config,		/* Network interface */
//	NULL,				/* Use fixed network configuration */
//	0,				/* Default network task priority */
//	0,				/* Default mbuf capacity */
//	0,				/* Default mbuf cluster capacity */
//	"127.0.0.1",			/* Host name */
//	"localdomain",			/* Domain name */
//	"127.0.0.1",			/* Gateway */
//	"127.0.0.1",			/* Log host */
//	{"127.0.0.1" },			/* Name server(s) */
//	{"127.0.0.1" },			/* NTP server(s) */
//	2,				/* sb_efficiency */
//	8192,				/* udp_tx_buf_size */
//	8192,				/* udp_rx_buf_size */
//	8192,				/* tcp_tx_buf_size */
//	8192				/* tcp_rx_buf_size */
//};
