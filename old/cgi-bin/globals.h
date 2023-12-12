// Project: SSOD
// Content: Global variables/structures

#include "mysql.h"
#include "mysql_config.h"

static char *_itoa(unsigned long i, char *a, unsigned long r)
{
	if (i/r > 0) a = _itoa(i/r,a,r);
	*a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
	return a+1;
} 

char *itoa(long i, char *a, long r)
{
	r = ((r < 2) || (r > 36)) ? 10 : r;
	
	if (i < 0)
	{
		*a = '-';
		*_itoa(-i,a+1,r) = 0;
	}
	else
	{
		*_itoa(i,a,r) = 0;
	}
	
	return a;
}


#define MAX_BYTES 32768

#define CR "<BR>"


char query[4096];
char me[1024];

MYSQL my_connection;

char **formData = NULL,**formName = NULL;
int numItems = 0;

char inbuff[MAX_BYTES];
char stdin_buffer[65535];

void log(const char *fmt,...);

//////////////////////////////////////////////////////////////////////////////////////////////////
// enum: PlayerRace
// An enumeration used to hold a player's race inside a Player class.
//////////////////////////////////////////////////////////////////////////////////////////////////


enum PlayerRace
{
	RErr = 0,
	Human,
	Elf,
	Orc,
	Dwarf,
	LesserOrc,
	Barbarian,
	Goblin,
	DarkElf

} PRace;


//////////////////////////////////////////////////////////////////////////////////////////////////
// enum: PlayerProfession
// An enumeration used to hold a player's profession inside a Player class.
//////////////////////////////////////////////////////////////////////////////////////////////////


enum PlayerProfession
{
	PErr = 0,
	Warrior,
	Wizard,
	Thief,
	Woodsman,
	Assassin,
	Mercenary

} PProfession;


char* _Profession(PlayerProfession P);
char* _Race(PlayerRace R);

bool mysql_init_conn(MYSQL &my_connection)
{
	int res;
	mysql_init(&my_connection);
	return mysql_real_connect(&my_connection, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0);
}


