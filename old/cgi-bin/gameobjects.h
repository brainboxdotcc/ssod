// Project: SSOD
// Content: Game-level object definitions


#ifndef __GAMEOBJECTS_H_DEFINED__
#define __GAMEOBJECTS_H_DEFINED__

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <fstream>

#include "mysql.h"
#include "mysql_config.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Function: Dice
// Returns a random number between 1 and 6.
//////////////////////////////////////////////////////////////////////////////////////////////////


int Dice()
{
	return (rand() % 6)+1;
}

int D12()
{
        return (rand() % 12)+1;
}

int twoD6()
{
        return Dice()+Dice();
}



void ImageLink(char* ImageName, char* Hint, char* InternalAddress)
{
	cout << "<a href=\"" << me << "?" << InternalAddress << "\">";
	cout << "<img src=\"" << IMAGE_DIR << "/" << ImageName << "\" alt=\"" << Hint << "\" border=0>";
	cout << "</a>";
}

void ExternalImageLink(char* ImageName, char* Hint, char* Address)
{
	cout << "<a href=\"" << Address << "\">";
	cout << "<img src=\"" << IMAGE_DIR << "/" << ImageName << "\" alt=\"" << Hint << "\" border=0>";
	cout << "</a>";
}

void Image(char* ImageName, char* Hint)
{
	cout << "<img src=\"" << IMAGE_DIR << "/" << ImageName << "\" alt=\"" << Hint << "\" border=0>";
}

void HyperLink(char* LinkText, char* InternalAddress)
{
	cout << "<a href=\"" << me << "?" << InternalAddress << "\">";
	cout << LinkText;
	cout << "</a>";
}

void HyperLinkExt(char* LinkText, char* InternalAddress)
{
        cout << "<a href=\"" << InternalAddress << "\">";
        cout << LinkText;
        cout << "</a>";
}



void HyperLinkNew(char* LinkText, char* InternalAddress)
{
       cout << "<a style=\"cursor:pointer\" onclick=\"open('" << InternalAddress << "','_blank','height=350,width=400,status=no,toolbar=no,menubar=no,location=no,scrollbars=yes');\">";
        cout << LinkText;
        cout << "</a>";
}

char rv[1024];

char* Describe(char* f, char* n)
{
	MYSQL_RES* a_res;
	MYSQL_ROW a_row;

	int res;
	char query[1024];

	sprintf(query,"SELECT idesc FROM game_item_descs WHERE name like \"%s\"",n);
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		a_res = mysql_use_result(&my_connection);
		while (a_row = mysql_fetch_row(a_res))
		{
			strcpy(rv,a_row[0]);
			mysql_free_result(a_res);
			return rv;
		}
		mysql_free_result(a_res);
	}
	
	if (!strncmp(f,"ST+",3))
	{
		char *moot = f+3;
		sprintf(rv,"Adds %s stamina when used.",moot);
		return rv;
	}
	if (!strncmp(f,"SK+",3))
	{
		char *moot = f+3;
		sprintf(rv,"Adds %s skill when used.",moot);
		return rv;
	}
	if (!strncmp(f,"LK+",3))
	{
		char *moot = f+3;sprintf(rv,"Adds %s luck when used.",moot);return rv;
	}
	if (!strncmp(f,"SN+",3))
	{
		char *moot = f+3;sprintf(rv,"Adds %s sneak when used.",moot);return rv;
	}
	if (!strncmp(f,"W+",2))
	{
		char *moot = f+2;sprintf(rv,"This is an item that adds %s to your weapon score",moot);return rv;
	}
	if (!strncmp(f,"A+",2))
	{
		char *moot = f+2;sprintf(rv,"This is an item that adds %s to your armour score",moot);return rv;
	}
	if (!strncmp(f,"W",1))
	{
		char *moot = f+1;sprintf(rv,"This is a weapon with a rating of %s",moot);return rv;
	}
	if (!strncmp(f,"A",1))
	{
		char *moot = f+1;sprintf(rv,"This is armour with a rating of %s",moot);return rv;
	}
        
	return n;
}

char foo[1024];

