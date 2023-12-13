// Project: SSOD
// Content: Game-level Helper functions

#include <iostream>
#include <ctype.h>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <sstream>

#include "mysql.h"
#include "mysql_config.h"

const long Levels[20] = {0,10,20,40,80,160,250,500,550,1000,1500,2000,2500,3000,3500,4000,5000,8000,10000,20000};

// Returns true if its "legal" to go from "current" to "next"...

char VPList[1024];

bool ValidNextParagraph(char* Current, char* Next)
{
	char Paralist[256][50];
	char para[1024], tag[1024], p_text[1024];
	strcpy(Paralist[0],Current);
	long xx = 1;
	VPList[0] = '\0';


                   int res;
                   MYSQL_RES *a_res;
                   MYSQL_ROW a_row;
                   char query[1024];
                   char pdata[65536];
                   char P[1024];

		   sprintf(query,"SELECT data FROM game_locations WHERE id=%s",Current);
                   {
                                       res = mysql_query(&my_connection,query);
                                       if (!res)
                                       {
                                            a_res = mysql_use_result(&my_connection);
                                            if (a_res)
                                            {
                                                   a_row = mysql_fetch_row(a_res);
                                                   while (a_row)
                                                   {
                                                         unsigned int field_count;
                                                         field_count = 0;
                                                         if(mysql_field_count(&my_connection) == 0)
                                                         {
                                                                   cout << "ERROR: SELECT returned no data, the location you tried to go to does not exist in the database!";
                                                         }
                                                         while (field_count < mysql_field_count(&my_connection))
                                                         {
                                                                   strcpy(pdata,a_row[field_count++]);
                                                         }
                                                         a_row = mysql_fetch_row(a_res);
                                                  }
                                          }
                                    }
                                    else
                                    {
                                          cout << "SELECT error: " << mysql_error(&my_connection);
                         	    }
                       }

	std::stringstream parafile(pdata);

	while (!parafile.eof())
	{
		if (parafile.peek() == '\n')
		{
			cout << CR;
		}

		parafile >> p_text;
		char NeatVersion[1024];
		strcpy(NeatVersion,p_text);
		if (parafile.eof())
		{
			break;
		}

		strncpy(tag,p_text,20);
		if (strstr(tag,"<paylink="))
		{
			int i=0, t=0;
			while (p_text[i++] != '=');
			char pnum[256],cst[256];
			pnum[0] = '\0';
			cst[0] = '\0';
			while (p_text[i] != ',')
			{
				cst[t++] = p_text[i++];
			}
			cst[t] = '\0';
			i++;
			t=0;
			while (p_text[i] != '>')
			{
				pnum[t++] = p_text[i++];
			}
			pnum[t] = '\0';
			strcpy(Paralist[xx++],pnum);
		}

		if ((strstr(Lower(tag),"<link=")) || (strstr(Lower(tag),"<autolink=")))
		{
			int i=0, t=0;
			while (p_text[i++] != '=');
			char pnum[256];
			pnum[0] = '\0';
			while (p_text[i] != '>')
			{
				pnum[t++] = p_text[i++];
			}
			pnum[t] = '\0';
			strcpy(Paralist[xx++],pnum);
		}

	}
	for (int ss = 0; ss<=xx-1; ss++)
	{
		strcat(VPList,Paralist[ss]);
		strcat(VPList," ");
	}
	VPList[strlen(VPList)-1] = '\0';
	for (int y = 0; y<=xx-1; y++)
		if (!strcmp(Paralist[y],Next)) return true;
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

char __ColourCode[1024];

char* ColourCode(char* Type)
{
	strcpy(__ColourCode,"#FF3030");
	if (!strcmp(Type,"SPELL"))
	{
		strcpy(__ColourCode,"yellow");
	}
	if (!strcmp(Type,"HERB"))
	{
		strcpy(__ColourCode,"green");
	}
	if ((!strncmp(Type,"A",1)) || (!strncmp(Type,"W",1)))
	{
		strcpy(__ColourCode,"#8F8F8F");
	}
	return __ColourCode;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Add a player to a player file, or create the file if it doesn't already exist and write the
// first player into it.
//////////////////////////////////////////////////////////////////////////////////////////////////

void AddPlayer(Player &P, char* Key)
{
	P.Put(false, Key);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// GetPlayer: Read a player from any given arbitary position in a file
//////////////////////////////////////////////////////////////////////////////////////////////////

bool GetPlayer(Player &P, char* Filename)
{
	P.Get(false,Filename);
	return true;
}

bool GetPlayer2(Player &P, char* Filename)
{
        P.GetWithDebug(Filename);
        return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// GetByGuid: Retrieves a player from the master file. A GUID must be passed as the parameter,
// which will be decoded to a record number (the only reason for this step is to make it more
// difficult for crackers to access arbitary records in the master file and hack around with
// other people's players!)
//////////////////////////////////////////////////////////////////////////////////////////////////

Player GetByGuid(char* GUID)
{
	Player ThePlayer;
	ThePlayer.Get(false,GUID);
	return ThePlayer;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

void PutByGuid(char* GUID, Player AnyPlayer)
{
	AnyPlayer.Put(false,GUID);
	return;
}

int DropAllItems(char* GUID, Player &AnyPlayer)
{
	char C[1024],A[1024],B[1024];
	int res,i,e;
	char query[2048];
	MYSQL my_connection2;
	AnyPlayer.Get(false,GUID);
	AnyPlayer.GetParagraph(C);
	if ((atoi(C)==1306) || (atoi(C)==1325) || (atoi(C)==1332))
	{
		AnyPlayer.Possessions.Clear();
		AnyPlayer.Herbs.Clear();
		PutByGuid(GUID,AnyPlayer);
		return 1;
	}
	mysql_init(&my_connection2);
	if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
	{
		for (i = 0; i < 30; i++)
		{
			AnyPlayer.Possessions.GetItem(i,A,B);
			if (strcmp(A,"[none]"))
			{
				sprintf(query,"INSERT INTO game_dropped_items VALUES ('%s',\"%s\",'%s')",C,A,B);
				res = mysql_query(&my_connection2,query);
			}
		}
		for (i = 0; i < 30; i++)
		{
			AnyPlayer.Herbs.GetItem(i,A,B);
			if (strcmp(A,"[none]"))
			{
				sprintf(query,"INSERT INTO game_dropped_items VALUES ('%s',\"%s\",'%s')",C,A,B);
				 res = mysql_query(&my_connection2,query);
			}
		}
		sprintf(query,"DELETE FROM game_owned_items WHERE owner_guid='%s'",GUID);
		res = mysql_query(&my_connection2,query);
	}
	mysql_close(&my_connection2);
	AnyPlayer.Possessions.Clear();
	AnyPlayer.Herbs.Clear();
	PutByGuid(GUID,AnyPlayer);
	return 0;
}

int RestoreBackup(char* GUID)
{
	char C[1024],A[1024],B[1024];
	int res,i,e;
	char query[2048];
	MYSQL my_connection2;
	Player AnyPlayer;
	DropAllItems(GUID,AnyPlayer);
	AnyPlayer.GetParagraph(C);
	e = AnyPlayer.GetExperience();

	// clear out herbs, spells, possessions
	AnyPlayer.Get(true,GUID);
	AnyPlayer.Possessions.Clear();
	AnyPlayer.Spells.Clear();
	AnyPlayer.Herbs.Clear();

        AnyPlayer.Possessions.Add("Hunting Dagger","W1");
	AnyPlayer.Possessions.Add("Leather Coat","A1");

	AnyPlayer.SetWeapon("Hunting Dagger",1);
	AnyPlayer.SetArmour("Leather Coat",1);

	// Because ppl complained the game was too hard (lamers)
	AnyPlayer.Possessions.Add("Stamina Potion","ST+9");
	AnyPlayer.Possessions.Add("Skill Potion","SK+9");

	long Level = 0;
	long j = 0;
	while ((e >= Levels[Level]) && (Level != 19)) Level++;
	Level--;
	if (Level > 2)
	{
		AnyPlayer.SetExperience(e-((Levels[Level+1]-Levels[Level])/4));
		j = ((Levels[Level+1]-Levels[Level])/4);
	}
	else
	{
		AnyPlayer.SetExperience(e);
		Level = 0;
		j = 0;
	}
	AnyPlayer.ResetToSpawnPoint();						       
	AnyPlayer.Put(false,GUID);

	// returns amount of experience lost
	return j;
}


void PutBackupByGuid(char* GUID, Player AnyPlayer)
{
	AnyPlayer.Put(true,GUID);
	return;
}

void LpToGuid(char* Fn, char* Login, char* Pass) // Convert a login name and password to a GUID
{
	unsigned long g1 = 0xBEEFDEAD+Login[0];
	unsigned long g2 = 0x53CCA02B-Login[0];
	unsigned long g3 = 0x61CF9250+Pass[0];
	unsigned long g4 = 0x12345678-Pass[0];

	for (unsigned int i = 0; i<=strlen(Login); i++)
	{
		g1 = g1 * (Login[i] % 10) + g2;
		g2 = (g2 / (g1+5)) ^ g3;
		g3 = (g2 + g1 + g3) ^ 0x6 + Login[i];
		g4 = (g1 + g3 - g2) ^ g1 % (i+1) - Login[i];
		if (i<=strlen(Pass))
		{
			g1 = g1 + Pass[i];
			if (i>0)
			{
				g2 = g2 + Pass[i-1];
			}
		}
	}
	char s1[256],s2[256],s3[256],s4[256],s[256];
	itoa(g1,s1,16);
	itoa(g2,s2,16);
	itoa(g3,s3,16);
	itoa(g4,s4,16);
	strcpy(s,s1);
	strcat(s,s2);	
	strcat(s,s3);
	strcat(s,s4);
	strcpy(Fn,s);
	for (unsigned int q=0; q<=strlen(Fn); q++)
	{
		if (Fn[q] == '-')
		{
			Fn[q] = (char)((q % 10) + 48);
		}
	}
	return;
}

bool IsCombatSpell(char* Name)
{
	if ((!strcmp(Name,"fire")) || (!strcmp(Name,"water")) || (!strcmp(Name,"strength")) ||
		(!strcmp(Name,"bolt")) || (!strcmp(Name,"fasthands")) || (!strcmp(Name,"thunderbolt")) ||
		(!strcmp(Name,"jump")) || (!strcmp(Name,"run")) || (!strcmp(Name,"invisible")) ||
		(!strcmp(Name,"grow")) || (!strcmp(Name,"heateyes")) || (!strcmp(Name,"espsurge")) ||
		(!strcmp(Name,"afterimage")) || (!strcmp(Name,"psychism")) || (!strcmp(Name,"growweapon")))
		{
			return true;
		}
	
	return false;
}

long GetSpellRating(char* Name)
{
	if (!strcmp(Name,"fire")) return 2;
	if (!strcmp(Name,"water")) return 4;
	if (!strcmp(Name,"strength")) return 2;
	if (!strcmp(Name,"bolt")) return 4;
	if (!strcmp(Name,"fasthands")) return 2;
	if (!strcmp(Name,"thunderbolt")) return 8;
	if (!strcmp(Name,"jump")) return 4;
	if (!strcmp(Name,"run")) return 2;
	if (!strcmp(Name,"invisible")) return 5;
	if (!strcmp(Name,"heateyes")) return 5;
	if (!strcmp(Name,"espsurge")) return 9;
	if (!strcmp(Name,"afterimage")) return 7;
	if (!strcmp(Name,"psychism")) return 5;
	if (!strcmp(Name,"growweapon")) return 4;
	if (!strcmp(Name,"vortex")) return 10;
	return 0;
}

char* Maxed(long A,long B)
{
	if (A == B)
	{
		return " <b>MAX</b>";
	}
	else
	{
		return " ";
	}
}

bool NotGotYet(char* Para,char* Item,char* GotFrom) // returns true if the user hasnt found an item before
{
	char F[32768];
	strcpy(F," [");
	strcat(F,Item);
	strcat(F,Para);
	strcat(F,"]");
	if (strstr(GotFrom,F))
	{
		return false;
	}
	else
	{
		return true;
	}
}


void DispScrolls(Player &SomePlayer)
{
	char Scrolls[5], New[1024];
	itoa(SomePlayer.GetScrolls(),Scrolls,10);
	if (SomePlayer.GetScrolls() > 7)
	{
		SomePlayer.SetScrolls(7);
	}
	if (SomePlayer.GetScrolls())
	{
		strcpy(New,Scrolls);
	}
	else
	{
		strcpy(New,"No");
	}
	strcat(New," scroll");
	if (SomePlayer.GetScrolls() !=1) strcat(New,"s");
	strcat(New," found...");
	for (int q2=0; q2 < SomePlayer.GetScrolls(); q2++)
	{
		Image("scroll_1.jpg",New);
	}
	if (SomePlayer.GetScrolls() != 7)
	{
		for (int q = SomePlayer.GetScrolls(); q<7; q++)
		{
			Image("scroll_0.jpg",New);
		}
	}
}

void DispStamina(long f)
{
	if (f<1) return;
	char j[128];
	itoa(f,j,10);
	strcat(j," stamina points");
	while (f)
	{
		if (f >= 10)
		{
			f -= 10;
			Image("10_stamina.jpg",j);
			continue;
		}
		if (f >=1)
		{
			f--;
			Image("1_stamina.jpg",j);
		}
	}
}


void DispDays(Player SomePlayer)
{
	char Days[5], New[1024];
	itoa(SomePlayer.GetDays(),Days,10);
	if (SomePlayer.GetDays()>1)
	{
		strcpy(New,Days);
	}
	else
	{
		strcpy(New,"No");
	}
	strcat(New," day");
	if (SomePlayer.GetDays() != 1) strcat(New,"s");
	strcat(New," remaining...");
	long N = 0;
	if (SomePlayer.GetDays() == -666) return;
	if (SomePlayer.GetDays() >= 1)
	{
		N = SomePlayer.GetDays();
	}
	if (N>=0)
	{
		for (int q=0; q < N; q++)
		{
			Image("days_1.jpg",New);
		}
		if (N != 14)
		{
			for (int q2 = SomePlayer.GetDays(); q2<14; q2++)
			{
				Image("days_0.jpg",New);
			}
		}
	}
}



bool comparison(char* condition, long C1, char* C2, int g_dice)
{
	int C = 0;

	if (!strcmp(C2,"dice>"))
	{
		C = g_dice;
	}
	else
	{
		C = atoi(C2);
	}
	
	if (!strcmp(condition,"eq"))
	{
		if (C1 == C)
		{
			return true;
		}
	}

	if (!strcmp(condition,"gt"))
	{
		if (C1 > C)
		{
			return true;
		}
	}

	if (!strcmp(condition,"lt"))
	{
		if (C1 < C)
		{
			return true;
		}
	}

        if (!strcmp(condition,"ne"))
        {
		if (C1 != C)
		{
			return true;
		}
	}

	return false;
}

long CountLogins()
{
	char query[1024];
	int res;
	MYSQL_RES* myres;
	sprintf(query,"SELECT username FROM game_users WHERE username != '**NONE**' AND lastuse > (UNIX_TIMESTAMP() - 600) ORDER BY username");
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_store_result(&my_connection);
		long rows = mysql_affected_rows(&my_connection);
		mysql_free_result(myres);
		return rows;
	}
	return 0;
}

long CountUsers()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
        sprintf(query,"SELECT username FROM game_default_users ORDER BY username");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
                myres = mysql_store_result(&my_connection);
                long rows = mysql_affected_rows(&my_connection);
                mysql_free_result(myres);
                return rows;
        }
	return 0;
							
}

long DispLogins()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
	MYSQL_ROW a_row;
        sprintf(query,"SELECT username FROM game_users WHERE username != '**NONE**' AND lastuse > (UNIX_TIMESTAMP() - 600) ORDER BY lastuse LIMIT 0,10");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
		int q = 0;
                myres = mysql_use_result(&my_connection);
                while (a_row = mysql_fetch_row(myres))
		{
			if (q)
			{
				cout << ", ";
			}
			cout << "<b>" << a_row[0] << "</b>";
			q++;
		}
                mysql_free_result(myres);
                return 1;
        }
        return 0;
}


