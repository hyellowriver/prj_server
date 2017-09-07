#include "sys_log.h"





sys_log *sys_log::m_instance = NULL;
sys_log::sys_log():
g_fpLog(NULL),
g_debug_level(1),
bDebug(false),
g_sLogFile("/tmp/test.log")
{


}
sys_log *sys_log::getInstance()
{
	if (m_instance == NULL){
		m_instance = new sys_log;
	} else {
		return m_instance;
	}


}

void sys_log::CreateDailyLogFile(FILE *p)
{
	time_t t;
	struct tm *ptm;
	char sLogfile[100];

	time(&t);
	ptm =localtime(&t);
	
	sprintf(sLogfile,"%s.%04d%02d%02d",g_sLogFile,1900+ptm->tm_year,ptm->tm_mon+1,ptm->tm_mday);

	//the file doesn't exist
	if(-1==access(sLogfile,F_OK))
	{
		if(g_fpLog)
			fclose(g_fpLog);
		
		g_fpLog = fopen(sLogfile,"a+");
		if(NULL==g_fpLog)
		{
			printf("fopen():%d: %s",__LINE__,strerror(errno));
			exit(1);
		}
	}
	else
	{
		if(NULL==g_fpLog)
		{
			g_fpLog = fopen(sLogfile,"a+");
			if(NULL==g_fpLog)
			{
				printf("fopen():%d: %s",__LINE__,strerror(errno));
				exit(1);
			}
		}
	}




}

int sys_log::write_log(int level, const char *format, ...)
{
	CreateDailyLogFile(g_fpLog);
	
	//if(g_debug_level < level)
	//	return 0;
	
	va_list args;
	char temp[4096]={0};

	time_t t;
	struct tm *ptm;
	struct timeb timebuffer;


	/*
   struct timeb {
                      time_t   time;
                      unsigned short millitm;
                      short    timezone;
                      short    dstflag;
                 };

   */
	time(&t);
	ptm =localtime(&t);
	
	ftime( &timebuffer );
	
	va_start(args, format);
	vsnprintf(temp, sizeof(temp), format, args);
	va_end(args);

	if(temp[0] && temp[strlen(temp)-1] =='\n')
		temp[strlen(temp)-1] =0;

	//NOTICE: fread,fwrite,fprintf are thread-safe
	if(g_fpLog)
	{
		if(g_bNewLine)
		{
			fprintf(g_fpLog, "\n[%02d.%02d %02d:%02d:%02d.%03hu] - %s%s\n", 
				ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
				timebuffer.millitm, LEVEL_STRING(g_debug_level),temp);
			g_bNewLine = false;
		}
		else
		{
			fprintf(g_fpLog, "[%02d.%02d %02d:%02d:%02d.%03hu] - %s%s\n", 
				ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
				timebuffer.millitm, LEVEL_STRING(g_debug_level),temp);
		}
		
		fflush(g_fpLog);
	}

	return 0;



}