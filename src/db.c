#include "db.h"

#define MAX_DB_COUNT 512
#define DB_MAGIC 0xcc

#ifdef RELEASE_DB
static u16 dbstarts[MAX_DB_COUNT];
static u16 dbsizes[MAX_DB_COUNT];
static u16 dbcount = 0;
#endif

void getDbStarts(void)
{
#ifdef RELEASE_DB
	u8 * db = (u8 *) 0xff0000;
	u8 inDb = 0, prevInDb = 0;
	u16 pos = 0;
	while ((u32) db < MEMORY_HIGH)
	{
		inDb = (db[0] == DB_MAGIC) && (db[1] == DB_MAGIC);
		
		if (inDb && !prevInDb)
		{
			dbstarts[pos] = (u32) db & 0xffff;
		}
		else if (prevInDb && !inDb)
		{
			dbsizes[pos] = ((u32) db & 0xffff) - dbstarts[pos];
			pos++;
		}
		
		prevInDb = inDb;
		++db;
	}
	
	dbcount = pos;
#endif
}

void checkDbs(u8 onlyCorrupted, u8 die)
{
#ifdef RELEASE_DB
	if (!onlyCorrupted)
		KLog_U1("dbcount ", dbcount);
	
	for (u16 i = 0; i < dbcount; ++i)
	{
		u8 * db;
		u16 corr = 0;
		
		db = (u8 *) 0xff0000;
		db = &db[dbstarts[i]];
		
		for (u16 j = 0; j < dbsizes[i]; ++j)
		{
			if (*db != DB_MAGIC)
				corr++;
			db++;
		}
		
		if (!onlyCorrupted || corr)
		{
			char s[100];
			char sv[10];
			
			db = (u8 *) 0xff0000;
			db = &db[dbstarts[i]];
			
			strcpy(s, "db at ");
			intToHex((u32) db, sv, 8);
			strcat(s, sv);
			strcat(s, " end ");
			intToHex((u32) &db[dbsizes[i]], sv, 8);
			strcat(s, sv);
			strcat(s, " corr ");
			intToStr(corr, sv, 8);
			strcat(s, sv);
			
			if (onlyCorrupted && die)
				SYS_die(s);
			else
				KLog(s);
		}
	}
#endif
}