long CountDroppedItems()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
        sprintf(query,"SELECT * FROM game_dropped_items");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
                myres = mysql_store_result(&my_connection);
                long rows = mysql_affected_rows(&my_connection);
                mysql_free_result(myres);
                return rows;
        }
        return 0;
}


long CountPendingChatEvents()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
        sprintf(query,"SELECT * FROM game_chat_events");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
                myres = mysql_store_result(&my_connection);
                long rows = mysql_affected_rows(&my_connection);
                mysql_free_result(myres);
                return rows;
        }
        return 0;
}

long CountBankItems()
{
	char query[1024];
	int res;
	MYSQL_RES* myres;
	sprintf(query,"SELECT * FROM game_bank");
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_store_result(&my_connection);
		long rows = mysql_affected_rows(&my_connection);
		mysql_free_result(myres);
		return rows;
	}
	return 0;
}

long CountAdmins()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
        sprintf(query,"SELECT * FROM game_admins");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
                myres = mysql_store_result(&my_connection);
                long rows = mysql_affected_rows(&my_connection);
                mysql_free_result(myres);
                return rows;
        }
        return 0;
}

long GetNextForumID()
{
	char query[1024];
	int res;
	MYSQL_RES* myres;
	sprintf(query,"SELECT * FROM %smembers",IBF_PREFIX);
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_store_result(&my_connection);
		long rows = mysql_affected_rows(&my_connection);
		mysql_free_result(myres);
		return rows;
	}
	return 0;
}

