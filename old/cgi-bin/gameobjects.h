// Project: SSOD
// Content: Game-level object definitions

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