char* AddEscapes(char* text)
{
	int j = 0;
	for (unsigned int a=0; a <= strlen(text); a++)
	{
		if (text[a] == ' ')
		{
			foo[j++] = '+';
		}
		else
		{
			foo[j++] = text[a];
		}
	}
	return foo;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Function: InitGameEngine
// Does misc. startup stuff such as seeding the random number generator and setting globals
//////////////////////////////////////////////////////////////////////////////////////////////////


void InitGameEngine()
{
	srand((unsigned)time(NULL));
	strcpy(me,getenv("SCRIPT_NAME"));
	ParseQuery();
	if (!mysql_init_conn(my_connection))
	{
		cout << "Content-Type: text/html\n\n";
		cout << "<html>Error: Could not connect to database! - mysql_init_conn() failed.";
		cout << mysql_error(&my_connection) << "</html>";
		exit(0);
	}
}

void InitUtilEngine()
{
	srand((unsigned)time(NULL));
	if (!mysql_init_conn(my_connection))
	{
	       cout << "Error: Could not connect to database! - mysql_init_conn() failed.\n";
	       exit(0);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// GameObject
// All game classes inherit the GameObject class. All it does is manage a unique ID for each
// game item, examples being adventurers, monsters, possessions, etc.
//////////////////////////////////////////////////////////////////////////////////////////////////



class GameObject
{

protected:

	long ID;

public:

	GameObject() { ID = 0; }
	~GameObject() { /* stub */ }

	long GetID() { return ID; }

	void SetID(long _ID) { ID = _ID; }

};

class Stats : public GameObject
{
protected:
	
	MYSQL_RES* a_res;
	MYSQL_ROW a_row;
	int res;
public:
	
	long PlayerDeaths;
	long LifeExpectancy;
	long MaxActivePlayers;
	long AverageLoadTime;
	long NPCDeaths;
	long VisitsToday;
	long VisitsTotal;
	long NewPlayersToday;
	long AvgVisitsPerDay;
	long AvgNewPlayersPerDay;
	char NewestPlayerGuid[50];

	Stats()
	{
		char query[1024];
		sprintf(query,"SELECT player_deaths,life_expectancy,max_active_players,avg_page_load_time,npc_deaths,visits_today,visits_total,new_players_today,avg_visits_per_day,avg_new_players_per_day,newest_player_guid FROM game_stats WHERE id='0'");
		res = mysql_query(&my_connection,query);
		if (!res)
		{
			a_res = mysql_use_result(&my_connection);
			if (a_res)
			{
				a_row = mysql_fetch_row(a_res);
				if (a_row)
				{
					PlayerDeaths 		=	atoi(a_row[0]);
					LifeExpectancy 		=	atoi(a_row[1]);
					MaxActivePlayers	=	atoi(a_row[2]);
					AverageLoadTime		=	atoi(a_row[3]);
					NPCDeaths		=	atoi(a_row[4]);
					VisitsToday		=	atoi(a_row[5]);
					VisitsTotal		=	atoi(a_row[6]);
					NewPlayersToday		=	atoi(a_row[7]);
					AvgVisitsPerDay		=	atoi(a_row[8]);
					AvgNewPlayersPerDay	=	atoi(a_row[9]);
					
					strcpy(NewestPlayerGuid,a_row[10]);
				}
			}
			mysql_free_result(a_res);
		}
	}

	
	
	~Stats()
	{
	}

	void Save()
	{
		char query[1024];
		sprintf(query,"UPDATE game_stats SET player_deaths=%d, life_expectancy=%d, max_active_players=%d, avg_page_load_time=%d, npc_deaths=%d, visits_today=%d, visits_total=%d, new_players_today=%d, avg_visits_per_day=%d, avg_new_players_per_day=%d, newest_player_guid='%s'",PlayerDeaths,LifeExpectancy,MaxActivePlayers,AverageLoadTime,NPCDeaths,VisitsToday,VisitsTotal,NewPlayersToday,AvgVisitsPerDay,AvgNewPlayersPerDay,NewestPlayerGuid);
		res = mysql_query(&my_connection,query);
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// RatedItem
// A game object with a name and a rating, e.g. armour or a weapon.
//////////////////////////////////////////////////////////////////////////////////////////////////


class RatedItem : public GameObject
{
public:
	char Name[256];
	long Rating;

	RatedItem() { Rating = 0; Name[0]='\0'; }

	void SetName(char* _Name) { strcpy(Name, _Name); }
	void SetRating(long _Rating) { Rating = _Rating; }
	void GetName(char* _Name) { strcpy(_Name, Name); }
	long GetRating() { return Rating; }
};



char* Lower(char* A)
{
	for (unsigned int q=0; q <= strlen(A); q++)
	{
		A[q] = tolower(A[q]);
	}
	return A;
}

void SafeWrite(ofstream &file, char* Line) // safe file write hat won't write an invalid string (null or starting with space)
{
	if (Line)
	{
		// fixed ** Wont write a null string to the file
		if ((Line[0] != ' ') && (Line[0] != 0))
		{

			// remove trailing spaces from the line -- these are dangerous
			int t;
			t = strlen(Line)-1;
			while ((t != 0) && (Line[t] == ' '))
			{
				Line[t] = 0;
				t--;
			}

			file << Line << endl;
		}
		else
		{
			file << "[none]" << endl;
		}
	}
	else
	{
		file << "[none]" << endl;
	}
		
}


//////////////////////////////////////////////////////////////////////////////////////////////////



class ItemRecord : public GameObject
{
public:
        char Name[90];
        char FlagInfo[30];

        ItemRecord()
        {
        }
        ~ItemRecord()
        {
        }

};

class ItemList : public GameObject
{
public:

	ItemRecord List[32];

	~ItemList()
	{
		// Stub
	}

	// Constructor

	ItemList()
	{

		for (int i = 0; i <= 30; i++) // Constructor clears the object ready for use
		{
			strcpy(List[i].Name,"[none]");
			strcpy(List[i].FlagInfo,"[none]");
		}
	}

	///////////////////////////////////////////////

	void Clear()
	{

		for (int i = 0; i <= 30; i++)
		{
			strcpy(List[i].Name,"[none]");
			strcpy(List[i].FlagInfo,"[none]");
		}
	}

	// ItemList::Add
	// Adds an item and its flags to the posession list
	// returns false if the list was full.

	bool Add(char* Item, char* Flags)
	{
		if (!strcmp(Item,""))
		{
			return false;
		}
		if (!strcmp(Flags,""))
		{
			return false;
		}
		for (int i = 0; i <= 30; i++)
		{
			if (!strcmp(List[i].Name,"[none]"))
			{
				strcpy(List[i].Name, Item);
				strcpy(List[i].FlagInfo, Flags);
				SortList();
				return true;
			}
		}
		
		return false;
	}

	// ItemList::Count
	// Returns the number of items in the list (0-30)

	int Count()
	{
		int total = 0;
		for (int i = 0; i <= 30; i++)
		{
			if (strcmp(List[i].Name,"[none]"))
			{
				total++;
			}
		}
		
		return total;
	}

	bool RandomItem(ItemRecord &IR)
	{
		for (int i = 30; i >= 0; i--)
		{
			if (strcmp(List[i].Name,"[none]"))
			{
				strcpy(IR.Name,List[i].Name);
				strcpy(IR.FlagInfo,List[i].FlagInfo);
				strcpy(List[i].FlagInfo,"[none]");
				strcpy(List[i].Name,"[none]");
				return true;
			}
		}
		return false;
	}


	// ItemList::Delete
	// Removes an item from the list.
	// Returns false if the item wasn't in the list to begin with.

	bool Delete(char* Item)
	{
		for (int i = 0; i <= 30; i++)
		{
			if (strcmp(List[i].Name,Item) == 0)
			{
				strcpy(List[i].Name,"[none]");
				strcpy(List[i].FlagInfo,"[none]");
				SortList();
				return true;
			}
		}
		
		return false;
	}

	// ItemList::Get
	// Returns true if an item exists in the list, and sets
	// "Flags" to the flags of the object the caller asks for.

	bool Get(char* Item, char* Flags)
	{
		for (int i = 0; i <= 30; i++)
		{
			if (strcasecmp(List[i].Name,Item) == 0)
			{
				strcpy(Flags,List[i].FlagInfo);
				return true;
			}
		}
		
		return false;
	}

	bool GetItem(int index,char* Item, char* Flags)
	{
		strcpy(Flags,List[index].FlagInfo);
		strcpy(Item,List[index].Name);
		return true;
	}

	bool Dump()
	{
		for (int i = 0; i <= 30; i++)
		{
			if (strcmp(List[i].Name,"[none]"))
			{
				cout << "<b>" << List[i].Name << "</b>" << " (Flags: " << List[i].FlagInfo << ")" << CR;
			}
		}
		
		return true;
	}


	bool Display(char* Para, char* guid, long S, char* CurrentWpn, char* CurrentArm)
	{
		SortList();
		char uri[1024];
		int e = 0, a = 0;
		if (S < 1)
		{
			return false;
		}
		cout << "<table border='1'>";
		for (int i = 0; i <= 30; i++)
		{
			cout << "<tr><td width=\"74%\" style='border-style:solid'>";
			if (!strcmp(List[i].Name,""))
			{
				strcpy(List[i].Name,"[none]");
				strcpy(List[i].FlagInfo,"[none]");
			}
			if (!strcmp(List[i].FlagInfo,""))
			{
				strcpy(List[i].Name,"[none]");
				strcpy(List[i].FlagInfo,"[none]");
			}
			if (strcmp(List[i].Name,"[none]"))
			{
				cout << List[i].Name;
				cout << "</td><td style='border-style:solid' width='8%'>";
				if (1)
				{
					if (((List[i].FlagInfo[0] == 'W') || (List[i].FlagInfo[0] == 'A')) && ((List[i].FlagInfo[1] != ' ') && (List[i].FlagInfo[1] != '+')))
					{
						// weapons and armour, but NOT weapon MODIFIERS and armour MODIFIERS have
						// "equip icons".
						char x[1024];
						strcpy(x,List[i].Name);
						sprintf(uri,"action=equip&guid=%s&item=%s&p=%s",guid,AddEscapes(List[i].Name),Para);
						ImageLink("equip.gif",Describe(List[i].FlagInfo,x),uri);
					}
					else
					{
						char x[1024];
						strcpy(x,List[i].Name);
						sprintf(uri,"action=use&guid=%s&item=%s&p=%s",guid,AddEscapes(List[i].Name),Para);
						ImageLink("use.gif",Describe(List[i].FlagInfo,x),uri);
					}
					cout << "</td><td style='border-style:solid' width='8%'>";
	
					// however, all items can be DROPPED.
					sprintf(uri,"action=dump&guid=%s&item=%s&p=%s",guid,AddEscapes(List[i].Name),Para);
					ImageLink("drop.gif","Drop item",uri);
					cout << "</td><td width='8%'>";
				}

				if (!strcmp(List[i].Name,CurrentArm))
	                        {
					if (!e)
					{
	                                	cout << "<img src='" << IMAGE_DIR << "/equipped.gif' border='0' alt='Item is equipped'>";
						e++;
					}
	                        }
				if (!strcmp(List[i].Name,CurrentWpn))
				{
					if (!a)
					{
						cout << "<img src='" << IMAGE_DIR << "/equipped.gif' border='0' alt='Item is equipped'>";
						a++;
					}
				}
				cout << "</td></tr>";
			}
		}
		cout << "</table>";
		return true;
	}
	

	void SortList() // Case insensitive bubble sort
	{
		bool sorted = false;

		while (!sorted)
		{

			sorted = true;

			for (int n=0; n <= (this->Count()-2); n++)
			{
				char A[1024], B[1024];
				strcpy(A,List[n].Name);
				strcpy(B,List[n+1].Name);
				if (strcmp(Lower(A),Lower(B)) > 0)
				{
					ItemRecord temp = List[n];
					List[n] = List[n+1];
					List[n+1] = temp;
					sorted = false;
				}
			}
		}
	}




};


//////////////////////////////////////////////////////////////////////////////////////////////////

void Readline(ifstream &in, char* buffer)
{
	if (!in.eof())
	{
		in >> buffer;
		if (!strcmp(buffer,""))
		{
			strcpy(buffer,"[none]");
			return;
		}
		char foo[1024];
		while (in.peek() != '\n')
		{
			in >> foo;
			strcat(buffer," ");
			strcat(buffer,foo);
		}
	}
	else
	{
		strcpy(buffer,"");
	}
}




class Player : public GameObject
{

protected:

	// In-game information about the character...
	
	PlayerRace			Race;
	PlayerProfession	Profession;
	PlayerProfession	X;
	long				Stamina;
	long				Skill;
	long				Luck;
	long				Sneak;
	long				Speed;
	long				Silver;
	long				Gold;
	long				Rations;
	long				Experience;
	long				Notoriety;
	long				Days;
	long				Scrolls;
	char				Paragraph[1024];
	RatedItem			Armour;
	RatedItem			Weapon;

	// Real-world information about the user playing the game


public:

	// These need to be public so the engine can get at their methods
	ItemList			Possessions;
	ItemList			Notes;
	ItemList			Spells;
	ItemList			Herbs;
	char				Username[50];
	char				Password[50];
	char				GotFrom[32768];
	time_t				LastUse;
	char				LastIP[128];
	time_t				LastStrike;
	time_t				Pinned;
	time_t				Muted;
	char				Email[256];
	int				Validated;
	long				Mana;
	time_t				ManaTick;
	char				ImagePack[256];

//////////////////////////////////////////////////////////////////////////////////////////////

	~Player()
	{
		// Stub
	}

	Player()
	{
		// The constructor for Player creates an adventurer ready for the game -
		// only spells and herbs need to be selected by the player.
		strcpy(Email,"none@null");
		Pinned = 0;
		Muted = 0;

		Skill = Dice() + 5;
		Stamina = Dice() + 5;
		Speed = Dice() + 5;

		Weapon.SetName("Hunting Dagger");
		Weapon.SetRating(1);

		Armour.SetName("Leather Coat");
		Armour.SetRating(1);

		Experience = 0;

		Sneak = Dice();

		int D = Dice();
		while (D == 6)
		{
			D = Dice();
		}

		switch (D)
		{

			case 1:
				Race = Human;
			break;

			case 2:
				Race = Dwarf;
			break;

			case 3:
				Race = Orc;
			break;

			case 4:
				Race = LesserOrc;
			break;

			case 5:
				Race = Elf;
			break;

		}

		Rations = Dice() + 4;

		Luck = Dice() + Dice() + 4;

		Gold = 10;
		Silver = 0;
		Days = 14;
		Scrolls = 0;
		Notoriety = 10;

		Possessions.Add("Hunting Dagger","W1");
		Possessions.Add("Leather Coat","A1");
		
		strcpy(Password,"**NONE**");
		strcpy(Username,"**NONE**");

		// Because ppl complained the game was too hard (lamers)
		Possessions.Add("Stamina Potion","ST+4");
		Possessions.Add("Skill Potion","SK+4");

		switch (Race)
		{
		case Human:
			strcpy(Paragraph,"1306");
		break;
		case Orc:
			strcpy(Paragraph,"1306");
		break;
		case LesserOrc:
			strcpy(Paragraph,"1325");
		break;
		case Elf:
			strcpy(Paragraph,"1325");
		break;
		case Dwarf:
			strcpy(Paragraph,"1332");
		break;
		}

		strcpy(GotFrom,"__ITEMS_FROM__");
	
	}

	bool GetFromDB(bool getbackup, char* FN)
	{
                for (int t = 0; t < strlen(FN); t++)
                {
                        if (strchr("\"\'<>&%+=,[]{}\\|@:;#~_!$^*()",FN[t]))
                        {
                                cout << "Content-Type: text/plain\r\n\r\nInvalid player GUID! Possible exploit attempt logged." << flush;
                                exit(0);
                        }
                }

                int res;
                MYSQL_RES *a_res;
                MYSQL_ROW a_row;
                char query[1024];

		Possessions.Clear();
		Herbs.Clear();
		Spells.Clear();

		if (getbackup)
		{
	                sprintf(query,"SELECT race,profession,x,stamina,skill,luck,sneak,speed,silver,gold,rations,experience,notoriety,days,scrolls,paragraph,armour,weapon,notes,username,password,gotfrom,armour_rating,weapon_rating,lastuse,lastip,laststrike,pinned,muted,email,validated,mana,manatick,imagepack FROM game_default_users WHERE guid='%s'",FN);
		}
		else
		{
			sprintf(query,"SELECT race,profession,x,stamina,skill,luck,sneak,speed,silver,gold,rations,experience,notoriety,days,scrolls,paragraph,armour,weapon,notes,username,password,gotfrom,armour_rating,weapon_rating,lastuse,lastip,laststrike,pinned,muted,email,validated,mana,manatick,imagepack FROM game_users WHERE guid='%s'",FN);
		}

		{
                      	res = mysql_query(&my_connection,query);
                      	if (!res)
                        {
                        	a_res = mysql_use_result(&my_connection);
                                if (a_res)
                                {
                                	a_row = mysql_fetch_row(a_res);
                                        if (a_row)
                                        {
				                switch (atoi(a_row[0]))
				                {
				                        case 1:
				                                this->Race = Human;
				                        break;
				
				                        case 2:
				                                this->Race = Elf;
				                        break;

				                        case 3:
				                                this->Race = Orc;
				                        break;

				                        case 4:
				                                this->Race = Dwarf;
				                        break;

				                        case 5:
				                                this->Race = LesserOrc;
				                        break;

				                        case 6:
				                                this->Race = Barbarian;
				                        break;

				                        case 7:
				                                this->Race = Goblin;
				                        break;

				                        case 8:
				                                this->Race = DarkElf;
				                        break;

				                }
	
						switch (atoi(a_row[1]))
						{
				                        case 1:
				                                this->Profession = Warrior;
				                        break;
				
				                        case 2:
				                                this->Profession = Wizard;
				                        break;
	
				                        case 3:
				                                this->Profession = Thief;
				                        break;

				                        case 4:
				                                this->Profession = Woodsman;
				                        break;

				                        case 5:
				                                this->Profession = Assassin;
				                        break;

				                        case 6:
				                                this->Profession = Mercenary;
			       				break;
				                }

						X = Profession;
						Stamina = atoi(a_row[3]);
						Skill = atoi(a_row[4]);
						Luck = atoi(a_row[5]);
						Sneak = atoi(a_row[6]);
						Speed = atoi(a_row[7]);
						Silver = atoi(a_row[8]);
						Gold = atoi(a_row[9]);
						Rations = atoi(a_row[10]);
						Experience = atoi(a_row[11]);
						Notoriety = atoi(a_row[12]);
						Days = atoi(a_row[13]);
						Scrolls = atoi(a_row[14]);
						strcpy(Paragraph,a_row[15]);
						strcpy(Armour.Name,a_row[16]);
						strcpy(Weapon.Name,a_row[17]);
						// todo: NOTES
						strcpy(Username,a_row[19]);
						strcpy(Password,a_row[20]);
						strcpy(GotFrom,a_row[21]);
						Armour.Rating = atoi(a_row[22]);
						Weapon.Rating = atoi(a_row[23]);
						LastUse = atoi(a_row[24]);
						if (a_row[25])
						{
							if (strcmp(a_row[25],""))
							{
								strcpy(LastIP,a_row[25]);
							}
						}
						LastStrike = atoi(a_row[26]);
						Pinned = atoi(a_row[27]);
						Muted = atoi(a_row[28]);
						strcpy(Email,a_row[29]);
						Validated = atoi(a_row[30]);
						Mana = atoi(a_row[31]);
						ManaTick = atoi(a_row[32]);
						strcpy(ImagePack,a_row[33]);
                          		}
                        }
			mysql_free_result(a_res);
		}
		else
		{
               		cout << "SELECT error: " << mysql_error(&my_connection);
			cout << CR << "Your character does not exist! Maybe it was deleted?";
                }

		// clear the list so no junk creeps in!
		Possessions.Clear();
		Herbs.Clear();
		Spells.Clear();
		
		sprintf(query,"SELECT item_desc,item_flags FROM game_owned_items WHERE owner_guid='%s'",FN);
		res = mysql_query(&my_connection,query);
                if (!res)
		{
			a_res = mysql_use_result(&my_connection);
			if (a_res)
			{
				while (a_row = mysql_fetch_row(a_res))
				{
					if (!strcmp(a_row[1],"SPELL"))
					{
						// insert a spell
						if (strcmp(a_row[0],""))
						{
							Spells.Add(a_row[0],a_row[1]);
						}
					}
					else if (!strcmp(a_row[1],"HERB"))
					{
						if (strcmp(a_row[0],""))
						{
							// insert a herb
							Herbs.Add(a_row[0],a_row[1]);
						}
					}
					else
					{
						if (strcmp(a_row[0],""))
						{
							// insert possession
							Possessions.Add(a_row[0],a_row[1]);
						}
					}
				}
			}
		}
		else
		{
                        cout << "SELECT error: " << mysql_error(&my_connection);
                        cout << CR << "Your character's backpack does not exist! Do we have an SQL thief in our midst?";
		}
																																							
	}
	return true;



	}

	bool Get(bool getbackup,char* FN) // Gets a player from current file position
	{
		// Depreciated call, calls GetFromDB
		return this->GetFromDB(getbackup,FN);
	}

	bool PutToDB(bool putbackup, char* FN)
	{
        	for (int t = 0; t < strlen(FN); t++)
        	{
                	if (strchr("\"\'<>&%+=,[]{}\\|@:;#~_!$^*()",FN[t]))
                	{
        	                cout << "Content-Type: text/plain\r\n\r\nInvalid player GUID! Possible exploit attempt logged." << flush;
			        exit(0);
	                }
	        }

		MYSQL_ROW a_row;
		int res;
		MYSQL_RES *a_res;
		char sql[65536];
		char foom[1024];

        	{
	                int i;

			// clear out old item list
			sprintf(sql,"DELETE FROM game_owned_items WHERE owner_guid='%s'",FN);
			res = mysql_query(&my_connection,sql);
			
	                for (i = 0; i <= 29; i++)
	                {
				if (!strcmp(Possessions.List[i].Name,""))
				{
					strcpy(Possessions.List[i].Name,"[none]");
					strcpy(Possessions.List[i].FlagInfo,"[none]");
																						                        }
				if (!strcmp(Possessions.List[i].FlagInfo,""))
				{
					strcpy(Possessions.List[i].Name,"[none]");
					strcpy(Possessions.List[i].FlagInfo,"[none]");
				}
				
				if (strcmp(Possessions.List[i].Name,"[none]"))
				{
		                        sprintf(sql,"INSERT INTO game_owned_items VALUES(\"%s\",\"%s\",\"%s\")",FN,Possessions.List[i].Name,Possessions.List[i].FlagInfo);
		                        res = mysql_query(&my_connection,sql);
				}
				if (strcmp(Herbs.List[i].Name,"[none]"))
				{
		                        sprintf(sql,"INSERT INTO game_owned_items VALUES(\"%s\",\"%s\",\"%s\")",FN,Herbs.List[i].Name,Herbs.List[i].FlagInfo);
		                        res = mysql_query(&my_connection,sql);
				}
				if (strcmp(Spells.List[i].Name,"[none]"))
				{
		                        sprintf(sql,"INSERT INTO game_owned_items VALUES(\"%s\",\"%s\",\"%s\")",FN,Spells.List[i].Name,Spells.List[i].FlagInfo);
		                        res = mysql_query(&my_connection,sql);
				}
	                }

			// delete old - this is slower but ensures update of items that dont exist yet
			if (putbackup)
			{
				sprintf(sql,"DELETE FROM game_default_users WHERE guid='%s'",FN);
			}
			else
			{
				sprintf(sql,"DELETE FROM game_users WHERE guid='%s'",FN);
			}

			LastUse = time(NULL);
			res = mysql_query(&my_connection,sql);


			char WD[1024],AD[1024];
			long WR,AR;
			
			GetWeapon(WD,WR);
			GetArmour(AD,AR);

			if (getenv("REMOTE_ADDR"))
			{
				strcpy(LastIP,getenv("REMOTE_ADDR"));
			}
			
                        //                                          AV   RA   PR   BL   ST   SK  LU    SN   SPD SLV  GLD  RAT  EXP  NOT  DAY  SCR  PAR  0     0     0   USR  PWD          FLG
			if (putbackup)
			{
				sprintf(sql,"INSERT INTO game_default_users VALUES(\"%s\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\")",
				FN,GetRace(),GetProfession(),
				"0",GetStamina(),GetSkill(),GetLuck(),GetSneak(),GetSpeed(),GetSilver(),
				GetGold(),GetRations(),GetExperience(),GetNotoriety(),GetDays(),GetScrolls(),
				Paragraph,AD,WD,"0",
				GetUsername(),
				GetPassword(),
				GetFlags(),
				AR,WR,LastUse,LastIP,LastStrike,Pinned,Muted,Email,Validated,Mana,ManaTick,ImagePack);
			}
			else
			{
                               sprintf(sql,"INSERT INTO game_users VALUES(\"%s\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%s\")",
                               FN,GetRace(),GetProfession(),
                               "0",GetStamina(),GetSkill(),GetLuck(),GetSneak(),GetSpeed(),GetSilver(),
                               GetGold(),GetRations(),GetExperience(),GetNotoriety(),GetDays(),GetScrolls(),
                               Paragraph,AD,WD,"0",
                               GetUsername(),
                               GetPassword(),
                               GetFlags(),
			       AR,WR,LastUse,LastIP,LastStrike,Pinned,Muted,Email,Validated,Mana,ManaTick,ImagePack);
			}
			res = mysql_query(&my_connection,sql);
        	}
		return true;

	}



        bool GetWithDebug(char* FN) // Gets a player from current file position
        {
                return true;
        }


	void Put(bool putbackup, char* FN) // puts a player to the current file position
	{
		// Depreciated call, calls PutToDB
		PutToDB(putbackup, FN);
	}

	void SetCredentials(char* username, char* password)
	{
		strcpy(Username,username);
		strcpy(Password,password);
		switch (Race)
		{
		case Human:
			strcpy(Paragraph,"1306");
		break;
		case Orc:
			strcpy(Paragraph,"1306");
		break;
		case LesserOrc:
			strcpy(Paragraph,"1325");
		break;
		case Elf:
			strcpy(Paragraph,"1325");
		break;
		case Dwarf:
			strcpy(Paragraph,"1332");
		break;
		}
	}

	void Strike()
	{
		// store this time as the time a hit was last made in combat :)
		LastStrike = time(NULL);
	}

	void SetParagraph(char* p)
	{
		strcpy(Paragraph,p);
	}

	void GetParagraph(char* p)
	{
		strcpy(p,Paragraph);
	}

	void ResetToSpawnPoint()
	{
                switch (Race)
                {
                case Human:
                        strcpy(Paragraph,"1306");
                break;
                case Orc:
                        strcpy(Paragraph,"1306");
                break;
                case LesserOrc:
                        strcpy(Paragraph,"1325");
                break;
                case Elf:
                        strcpy(Paragraph,"1325");
                break;
                case Dwarf:
                        strcpy(Paragraph,"1332");
                break;
                }

	}


	char* GetUsername()
	{
		return Username;
	}

	char* GetPassword()
	{
		return Password;
	}

	char* GetFlags()
	{
		return GotFrom;
	}

	
	bool SneakTest(long MonsterSneak)
	{
		if ((Sneak+Dice()) > (MonsterSneak+Dice()))
		{
			return true;
		}
		return false;
	}


	bool Auth(char* username, char* password)
	{
                MYSQL_ROW a_row;
                int res;
                MYSQL_RES *a_res;
                char sql[65536];
		bool found = false;

		if (strcmp(username,Username))
		{
			return false;
		}
                
                int i;
                sprintf(sql,"SELECT username FROM game_users WHERE username='%s' AND password='%s'",username,password);
                res = mysql_query(&my_connection,sql);
                if (!res)
                {
                        a_res = mysql_use_result(&my_connection);
                        if (a_res)
                        {
                                 a_row = mysql_fetch_row(a_res);
                                 if (a_row)
                                 {
                                          found = true;
				 }
			}
			mysql_free_result(a_res);
			
		}
		return found;
		
	}
	
	PlayerRace GetRace()
	{
		return this->Race;
	}

	PlayerProfession GetProfession()
	{
		return this->Profession;
	}

	void SetProfession(PlayerProfession P)
	{
		this->Profession=P;
	}

	void SetRace(PlayerRace R)
	{
		this->Race=R;
	}

	// Stamina

	void SetStamina(long S)
	{
		Stamina = S;
	}

	long GetStamina()
	{
		return Stamina;
	}

	void AddStamina(long S) // Pass -ve numbers to subtract (!)
	{
		if (Stamina < 1)
			return;
		// cant add stamina to a dead person (stops resurrect bugs)
		Stamina+=S;
	}

	void AddExperience(long E) // Pass -ve numbers to subtract (!)
	{
		Experience+=E;
	}

	/////////// HELPER FUNCTIONS ///////////

	bool IsDead()
	{
		return (Stamina <= 0);
	}

	bool TimeUp()
	{
		return (Days <= 0);
	}

	///////////////////////////////////////

	void SetSkill(long S)
	{
		Skill = S;
	}

	long GetSkill()
	{
		return Skill;
	}

	void AddSkill(long S) // Pass -ve numbers to subtract (!)
	{
		Skill+=S;
	}

	// Luck

	void SetLuck(long L)
	{
		Luck = L;
	}

	long GetLuck()
	{
		return Luck;
	}

	bool TestLuck()
	{
		long Roll = Dice() + Dice();
		bool Result = false;
		if (Roll <= Luck)
		{
			Result = true;
		}
		if ((Luck--) < 0)
		{
			Luck = 0;
		}
		return Result;
	}

	bool TestStamina()
	{
		long Roll = Dice() + Dice();
		bool Result = false;
		if (Roll <= Stamina)
		{
			Result = true;
		}
		return Result;
	}

	bool TestSkill()
	{
		long Roll = Dice() + Dice();
		bool Result = false;
		if (Roll <= Skill)
		{
			Result = true;
		}
		return Result;
	}

	bool TestSpeed()
	{
		long Roll = Dice() + Dice();
		bool Result = false;
		if (Roll <= Speed)
		{
			Result = true;
		}
		return Result;
	}

	bool TestExperience()
	{
		long Roll = Dice() + Dice();
		bool Result = false;
		if (Roll <= Experience)
		{
			Result = true;
		}
		return Result;
	}

	void AddLuck(long S) // Pass -ve numbers to subtract (!)
	{
		Luck+=S;
	}


	//sneak

	void SetSneak(long Sn)
	{
		Sneak = Sn;
	}

	long GetSneak()
	{
		return Sneak;
	}

	void AddSneak(long S) // Pass -ve numbers to subtract (!)
	{
		Sneak+=S;
	}

	//speed

	void SetSpeed(long Sp)
	{
		Speed = Sp;
	}

	long GetSpeed()
	{
		return Speed;
	}

	void AddSpeed(long S) // Pass -ve numbers to subtract (!)
	{
		Speed+=S;
	}

	//Silver;

	void SetSilver(long Si)
	{
		Silver = Si;
		if (Silver < 0)
			Silver = 0;
	}

	long GetSilver()
	{
		return Silver;
	}

	//Gold

	void SetGold(long Go)
	{
		Gold = Go;
		if (Gold < 0)
			Gold = 0;
	}

	long GetGold()
	{
		return Gold;
	}

	void AddGold(long Go) // Pass -ve numbers to subtract gold (!)
	{
		Gold+=Go;
		if (Gold < 0)
			Gold = 0;
	}

	void AddSilver(long Si) // Pass -ve numbers to subtract silver (!)
	{
		Silver+=Si;
		if (Silver < 0)
			Silver = 0;
	}

	//Rations;

	void SetRations(long Ra)
	{
		Rations = Ra;
	}

	long GetRations()
	{
		return Rations;
	}

	// Remove a ration point, or subtract 2 stamina if none left -
	// returns false if out of rations (so warnings can be displayed)

	bool EatRation()
	{
		if (Rations-- < 1)
		{
			Stamina -= 2;
			Rations = 0;
			return false;
		}
		return true;
	}

	void AddRations(long Ra) // Pass -ve numbers to subtract rations (!)
	{
		Rations += Ra;
	}

	//Experience;

	void SetExperience(long Ex)
	{
		Experience = Ex;
	}

	long GetExperience()
	{
		return Experience;
	}

	//Notoriety;

	void SetNotoriety(long No)
	{
		Notoriety = No;
	}

	long GetNotoriety()
	{
		return Notoriety;
	}

	//RatedItem Weapon;

	void SetWeapon(char* Name, long Rating)
	{
		Weapon.SetName(Name);
		Weapon.SetRating(Rating);
	}

	void GetWeapon(char* Name, long &Rating)
	{
		Weapon.GetName(Name);
		Rating = Weapon.GetRating();
	}


	//RatedItem Armour;

	void SetArmour(char* Name, long Rating)
	{
		Armour.SetName(Name);
		Armour.SetRating(Rating);
	}

	void GetArmour(char* Name, long &Rating)
	{
		Armour.GetName(Name);
		Rating = Armour.GetRating();
	}


	//Days;

	void SetDays(long Da)
	{
		Days = Da;
	}

	long GetDays()
	{
		return Days;
	}

	// Remove a day and return false if the time is now up

	bool RemoveDay()
	{
		if (Days-- < 1)
		{
			Days = 0;
			return false;
		}
		return true;
	}

	//Scrolls;

	void SetScrolls(long Sc)
	{
		Scrolls = Sc;
	}

	long GetScrolls()
	{
		return Scrolls;
	}

	void AddScroll()
	{
		Scrolls++;
	}


};

//////////////////////////////////////////////////////////////////////////////////////////////////


class EngineObject
{

protected:

	long ID;

public:

	EngineObject() { ID = 0; }
	~EngineObject() { /* stub */ }

	long GetID() { return ID; }

	void SetID(long _ID) { ID = _ID; }

};


void GetPerms(int &nomagic, int &nocombat, int &notheft, int &nochat, Player &P)
{
	int res;
	MYSQL_RES* a_res;
	MYSQL_ROW a_row;
					char LastLink[1024];
					P.GetParagraph(LastLink);
                                        sprintf(query,"SELECT magic_disabled,combat_disabled,theft_disabled,chat_disabled FROM game_locations WHERE id=%s",LastLink);

                                        {
                                                res = mysql_query(&my_connection,query);
                                                if (!res)
                                                {
                                                        a_res = mysql_use_result(&my_connection);
                                                        if (a_res)
                                                        {
                                                                a_row = mysql_fetch_row(a_res);
                                                                if (a_row)
                                                                {
                                                                        unsigned int field_count;
                                                                        field_count = 0;

                                                                        if(mysql_field_count(&my_connection) == 0)
                                                                        {
                                                                                cout << "ERROR: SELECT returned no data!";
                                                                                return;
                                                                        }
                                                                        nomagic = atoi(a_row[0]);
                                                                        nocombat = atoi(a_row[1]);
                                                                        notheft = atoi(a_row[2]);
                                                                        nochat = atoi(a_row[3]);
                                                                }
                                                        }
							mysql_free_result(a_res);
                                                }
                                                else
                                                {
                                                        cout << "SELECT error: " << mysql_error(&my_connection);
                                                }
                                        }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#endif // __GAMEOBJECTS_H_DEFINED__