long CountOwnedItems()
{
        char query[1024];
        int res;
        MYSQL_RES* myres;
        sprintf(query,"SELECT * FROM game_owned_items");
        res = mysql_query(&my_connection,query);
        if (!res)
        {
                myres = mysql_store_result(&my_connection);
                long rows = mysql_affected_rows(&my_connection);
                mysql_free_result(myres);
                return rows;
        }
        return 0;
}

bool GlobalSet(char* flag)
{
	MYSQL my_connection2;
	char query[1024];
	int res;
	MYSQL_RES* myres;
        // This requires a second connection otherwise we get "commands out of sync" :(
        mysql_init(&my_connection2);
        if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
	{
		sprintf(query,"SELECT flag FROM game_global_flags WHERE flag='%s'",flag);
		res = mysql_query(&my_connection2,query);
		if (!res)
		{
			myres = mysql_store_result(&my_connection2);
			long rows = mysql_affected_rows(&my_connection2);
			mysql_free_result(myres);
			mysql_close(&my_connection2);
			return (rows > 0);
		}
		mysql_close(&my_connection2);
	}
	return false;
}

void ListPossessions(Player P)
{
	char query[1024],flags[1024],name[1024],GUID[1024];
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	cout << "<center><form>";
	cout << "<input type='hidden' name='action' value='deposititem'>";
	cout << "<input type='hidden' name='guid' value='" << GUID << "'>";
	cout << "<select name='item' width='20'>";
	for (int i = 0; i <= 30; i++)
	{
		if (strcmp(P.Possessions.List[i].Name,"[none]"))
		{
			cout << "<option>" << P.Possessions.List[i].Name << "</option>";
		}
	}
	cout << "</select>" << CR;
	cout << "<input type='submit' value='Deposit'>";
	cout << "</form>";
	cout.flush();
}

