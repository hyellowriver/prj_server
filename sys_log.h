 #include "io_mutex.h"
#include <stdarg.h>
 #include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#ifndef SYS_LOG_H
#define SYS_LOG_H

class sys_log{
private:
	static sys_log *m_instance;


public:

	FILE* g_fpLog ;
	int g_debug_level ;
	bool bDebug;
	bool g_bNewLine;
	bool  g_bWantExit;
	const char *g_sLogFile;
public:
	sys_log();
	static sys_log *getInstance();
	void CreateDailyLogFile(FILE *p);
	int write_log(int level, const char *format, ...);
	inline const char* LEVEL_STRING(int n);


};

inline const char* sys_log::LEVEL_STRING(int n) 
{
	if(n==1) return "[ERROR]";
	else if(n==2) return "[INFO]";
	else if(n==3) return "[DEBUG]";
} 
#endif