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


#include <rtems/bdbuf.h>
#include <rtems/console.h>
#include <rtems/ftpd.h>
#include <rtems/media.h>
#include <assert.h>
/*	When CFDP is included in the build, the test fails due to
 *	insufficient memory resources.  Need to figure out how to
 *	get RTEMS to allocate enough memory for the test, but for
 *	now we just exclude CFDP from the build (since it is not
 *	involved in sending the test bundle.				*/
#define	NASA_PROTECTED_FLIGHT_CODE

#ifndef NASA_PROTECTED_FLIGHT_CODE
#include "cfdp.h"
#endif

#define BPDRIVER

#ifdef HOST2_3NODE
#define	ION_NODE_NBR	2
#endif
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
"a span 2 5 120000 5 120000 1400 120000 1 'udplso 192.168.1.101:1113'\n",
"s 'udplsi 192.168.1.105:1113'\n",
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


static int	startDTN()
{
	uvast	nodenbr = ION_NODE_NBR;
	char	cmd[80];
	int	count;

	sm_ipc_init();
	isprintf(cmd, sizeof cmd, "ionadmin /ion/node" UVAST_FIELDSPEC
			".ionrc", nodenbr);
	pseudoshell(cmd);
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

	pseudoshell("ionadmin /ion/global.ionrc");
	snooze(1);
	isprintf(cmd, sizeof cmd, "ionsecadmin /ion/node" UVAST_FIELDSPEC
			".ionsecrc", nodenbr);
	pseudoshell(cmd);
	snooze(1);

	/*	Now start the higher layers of the DTN stack.		*/

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

	isprintf(cmd, sizeof cmd, "lgagent ipn:" UVAST_FIELDSPEC ".127",
			nodenbr);
	pseudoshell(cmd);
	snooze(1);

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
    rtems_status_code sc;
	pthread_t thread[1];

	rtems_shell_add_cmd("ionstart","misc","ionstart",ionstart);
	rtems_shell_add_cmd("ionstop","misc","ionstop",ionstop);
	rtems_shell_add_cmd("bpdriver","misc","bpdriver",testLoopback);//c add 



sc = rtems_shell_init("SHLL", 16 * 1024, 1, CONSOLE_DEVICE_NAME,
	    false, true, NULL);
	assert(sc == RTEMS_SUCCESSFUL);
	exit(0);
}

#define DEFAULT_EARLY_INITIALIZATION

static rtems_status_code
media_listener(rtems_media_event event, rtems_media_state state,
    const char *src, const char *dest, void *arg)
{
	printf(
		"media listener: event = %s, state = %s, src = %s",
		rtems_media_event_description(event),
		rtems_media_state_description(state),
		src
	);

	if (dest != NULL) {
		printf(", dest = %s", dest);
	}

	if (arg != NULL) {
		printf(", arg = %p\n", arg);
	}

	printf("\n");

	if (event == RTEMS_MEDIA_EVENT_MOUNT && state == RTEMS_MEDIA_STATE_SUCCESS) {
		char name[256];
		int n = snprintf(&name[0], sizeof(name), "%s/test.txt", dest);
		FILE *file;

		assert(n < (int) sizeof(name));

		printf("write file %s\n", &name[0]);
		file = fopen(&name[0], "w");
		if (file != NULL) {
			const char hello[] = "Hello, world!\n";

			fwrite(&hello[0], sizeof(hello) - 1, 1, file);
			fclose(file);
		}
	}

	return RTEMS_SUCCESSFUL;
}


static void
early_initialization(void)
{
	rtems_status_code sc;

	sc = rtems_bdbuf_init();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_listener_add(media_listener, NULL);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_server_initialize(
		200,
		32 * 1024,
		RTEMS_DEFAULT_MODES,
		RTEMS_DEFAULT_ATTRIBUTES
	);
	assert(sc == RTEMS_SUCCESSFUL);
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

#define	CONFIGURE_MAXIMUM_SEMAPHORES				20
#define	CONFIGURE_MAXIMUM_MESSAGE_QUEUES			10
#define	CONFIGURE_MAXIMUM_TASKS					40

#ifndef CONFIGURE_MICROSECONDS_PER_TICK
#define	CONFIGURE_MICROSECONDS_PER_TICK				10000
#endif
#ifndef CONFIGURE_TICKS_PER_TIMESLICE
#define	CONFIGURE_TICKS_PER_TIMESLICE				10
#endif
#define CONFIGURE_FILESYSTEM_DOSFS

#define CONFIGURE_MAXIMUM_POSIX_THREADS				40
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES			100
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES			10

#define	CONFIGURE_STACK_CHECKER_ON
#define	CONFIGURE_ZERO_WORKSPACE_AUTOMATICALLY			TRUE

#define	CONFIGURE_DISABLE_CLASSIC_NOTEPADS

#define	CONFIGURE_INIT

#undef Object

#define DEFAULT_NETWORK_DHCPCD_ENABLE

#include <default-network-init.h>
#define CONFIGURE_SHELL_COMMANDS_INIT
#include <bsp/irq-info.h>
#include <rtems/netcmds-config.h>

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_ARP_Command, \
  &rtems_shell_HOSTNAME_Command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_SYSCTL_Command, \
  &rtems_shell_IFCONFIG_Command, \
  &rtems_shell_VMSTAT_Command, \
  &rtems_shell_WPA_SUPPLICANT_Command, \
  &rtems_shell_WPA_SUPPLICANT_FORK_Command

#define CONFIGURE_SHELL_COMMAND_CPUINFO
#define CONFIGURE_SHELL_COMMAND_CPUUSE
#define CONFIGURE_SHELL_COMMAND_PERIODUSE
#define CONFIGURE_SHELL_COMMAND_STACKUSE
#define CONFIGURE_SHELL_COMMAND_PROFREPORT

#define CONFIGURE_SHELL_COMMAND_CP
#define CONFIGURE_SHELL_COMMAND_PWD
#define CONFIGURE_SHELL_COMMAND_LS
#define CONFIGURE_SHELL_COMMAND_LN
#define CONFIGURE_SHELL_COMMAND_LSOF
#define CONFIGURE_SHELL_COMMAND_CHDIR
#define CONFIGURE_SHELL_COMMAND_CD
#define CONFIGURE_SHELL_COMMAND_MKDIR
#define CONFIGURE_SHELL_COMMAND_RMDIR
#define CONFIGURE_SHELL_COMMAND_CAT
#define CONFIGURE_SHELL_COMMAND_MV
#define CONFIGURE_SHELL_COMMAND_RM
#define CONFIGURE_SHELL_COMMAND_MALLOC_INFO

#define CONFIGURE_SHELL_COMMAND_FDISK
#define CONFIGURE_SHELL_COMMAND_BLKSTATS
#define CONFIGURE_SHELL_COMMAND_BLKSYNC
#define CONFIGURE_SHELL_COMMAND_MSDOSFMT
#define CONFIGURE_SHELL_COMMAND_DF
#define CONFIGURE_SHELL_COMMAND_MOUNT
#define CONFIGURE_SHELL_COMMAND_UNMOUNT
#define CONFIGURE_SHELL_COMMAND_MSDOSFMT

#include <rtems/shellconfig.h>

/* Loopback Network Configuration needed to prevent linking with dummy.o */