long ConsolidateBankGold(Player &P)
{
	// combines all gold into one row, and returns total gold in the UtopiaBank for one player by GUID
	char query[1024],flags[1024],name[1024],GUID[1024];
	int res, n = 0;
	long GoldTotal = 0;
	MYSQL_RES* myres;
	MYSQL_ROW a_row;
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	sprintf(query,"select sum(item_desc) from game_bank where owner_id='%s' and item_flags='GOLD'",GUID);
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_use_result(&my_connection);
		while (a_row = mysql_fetch_row(myres))
		{
			if (a_row[0])
			{
				GoldTotal = atoi(a_row[0]);
			}
		}
		mysql_free_result(myres);
	}
	if (GoldTotal > 0)
	{
		sprintf(query,"delete from game_bank where owner_id='%s' and item_flags='GOLD'",GUID);
		res = mysql_query(&my_connection,query);
		sprintf(query,"insert into game_bank values('%s','%d','GOLD')",GUID,GoldTotal);
		res = mysql_query(&my_connection,query);
	}
	cout.flush();
	return GoldTotal;
}

void ListBankItems(Player &P)
{
	char query[1024],flags[1024],name[1024],GUID[1024];
	int res, n = 0;
	MYSQL_RES* myres;
	MYSQL_ROW a_row;
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	cout << "<center><form>";
	cout << "An item:";
	cout << "<input type='hidden' name='action' value='withdrawitem'>";
	cout << "<input type='hidden' name='guid' value='" << GUID << "'>";
	cout << "<select name='item' width='20'>";
	sprintf(query,"SELECT item_desc,item_flags FROM game_bank WHERE owner_id='%s' ORDER BY item_desc",GUID);
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_use_result(&my_connection);
		while (a_row = mysql_fetch_row(myres))
		{
			if (strcmp(a_row[1],"GOLD"))
			{
				cout << "<option>" << a_row[0] << "</option>";
				n++;
			}
		}
		mysql_free_result(myres);
	}
	if (!n)
	{
		cout << "<option selected>(You have no items in the bank)</option>" << CR;
	}
	cout << "</select> ";
	cout << "<input type='submit' value='Withdraw'>";
	cout << "</form>";
	long totalgold = ConsolidateBankGold(P);
	if (totalgold)
	{
		cout << CR << "<form>";
		cout << "<input type='hidden' name='action' value='withdrawgold'>";
		cout << "<input type='hidden' name='guid' value='" << GUID << "'>";
		cout << "Gold: <input type='text' name='amount' value='" << totalgold << "'> ";
		cout << "<input type='submit' value='Withdraw'>";
		cout << "</form>";
	}
	cout.flush();
}

bool DepositGoldInBank(Player &P, char* amount)
{
	char query[1024],flags[1024],GUID[1024];
	int res;
	MYSQL_RES* myres;
	if ((P.GetGold() < atoi(amount)) || (atoi(amount) < 10))
	{
		return false; // player doesnt have this much gold, or tried to deposit less than 10!
	}
	P.AddGold(-atoi(amount));
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	PutByGuid(GUID,P);
	sprintf(query,"INSERT INTO game_bank VALUES(\"%s\",\"%s\",\"GOLD\")",GUID,amount);
	res = mysql_query(&my_connection,query);
	cout.flush();
	return (res == 0);
}

bool WithdrawGoldFromBank(Player &P, char* amount)
{
	char query[1024],flags[1024],GUID[1024];
	int res;
	MYSQL_RES* myres;
	long totalgold = ConsolidateBankGold(P);
	if (atoi(amount) < 1)
	{
		return false; // cant withdraw a negative amount of gold!!!! (Yeti gets credit for this gem)
	}
	if (totalgold < atoi(amount))
	{
		return false; // not this much money in the bank, utopiabank doesnt have an overdraft facility!!!
	}
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	sprintf(query,"DELETE FROM game_bank WHERE owner_id='%s' AND item_flags='GOLD'",GUID);
	res = mysql_query(&my_connection,query);
	P.AddGold(atoi(amount));
	PutByGuid(GUID,P);
	sprintf(query,"INSERT INTO game_bank VALUES(\"%s\",\"%d\",\"GOLD\")",GUID,totalgold-atoi(amount));
	res = mysql_query(&my_connection,query);
	cout.flush();
	return (res == 0);
}

bool WithdrawItemFromBank(Player &P, char* iname)
{
	char query[1024],flags[1024],GUID[1024], iflags[1024];
	int res;
	bool found = false;
	MYSQL_RES* myres;
	MYSQL_ROW row;
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	sprintf(query,"SELECT item_desc,item_flags FROM game_bank WHERE owner_id='%s' AND item_desc=\"%s\" LIMIT 1",GUID,iname);
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		myres = mysql_use_result(&my_connection);
		while (row = mysql_fetch_row(myres))
		{
			P.Possessions.Add(row[0],row[1]);
			strcpy(iflags,row[1]);
			found = true;
		}
		mysql_free_result(myres);
	}
	if (found)
	{
		PutByGuid(GUID,P);
		// use LIMIT 1 to only delete one if there are multiple items, e.g. stamina restorers
		sprintf(query,"DELETE FROM game_bank WHERE owner_id='%s' AND item_desc=\"%s\" AND item_flags=\"%s\" LIMIT 1",GUID,iname,iflags);
		res = mysql_query(&my_connection,query);
		return (res == 0);
	}
	else
	{
		return false;
	}
}

bool DepositItemInBank(Player &P, char* iname)
{
	char query[1024],flags[1024],GUID[1024];
	int res;
	MYSQL_RES* myres;
	if (!P.Possessions.Get(iname,flags))
	{
		return false; // Item is not in backpack, probably cheating!
	}
	P.Possessions.Delete(iname);
	LpToGuid(GUID,P.GetUsername(),P.GetPassword());
	PutByGuid(GUID,P);
	sprintf(query,"INSERT INTO game_bank VALUES(\"%s\",\"%s\",\"%s\")",GUID,iname,flags);
	res = mysql_query(&my_connection,query);
	cout.flush();
	return (res == 0);
}

