//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
// The Seven Spells Of Destruction																//
// -------------------------------																//
//																								//
// At last, SSOD exists as it always should have - as a web-based game :o)						//
// Programmed By BRAiN (C. J. Edwards), project started May 2001.								//
//																								//
// Greets to Thor, Angel_F, The Shadow and Halo Unit											//
//																								//
// Recommended: Apache 1.3.9 or greater, 50mb hard disk space, ANSI C++ compiler				//
// (and pure luck if you REALLY don't want it to crash - sack/dual crew+shining)				//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
// Tested on:																					//
// ----------																					//
//																								//
// (1) Apache/1.3.9+win32, MSVC++ 6.0, 64mb, 333mhz, Microsoft Windows Millenium				//
// (2) Apache/1.3.9-7mdk, G++ pgcc-2.91.66, 64mb, 400mhz, Mandrake Linux 6.1 (Helios)			//
// (3) Apache/1.3.13, G++ pgcc-2.91.66, ???mb, 2x400mhz, SuSE Linux 7.2							//
// (4) Apache/1.3.14+ExtraNet, G++ 2.95.3, 64mb, 400mhz, Mandrake Linux 7.2 (Oddesey)			//
// (5) Apache/2.0.x, g++-2.96, 128mb, 200mhz, Redhat Linux 7.2									//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

// Project: SSOD
// Content: Main file

// VersionInfo:
// 
// 0.98a  - Migrated administration module out of this file and placed it into a .htaccess area
// 0.99a  - Added new character/profession types and modifiers.
// 0.99.1 - All new players now get given one stamina potion and one skill potion at the start.
// 0.99.2 - Moved location data (paragraphs) into a mysql database, added mysql code.
//


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sstream>
#include "admin.h"
#include "config.h"
#include "globals.h"
#include "cgi_utility.h"
#include "gameobjects.h"
#include "gameutility.h"
#include "html.h"

using namespace std;

char New[1024];		// used for redirection

int nomagic = 0, notheft = 0, nochat = 0, nocombat = 0; // global vars for whats allowed and what isnt for the location

void SayToRoom(char* GUID, char* Data)
{
                                        Player P;
                                        char moo[1024];
                                        char Para[1024];
                                        char outdata[1024];
                                        char od[2048];
                                        int res;

                                        GetPlayer(P,formData[1]);
					PutByGuid(formData[1],P);
                                        P.GetParagraph(Para);
                                        // post line of text in formData[2] to room...
                                        sprintf(moo,"DELETE FROM game_chat_events WHERE time < %d",time(NULL)-60);
                                        res = mysql_query(&my_connection,moo);
                                        if (res)
                                        {
                                                ContentType();
                                                cout << mysql_error(&my_connection);
                                                return;
                                        }

                                        mysql_real_escape_string(&my_connection,outdata, Data, strlen(Data));

                                        strcpy(od,"");
                                        if (strlen(outdata) > 256)
                                                outdata[255] = '\0';

                                        for(int i = 0; i < strlen(outdata); i++)
                                        {
                                                if (outdata[i] == '<')
                                                {
                                                        strcat(od,"&lt;");
                                                }
                                                else
                                                if (outdata[i] == '>')
                                                {
                                                        strcat(od,"&gt;");
                                                }
                                                else
                                                if (outdata[i] == '&')
                                                {
                                                        strcat(od,"&amp;");
                                                }
                                                else
                                                {
                                                        char a[2];
                                                        a[0] = outdata[i];
                                                        a[1] = '\0';
                                                        strcat(od,a);
                                                }
                                        }
                                        sprintf(moo,"INSERT INTO game_chat_events VALUES ('%s','%s','%s','%d')",GUID,Para,od,time(NULL));
                                        res = mysql_query(&my_connection,moo);
                                        if (res)
                                        {
                                                ContentType();
                                                cout << mysql_error(&my_connection);
                                                return;
                                        }

}

void InjectHTML(char* TargetGUID,char* data)
{
	char outdata[10240],moo[20480];
	int res;
	mysql_real_escape_string(&my_connection,outdata, data, strlen(data));
	sprintf(moo,"INSERT INTO game_chat_events VALUES ('0','%s','%s','%d')",TargetGUID,outdata,time(NULL));
	res = mysql_query(&my_connection,moo);
	return;
}

void SayToUser(char* GUID, char* TargetGUID, char* Data)
{
                                        Player P;
                                        char moo[1024];
                                        char outdata[1024];
                                        char od[2048];
                                        int res;

                                        GetPlayer(P,formData[1]);
                                        // post line of text in formData[2] to room...
                                        sprintf(moo,"DELETE FROM game_chat_events WHERE time < %d",time(NULL)-60);
                                        res = mysql_query(&my_connection,moo);
                                        if (res)
                                        {
                                                ContentType();
                                                cout << mysql_error(&my_connection);
                                                return;
                                        }

                                        mysql_real_escape_string(&my_connection,outdata, Data, strlen(Data));

                                        strcpy(od,"");
                                        if (strlen(outdata) > 256)
                                                outdata[255] = '\0';

                                        for(int i = 0; i < strlen(outdata); i++)
                                        {
                                                if (outdata[i] == '<')
                                                {
                                                        strcat(od,"&lt;");
                                                }
                                                else
                                                if (outdata[i] == '>')
                                                {
                                                        strcat(od,"&gt;");
                                                }
                                                else
                                                if (outdata[i] == '&')
                                                {
                                                        strcat(od,"&amp;");
                                                }
                                                else
                                                {
                                                        char a[2];
                                                        a[0] = outdata[i];
                                                        a[1] = '\0';
                                                        strcat(od,a);
                                                }
                                        }
                                        sprintf(moo,"INSERT INTO game_chat_events VALUES ('%s','%s','%s','%d')",GUID,TargetGUID,od,time(NULL));
                                        res = mysql_query(&my_connection,moo);
                                        if (res)
                                        {
                                                ContentType();
                                                cout << mysql_error(&my_connection);
                                                return;
                                        }

}



void SQLTable(char* query, char* title)
{
	int res;
	MYSQL_RES *a_res;
	MYSQL_ROW a_row;
																			
	res = mysql_query(&my_connection,query);
        if (!res)
        {
		cout << "<b><u><center>" << title << "</u></b>" << CR;
                cout << "<table width='80%' align='center'>";
                cout << "<tr><td width='60%'><b>Name</b></td><td><b>Score</b></td></tr>";
                a_res = mysql_use_result(&my_connection);
                if (a_res)
                {
			while (a_row = mysql_fetch_row(a_res))
                        {
                                cout << "<tr><td>" << a_row[0] << "</td><td>" << a_row[1] << "</td></tr>";
                        }
                }
                cout << "</table></center>" << CR;
		mysql_free_result(a_res);
        }
        else
        {
                cout << "SELECT error: " << mysql_error(&my_connection);
        }
}



int main()
{
	InitGameEngine();
	Player SomePlayer;

	if (numItems == 0)
	{
		// The CGI runs in a 100% frame for security reasons -
		// because it's in a frame, the actual content of the GET requests are concealed,
		// so that a cracker cannot obtain a password simply by noting down a URL on a machine
		// when a user logs in, similar to the old problems on hotmail.
		//ContentType();
		//cout << "<HTML><HEAD><TITLE>The Seven Spells Of Destruction</TITLE></HEAD>";
		
		//cout << "<FRAMESET>";
		//cout << "<FRAME src=\"" << me << "?action=title\">";
		//cout << "</FRAMESET>"; //<NOFRAMES>";
		
		//cout << "<HEAD><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=" << me <<"?action=title\"></HEAD><BODY>";
		//cout << "Your browser supports neither frames nor meta-refresh tags. Click ";
		//cout << "<a href=\"" << me << "?action=title\">";
		//cout << "here</a> to continue, but you may experience multiple problems in this browser.";
		//cout << "</body></NOFRAMES></HTML>";
		sprintf(New,"%s?action=title",me);
		RedirectTo(New);
		return 0;
	}
	else
	{
		if (numItems > 0)
		{
			if (!strcmp(formName[0],"action"))
			{
				if (!strcmp(formData[0],"create"))
				{
					Player Adventurer;
					int D = Dice();
					while (D >= 5)
					{
						D = Dice();
					}

					switch (D)
					{

						case 1:
							Adventurer.SetProfession(Wizard);
						break;
			
						case 2:
							Adventurer.SetProfession(Warrior);
						break;
			
						case 3:
							Adventurer.SetProfession(Thief);
						break;

						case 4:
							Adventurer.SetProfession(Woodsman);
						break;

					}

					char ThingName[90];
					long ThingRating;

					ContentType();
					HtmlStart();

					long Temporary1 = rand() * rand() * rand();
					long Temporary2 = rand() * rand() * rand();
					char Filename[256];
					char Filename2[256];
					itoa(Temporary1,Filename,16);
					itoa(Temporary2,Filename2,16);
					strcat(Filename,Filename2);

					AddPlayer(Adventurer,Filename);

					cout << "<b><u>New Character Created</b></u>" << CR << CR << CR <<CR;
					cout << "<table width='900' border=0 cellpadding=2 cellspacing=4><tr><td width='395' background='" << IMAGE_DIR << "/wizard.jpg' style='border:solid #800000 .25pt'>";
					cout << "<form name=\"charcreate\" action=\"" << me << "\" method=post>";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"updatemaster\">";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\""<< Filename << "\">";
					cout << "<pre>Race:       ";
					
					cout << " <select name=\"race\" size=1>";
					cout << "<option selected>Human</option>";
					cout << "<option>Orc</option>";
					cout << "<option>Dwarf</option>";
					cout << "<option>Elf</option>";
					cout << "<option>Barbarian</option>";
					cout << "<option>Goblin</option>";
					cout << "<option>Dark Elf</option>";
					cout << "<option>Lesser Orc</option></select>" << CR;

					cout << "Profession: ";

					cout << " <select name=\"profession\" size=1>";
					cout << "<option selected>Warrior</option>";
					cout << "<option>Wizard</option>";
					cout << "<option>Thief</option>";
					cout << "<option>Assassin</option>";
					cout << "<option>Mercenary</option>";
					cout << "<option>Woodsman</option></select>" << CR;
					
					cout << "</b>Stamina:     <b>" << Adventurer.GetStamina() << CR;
					cout << "</b>Skill:       <b>" << Adventurer.GetSkill() << CR;
					cout << "</b>Luck:        <b>" << Adventurer.GetLuck() << CR;
					cout << "</b>Sneak:       <b>" << Adventurer.GetSneak() << CR;
					cout << "</b>Speed:       <b>" << Adventurer.GetSpeed() << CR;
					cout << "</b>Gold:        <b>" << Adventurer.GetGold() << CR;
					cout << "</b>Silver:      <b>" << Adventurer.GetSilver() << CR;
					cout << "</b>Rations:     <b>" << Adventurer.GetRations() << CR;
					cout << "</b>Experience:  <b>" << Adventurer.GetExperience() << CR;
					cout << "</b>Notoriety:   <b>" << Adventurer.GetNotoriety() << CR;

					Adventurer.GetArmour(ThingName,ThingRating);

					cout << "</b>Armour:      <b>" << ThingName << " (Rating " << ThingRating << ")" << CR;

					Adventurer.GetWeapon(ThingName,ThingRating);

					cout << "</b>Weapon:      <b>" << ThingName << " (Rating " << ThingRating << ")" << CR << CR << endl << endl;

					cout << "<br><br></td><td width='40'>";

					cout << "</td><td style='border:solid #800000 .25pt'><center>";

					cout << "This character will be kept on the system for exactly <b>one week</b>. To keep this character, enter a <b>username and password</b> below. Once you do this, the character will remain on the system permenantly. Otherwise, click your <b>refresh</b> button on your browser and create a new profile. You may <b>save this page to disk</b> to register this profile at a later date." << CR << CR;

					cout << "<table align='center'>";

					cout << "<tr><td><center>Username:</td><td><center><INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"username\"></td></tr>";
					cout << "<tr><td><center>Password:</td><td><center><INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password\"></td></tr>";
					cout << "<tr><td><center>E-Mail:</td><td><center><INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"email\"></td></tr>" << CR;
					cout << "<tr><td colspan='2'><br>You must enter a <b>valid</b> email address - A validation email will be sent to it, to enable access to the game for your character.<br><br></td></tr>";

					cout << "</table>";

					cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Save\" ACTION=\"" << me << "\" method=post></center>";

					cout << "</FORM></td></tr></table>";

					cout << CR << CR << CR << "If you do not receive your authorisation email within a week, please send an email to the <b><u>Admin Contact</u></b> on the front page of this site, using the email you signed up to the game with. I will then be able to activate your account manually, if it appears to have bounced for accidental reasons.";

					log("Temporary character stats created, stored as %s",Filename);

					return 0;
				
				}

				if (!strcmp(formData[0],"award"))
				{
						GetPlayer(SomePlayer,formData[1]);
						strcpy(New,me);
                                                strcat(New,"?action=paragraph&guid=");
                                                strcat(New,formData[1]);
                                                strcat(New,"&p=");
                                                strcat(New,formData[2]);
                                                strcat(New,"&fragment=");
                                                strcat(New,formData[3]);
                                                RedirectTo(New);
                                                SomePlayer.AddExperience(1);
                                                PutByGuid(formData[1],SomePlayer);
                                                return 0;

				}
				if (!strcmp(formData[0],"topten"))
				{
						ContentType();
						HtmlStart();
					
						cout << "<b><u>Hall of fame</b></u>" << CR << CR;

						cout << "<table width='90%'><tr><td width='33%'>";

						SQLTable("select username,experience from game_users where username != '**NONE**' order by experience desc limit 0,10","Highest Experience");
						cout << "</td><td width='33%'>";
						SQLTable("select username,experience from game_users where username != '**NONE**' order by experience limit 0,10","Lowest Experience");
                                                cout << "</td><td width='33%'>";
                                                SQLTable("select username,stamina from game_users where username != '**NONE**' and stamina != -666 order by stamina limit 0,10","Weakest Players");
						cout << "</td></tr><tr><td>";
						SQLTable("select username,gold from game_users where username != '**NONE**' order by gold desc limit 0,10","Heaviest Purse");
						cout << "</td><td>";
						SQLTable("select username,scrolls from game_users where username != '**NONE**' order by scrolls desc limit 0,10","Most Scrolls");

                                                cout << "</td><td width='33%'>";
						SQLTable("select username,stamina from game_users where username != '**NONE**' order by stamina desc limit 0,10","Strongest Players");
						cout << "</td></tr><tr><td>";
						cout << "</table>";

						return 0;

				}
				if (!strcmp(formData[0],"drawstats"))
				{
					RenderImage();
					return 0;
				}
				if (!strcmp(formData[0],"deposititem"))
				{
					Player P;
					GetPlayer(P,formData[1]);
					if (DepositItemInBank(P,formData[2]))
					{
						char url[1024],Par[1024];
						P.GetParagraph(Par);
						sprintf(url,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Par);
						RedirectTo(url);
						return 0;
					}
					else
					{
						ContentType();
						HtmlStart();
						cout << "<b>Error</b>: Oooh errr! Looks like you tried to deposit an item in the bank which isnt actually in your possessions! Cheating is bad, mmmm-k? Bugger. Youre going to have to log back in again now :)";
						return 0;
					}
				}
				if (!strcmp(formData[0],"depositgold"))
				{
					Player P;
					char Par[1024], url[1024];
					GetPlayer(P,formData[1]);
					DepositGoldInBank(P,formData[2]);
					P.GetParagraph(Par);
					sprintf(url,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Par);
					RedirectTo(url);
					return 0;	
				}
				if (!strcmp(formData[0],"withdrawitem"))
				{
					Player P;
					GetPlayer(P,formData[1]);
					if (WithdrawItemFromBank(P,formData[2]))
					{
						char url[1024],Par[1024];
						P.GetParagraph(Par);
						sprintf(url,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Par);
						RedirectTo(url);
						return 0;
						
					}
					else
					{
						ContentType();
						HtmlStart();
						cout << "<b>Error</b>: Oooh errr! Looks like you tried to withdrawt an item from the bank which isnt actually in the bank! Cheating is bad, mmmm-k? Bugger. Youre going to have to log back in again now :)";
						return 0;
					}
				}
				if (!strcmp(formData[0],"withdrawgold"))
				{
					Player P;
					char Par[1024], url[1024];
					GetPlayer(P,formData[1]);
					WithdrawGoldFromBank(P,formData[2]);
					P.GetParagraph(Par);
					sprintf(url,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Par);
					RedirectTo(url);
					return 0;
				}
				
				if (!strcmp(formData[0],"updatemaster"))
				{
					Player NewPlayer;
					ContentType();
					HtmlStart();

					if ((formData[4][0] == '\0') || (formData[5][0] == '\0') || (formData[6][0] == '\0'))
					{
						cout << "You didn't specify a username, email, or you forgot to enter a password. Click the back button on your browser and try again." << CR;
						return 0;
					}

					if (!email_ok(formData[6]))
					{
						cout << "You specified an invalid email address! Click the back button on your browser and try again." << CR;
						return 0;
					}

					int res;
				        MYSQL_RES *a_res;
					MYSQL_ROW a_row;
					char query[1024];
					bool already_exists = false;
                                                                                                                             
					sprintf(query,"SELECT username FROM game_users WHERE username RLIKE '%s'",formData[4]);
				        res = mysql_query(&my_connection,query);
				        if (!res)
				        {
				                a_res = mysql_use_result(&my_connection);
				                if (a_res)
				                {
							if (mysql_fetch_row(a_res))
							{
								already_exists = true;
							}
				                }
				                mysql_free_result(a_res);
				        }
				        else
				        {
				                cout << "SELECT error: " << mysql_error(&my_connection);
				        }

					if (already_exists)
					{
                                                cout << "This username is already in use! Please click back and try again.";
                                                return 0;
					}

					GetPlayer(NewPlayer, formData[1]);

					strcpy(NewPlayer.Email,formData[6]);

					if (!strcmp(formData[2],"Human"))
					{
						NewPlayer.SetRace(Human);
					}

					if (!strcmp(formData[2],"Elf"))
					{
						NewPlayer.SetRace(Elf);
					}

					if (!strcmp(formData[2],"Orc"))
					{
						NewPlayer.SetRace(Orc);
					}

					if (!strcmp(formData[2],"Lesser Orc"))
					{
						NewPlayer.SetRace(LesserOrc);
					}

					if (!strcmp(formData[2],"Dwarf"))
					{
						NewPlayer.SetRace(Dwarf);
					}

					if (!strcmp(formData[2],"Barbarian"))
					{
						NewPlayer.SetRace(Barbarian);
					}

					if (!strcmp(formData[2],"Goblin"))
					{
						NewPlayer.SetRace(Goblin);
					}

					if (!strcmp(formData[2],"Dark Elf"))
					{
						NewPlayer.SetRace(DarkElf);
					}



					//////////////////////////////////

					if (!strcmp(formData[3],"Warrior"))
					{
						NewPlayer.SetProfession(Warrior);
					}

					if (!strcmp(formData[3],"Wizard"))
					{
						NewPlayer.SetProfession(Wizard);
					}

					if (!strcmp(formData[3],"Thief"))
					{
						NewPlayer.SetProfession(Thief);
					}

					if (!strcmp(formData[3],"Woodsman"))
					{
						NewPlayer.SetProfession(Woodsman);
					}

					if (!strcmp(formData[3],"Assassin"))
					{
						NewPlayer.SetProfession(Assassin);
					}

					if (!strcmp(formData[3],"Mercenary"))
					{
						NewPlayer.SetProfession(Mercenary);
					}



					//////////////////////////////////

					NewPlayer.SetCredentials(formData[4],formData[5]);
					NewPlayer.SetStamina(NewPlayer.GetStamina()+Bonuses_Numeric(1,NewPlayer.GetRace(),NewPlayer.GetProfession()));
					NewPlayer.SetSkill(NewPlayer.GetSkill()+Bonuses_Numeric(2,NewPlayer.GetRace(),NewPlayer.GetProfession()));
					NewPlayer.SetLuck(NewPlayer.GetLuck()+Bonuses_Numeric(3,NewPlayer.GetRace(),NewPlayer.GetProfession()));
					NewPlayer.SetSneak(NewPlayer.GetSneak()+Bonuses_Numeric(4,NewPlayer.GetRace(),NewPlayer.GetProfession()));
					NewPlayer.SetSpeed(NewPlayer.GetSpeed()+Bonuses_Numeric(5,NewPlayer.GetRace(),NewPlayer.GetProfession()));

                                        if (NewPlayer.GetProfession() == Warrior)
	                                                NewPlayer.SetStamina(NewPlayer.GetStamina()+5);
                                        if ((SomePlayer.GetRace() == Barbarian) || (SomePlayer.GetRace() == Orc))
	                                                NewPlayer.SetStamina(NewPlayer.GetStamina()+3);;
					

					char FileN[1024];
					Stats ServerStats;
					LpToGuid(FileN, formData[4], formData[5]);
					NewPlayer.Validated = 0;
					
					AddPlayer(NewPlayer,FileN);
					NewPlayer.PutToDB(true,FileN);

					cout << "The character was saved successfully under your username," << CR;
					cout << "<font size=+2>\"<b>" << formData[4] << "</b>\" as a <b>" << Race(NewPlayer.GetRace()) << " " << Profession(NewPlayer.GetProfession()) << "</b></font>" << CR;
					cout << "An email has been sent to the email address you provided (<b>" << NewPlayer.Email << "</b>) with an address you must visit to activate this character. Until you do this, you will not be able to play the game." << CR;
					cout << CR;
					cout << "When you activate your account, you will automatically get an account on our ";
					HyperLinkExt("forums","http://www.ssod.org/forums");
					cout << " with the same username and password.";
					
					log("New character created: %s, %s %s [%s]",formData[4],Race(NewPlayer.GetRace()),Profession(NewPlayer.GetProfession()),FileN);
					strcpy(ServerStats.NewestPlayerGuid,FileN);
					ServerStats.Save();
					ValidationMail(FileN,NewPlayer.Email);

					return 0;

				}

				if (!strcmp(formData[0],"activate"))
				{
					Player P;
					char query[2048];
					GetPlayer(P,formData[1]);
					if (P.Validated)
					{
						ContentType();
						HtmlStart();
						cout << "The character <b>" << P.GetUsername() << "</b> has already been activated before!" << CR;
						cout << "Maybe you clicked the link in your email by accident?" << CR;
						log("%s attempted to re-authorize an already authorized character!!!",P.GetUsername());
						return 0;
					}
					P.Validated = 1;
					PutByGuid(formData[1],P);
					P.PutToDB(true,formData[1]); // write backup too
					ContentType();
					HtmlStart();
					cout << "The character <b>" << P.GetUsername() << "</b> has been successfully activated." << CR;
					cout << "Click <a href=\"" << me << "?action=login\">here</a> to log in with this character," << CR;
					cout << "Or click <a href=\"" << me << "\">here</a> to return to the main menu." << CR << CR;
					cout << "Your username and password can also be used for access to our ";
					HyperLink("forums","/forums");
					cout << ".";
					log("%s Mail-Authorized character via %s",P.GetUsername(),P.Email);
					cout.flush();
					long nextid = GetNextForumID();
					sprintf(query,"INSERT INTO %smembers VALUES(%d,'%s',3,md5('%s'),'%s','%d','%s','noavatar','',0,'','','','','','','','',0,'','','1','','','','','','%d','1','1','1','1','1','','','','0','','','0','in:Inbox|sent:Sent Items','','','0','','','%d','%d','0','-1&-1','0','0','0')",IBF_PREFIX,nextid,P.GetUsername(),P.GetPassword(),P.Email,time(NULL),P.LastIP,time(NULL),time(NULL),time(NULL));
					if (mysql_query(&my_connection,query))
					{
						cout << CR << "Error adding forum account: " << mysql_error(&my_connection);
					}
					sprintf(query,"UPDATE %sstats SET LAST_MEM_NAME='%s', LAST_MEM_ID='%d', MEM_COUNT='%d'",IBF_PREFIX,P.GetUsername(),nextid,nextid);
					mysql_query(&my_connection,query);
					cout.flush();
					return 0;
				}

				if (!strcmp(formData[0],"credits"))
				{
					Credits();
					return 0;
				}

				if (!strcmp(formData[0],"combat"))
				{
					ContentType();
					HtmlStart();
					StopCheat();

					const int OFFENSIVE = 1, DEFENSIVE = 2, CUTTING = 1, PIERCING = 2;

					Player SomePlayer;
					GetPlayer(SomePlayer,formData[1]);

					char WN[1024], AN[1024];
					long PArmour, PWeapon;
					SomePlayer.GetArmour(AN,PArmour);
					SomePlayer.GetWeapon(WN,PWeapon);

					char WeaponFlags[1024];

					int PAttackType = CUTTING;
					int PStance = OFFENSIVE;

					int EAttackType = PIERCING;
					int EStance = DEFENSIVE;

					if (Dice()>=3)
						EAttackType = CUTTING;
					if (Dice()>=3)
						EStance = OFFENSIVE;
					
					if (numItems > 9) // equip item parameter has been included, along with stance, etc
					{
						if (SomePlayer.Possessions.Get(formData[11],WeaponFlags)) // possessions...
						{
							if (WeaponFlags[1] == '+')
							{
								// item that modifies the rating of the existing weapon
								// (usually one-shot, this is to be implemented)
								SomePlayer.SetWeapon(WN,PWeapon+(WeaponFlags[2]-48));
							}
							else
							{
								// item with its own fixed rating...
								SomePlayer.SetWeapon(formData[11],WeaponFlags[1]-48);
							}
						}
						else // spells...
						{
							SomePlayer.SetWeapon(formData[11],GetSpellRating(formData[11]));
						}

						// we have to update this because it's changed since last read...
						SomePlayer.GetWeapon(WN,PWeapon);


						// 9 = stance: 1 offensive, 2 defensive, 10 = attack type: 1 = cutting, 2 = piercing
						
						if (!strcmp(formData[9],"O"))
						{
							PStance = OFFENSIVE;
						}
						else
						{
							PStance = DEFENSIVE;
						}

						if (!strcmp(formData[10],"C"))
						{
							PAttackType = CUTTING;
						}
						else
						{
							PAttackType = PIERCING;
						}

						

					}

					long ESkill = atoi(formData[3]), EStamina = atoi(formData[4]), EArmour = atoi(formData[5]), EWeapon = atoi(formData[6]);

					cout << CR << "Combat: <b>" << SomePlayer.GetUsername() << " vs. " << formData[2] << "</b>" << CR << CR;

                                        if (EStamina <= 0)
                                        {
						cout << "This monster is already dead!" << CR << CR;
                                                strcpy(New,"action=paragraph&guid=");
                                                strcat(New,formData[1]);
                                                strcat(New,"&p=");
                                                strcat(New,formData[7]);
                                                strcat(New,"&fragment=");
                                                strcat(New,formData[8]);
                                                HyperLink("Click here to continue your quest...",New);
                                                return 0;
                                        }

					cout << "<b><u>Information:</b></u>" << CR;
					long EAttack = Dice() + Dice() + ESkill + EWeapon;
					long PAttack = Dice() + Dice() + SomePlayer.GetSkill() + PWeapon;

					cout << "The enemy takes a ";
					if (EStance == DEFENSIVE)
						cout << "defensive";
					else
						cout << "offensive";
					cout << " stance in this battle round. You are being ";
					if (PStance == DEFENSIVE)
						cout << "defensive";
					else
						cout << "offensive";

					if ((EStance == OFFENSIVE) && (PStance == DEFENSIVE))
					{
						int Bonus = Dice();
						EAttack+=Bonus;
						cout << ". Because you are shielding yourself from possible attack and the enemy is bearing down, this means the enemy is more likely to score a hit and gets a <B>" << Bonus << " point</B> bonus to their attack score.";
					}
					if ((PStance == OFFENSIVE) && (EStance == DEFENSIVE))
					{
						int Bonus = Dice();
						PAttack+=Bonus;
						cout << ". Because you are being offensive in stance, and the enemy is shielding themselves from your blow, this means that you are more likely to score a hit and get a <B>" << Bonus << " point</B> bonus to your attack score.";
					}
					if (PStance == EStance)
					{
						cout << ". Because you are both taking the same stance, there are no bonuses to attack scores this round.";
					}
					
						
					cout << " You get a total attack score of " << PAttack << ", the enemy gets a total attack score of " << EAttack << ". ";
					cout << "(" << PWeapon << " point";
					if (PWeapon>1) 
					{
						cout << "s";
					}
					cout << " of this is attributed to your weapon, \"";
					if (!strcmp("Unknown Spell",SpExp(WN)))
					{
						cout << WN;
					}
					else
					{
						cout << SpExp(WN);
					}

					cout << "\"). This means that ";

					long SaveRoll = Dice() + Dice();
					bool Saved = false;

					if (EAttack > PAttack)
					{
						cout << "<b>the enemy hits you</b>.";
						
						if (PStance == DEFENSIVE)
						{
							cout << " Because you are in a defensive stance, you gain a bonus to your armour, and are more able to defend against the blow.";
							SaveRoll-=Dice();
						}
						if (SaveRoll <= PArmour)
						{
							Saved = true;
						}
					}
					else
					{
						cout << "<b>you hit the enemy</b>.";
						
						if (EStance == DEFENSIVE)
						{
							cout << " Because the enemy is cowering in a defensive position this round, it gains extra bonuses to its armour which may increase its chances of avoiding damage.";
							SaveRoll-=Dice();
						}
						if (SaveRoll <= EArmour)
						{
							Saved = true;
						}
					}
					long D6 = Dice();
					long SDamage, KDamage;
					int KAttackType = 0;

					if (EAttack > PAttack)
						KAttackType = EAttackType;
					else
						KAttackType = PAttackType;

					if (Saved)
					{
						cout << " The blow bounces harmlessly off armour...";
					}
					else
					{

						cout << " The blow cuts through armour and lands in the <b>";

						switch (D6)
						{
						case 1:
							cout << "head/neck";
							SDamage = Dice();
							KDamage = 1;
							break;
						case 2:
							cout << "legs/tail";
							SDamage = 3;
							KDamage = 1;
							break;
						case 3:
							cout << "body/torso";
							SDamage = Dice();
							KDamage = 0;
							break;
						case 4:
							cout << "arms/other limbs";
							SDamage = 2;
							KDamage = 2;
							break;
						case 5:
							cout << "hands/claws";
							SDamage = 2;
							KDamage = 1;
							break;
						case 6:
							cout << "weapon";
							SDamage = 0;
							KDamage = 1;
							break;
						}

						cout << "</b> area. ";


						switch (D6)
						{
							case 1:
								if (KAttackType == CUTTING)
								{
									cout << "Because the attack was a cutting attack, it causes severe damage to this part of the body, and extra stamina points are lost as a result!";
									SDamage+=Dice();
								}
							break;
							case 2:
								if (KAttackType = PIERCING)
								{
									cout << "Because the attack was a piercing attack, it causes severe damage to the body, and extra stamina is lost due to the attack!";
									SDamage+=Dice();
								}
							break;
						}

						cout << " This causes ";

						if (SDamage == 0)
						{
							cout << "no <b>stamina</b> loss, ";
						}
						else
						{
							cout << SDamage << " points of <b>stamina</b> loss, ";
						}
						cout << "and ";

						if (KDamage == 0)
						{
							cout << "no <b>skill</b> loss. ";
						}
						else
						{
							cout << KDamage << " points of <b>skill</b> loss. ";
						}


						if (EAttack > PAttack)
						{
							SomePlayer.SetStamina(SomePlayer.GetStamina()-SDamage);
							SomePlayer.SetSkill(SomePlayer.GetSkill()-KDamage);
						}
						else
						{
							EStamina -= SDamage;
							ESkill -= KDamage;
						}

						if (SomePlayer.GetStamina() < 4)
						{
							cout << "You are very disorientated and confused and feeling very weak. ";
						}
						else if (EStamina < 4)
						{
							cout << "The enemy is dazed and staggering, close to death. ";
						}

						if (SomePlayer.GetSkill() < 5)
						{
							cout << "Your hands are trembling and you are unable to properly aim at the enemy, ";
						}
						else if (ESkill < 5)
						{
							cout << "The enemy is unable to focus properly upon you and stares at you trying to predict your next move. ";
						}

						if ((EStamina <= 0) || (SomePlayer.GetStamina() <= 0))
						{
							long NumDeaths = 0;
							char DeathMessage[1024],DM[1024];
							long DeathToUse = (rand() % DEATH_MESSAGES) + 1;
							NumDeaths = 0;
							ifstream Deaths("death.msg");
							while (DeathToUse > NumDeaths)
							{
								Readline(Deaths,DeathMessage);
								NumDeaths++;
							}
							Deaths.close();
							if (SomePlayer.GetStamina() < 0)
							{
								snprintf(DM,1020,DeathMessage,SomePlayer.GetUsername(),formData[2]);
							}
							else
							{
								snprintf(DM,1020,DeathMessage,formData[2],SomePlayer.GetUsername());
							}
							cout << CR << CR << "<b>" << DM << "</b>";

						}

					}

					cout << "<table><tr><td><BR>";

					cout << "<table width=\"100%\"><tr><td valign=top><b>" << SomePlayer.GetUsername() << "</td><td valign=top><b>" << formData[2] << "</td></tr>";

					cout << "<tr>";
					cout << "<td valign=top width=\"50%\">";

					DispStamina(SomePlayer.GetStamina());

					cout << "</td><td>";

					DispStamina(EStamina);

					cout << "</td></tr><tr><td>";


						
					cout << CR << "Skill: " << SomePlayer.GetSkill() << CR;
					cout << "Stamina: " << SomePlayer.GetStamina() << CR;
					cout << "Armour: " << PArmour << CR;
					cout << "Weapon: " << PWeapon << CR;
					cout << "</td>";

					cout << "<td valign=top>" << CR;

					cout << CR << "Skill: " << ESkill << CR;
					cout << "Stamina: " << EStamina << CR;
					cout << "Armour: " << EArmour << CR;
					cout << "Weapon: " << EWeapon << CR;
					cout << "</td>";

					cout << "</tr></table>";				

					bool CombatEnded = false;

					if (EStamina <= 0)
					{
						strcpy(New,"action=award&guid=");
						strcat(New,formData[1]);
						strcat(New,"&p=");
						strcat(New,formData[7]);
						strcat(New,"&fragment=");
						strcat(New,formData[8]);
						HyperLink("Click here to continue your quest...",New);
						CombatEnded = true;
					}
					if ((SomePlayer.GetStamina() <= 0))
					{
						CombatEnded = true;
						HyperLink("Click here to return to the main menu...","action=title");
						cout << "</td></tr></table>"; // end this table to avoid formatting errors to the page
						cout.flush();
					}

					if (!CombatEnded)
					{
						
						cout << "<B><U>Options:<U><B>" << CR;

						char Name[1024],Flags[1024];

						cout << "<form action=\"" << me << "\" method=get>";

						cout << "<input type=hidden name=action value=combat>";
						cout << "<input type=hidden name=guid value=" << formData[1] << ">";
						cout << "<input type=hidden name=name value=\"" << formData[2] << "\">";
						cout << "<input type=hidden name=skill value=" << ESkill << ">";
						cout << "<input type=hidden name=stamina value=" << EStamina << ">";
						cout << "<input type=hidden name=armour value=" << EArmour << ">";
						cout << "<input type=hidden name=weapon value=" << EWeapon << ">";
						cout << "<input type=hidden name=return value=" << formData[7] << ">";
						cout << "<input type=hidden name=fragment value=" << formData[8] << ">";

						cout << "<table><tr>";
						
						
						cout << "<td>Stance: <select name=\"stance\" size=1><option selected value=\"O\">Offensive</option><option value=\"D\">Defensive</option></select></td><td>Attack type: <select name=\"cut\" size=\"1\"><option selected value=\"C\">Cutting</option><option value=\"P\">Piercing</option><select></td>" << CR;


                                                cout << "<td>Use weapon: <select name=\"equip\" size=1>";
                                                for (int i=0; i<=29; i++)
                                                {
                                                        SomePlayer.Possessions.GetItem(i,Name,Flags);
                                                        if (strcmp(Name,"[none]") != 0) // there's an item here...
                                                        {
                                                                if (Flags[0] == 'W') // it's usable as a weapon...
                                                                {
                                                                        if (!strcmp(WN,Name))
                                                                        {
                                                                                cout << "<option selected>" << Name << "</option>";
                                                                        }
                                                                        else
                                                                        {
                                                                                cout << "<option>" << Name << "</option>";
                                                                        }
                                                                }
                                                        }
                                                }

                                                for (int i=0; i<=29; i++)
                                                {
                                                        SomePlayer.Spells.GetItem(i,Name,Flags);
                                                        if (strcmp(Name,"[none]") != 0) // there's an item here...
                                                        {
                                                                // it's usable as a weapon if it has a combat rating and the
                                                                // player is carrying some of its component herb so it can be
                                                                // cast...
                                                                if ((IsCombatSpell(Name)) && (HasComponentHerb(Name,SomePlayer)))
                                                                {
                                                                        if (!strcmp(WN,Name))
                                                                        {
                                                                                cout << "<option selected value=\"" << Name << "\">" << SpExp(Name) << " (spell)</option>";
                                                                        }
                                                                        else
                                                                        {
                                                                                cout << "<option value=\"" << Name << "\">" << SpExp(Name) << " (spell)</option>";
                                                                        }
                                                                }
                                                        }
                                                }
                                                cout << "</select></td>";





						cout << "</tr></table>";

						cout << "</td><td>";
						
						cout << "<input type=\"image\" style=\"cursor:crosshair\" src=\"" << IMAGE_DIR << "/body.jpg\"><br>Click an area to attack it";
						cout << "</form>";
						cout << "</td></tr></table>";
					}

					PutByGuid(formData[1],SomePlayer);
					return 0;
				}

				if (!strcmp(formData[0],"equip"))
				{
					char N[1024],F[256];
					bool rec = false;

					// We have to figure out what kind of item the thing is, if we want
					// to figure out a way to automatically equip it...

					GetPlayer(SomePlayer,formData[1]);

					SomePlayer.Possessions.Get(formData[2],F);

					if (F[0] == 'W')
					{
						// it's a weapon...
						SomePlayer.SetWeapon(formData[2],F[1]-48);
						rec = true;
					}

					if (F[0] == 'A')
					{
						// it's armour...
						SomePlayer.SetArmour(formData[2],F[1]-48);
						rec = true;
					}

					if (!rec)
					{
						// it's something else... (not implemented!)
					}

					PutByGuid(formData[1],SomePlayer);

					sprintf(N,"%s?action=pack&guid=%s",me,formData[1]);
					RedirectTo(N);
					return 0;
				}


				if (!strcmp(formData[0],"use"))
				{
					char N[1024],F[256],P[256];
					bool rec = false;

					GetPlayer(SomePlayer,formData[1]);
					SomePlayer.GetParagraph(P);

                                        if (SomePlayer.LastUse > time(NULL)-1)
                                        {
                                                ContentType();
                                                HtmlStart();
                                                cout << "WOAH THERE! Slow down!<br><br>Youre going too fast. The rate limit for using items is 1 item every second, try again, and this time slower!" << CR << CR;
                                                sprintf(New,"action=pack&guid=%s",formData[1]);
                                                HyperLink("Go back",New);
						return 0;
					}					
					if (!SomePlayer.Possessions.Get(formData[2],F))
					{
						ContentType();
						HtmlStart();
						cout << "OI! Stop cheating and trying to use what you don't have, or i'll have to get medieval on your ass! :-)";
						sprintf(New,"action=paragraph&guid=%s&p=%s",formData[1],P);
						HyperLink("Go back",New);
						return 0;
					}

					SomePlayer.Possessions.Delete(formData[2]);
					PutByGuid(formData[1],SomePlayer);

					if ((F[0] == 'S') && (F[1] == 'T')) // stamina modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							SomePlayer.AddStamina(F[3]-48);
						}
						else
						{
							SomePlayer.SetStamina(F[2]-48);
						}
					}


					if ((F[0] == 'S') && (F[1] == 'K')) // skill modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							SomePlayer.AddSkill(F[3]-48);
						}
						else
						{
							SomePlayer.SetSkill(F[2]-48);
						}
					}


					if ((F[0] == 'S') && (F[1] == 'N')) // sneak modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							SomePlayer.AddSneak(F[3]-48);
						}
						else
						{
							SomePlayer.SetSneak(F[2]-48);
						}
					}


					if ((F[0] == 'L') && (F[1] == 'K')) // luck modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							SomePlayer.AddLuck(F[3]-48);
						}
						else
						{
							SomePlayer.SetLuck(F[2]-48);
						}
					}


					if ((F[0] == 'S') && (F[1] == 'P')) // speed modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							SomePlayer.AddSpeed(F[3]-48);
						}
						else
						{
							SomePlayer.SetSpeed(F[2]-48);
						}
					}


					if ((F[0] == 'W')) // weapon modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							char N[1024];
							long R;
							SomePlayer.GetWeapon(N,R);
							SomePlayer.SetWeapon(N,R+(F[3]-48));
						}
					}

					if ((F[0] == 'A')) // armour modifier
					{
						if ((F[2] == '+') || (F[2] == ' '))
						{
							long R;
							SomePlayer.GetArmour(N,R);
							SomePlayer.SetArmour(N,R+(F[3]-48));
						}
					}


					if ((strstr(formData[2],"5 ration")) || (strstr(formData[2],"5 Ration"))) // ration
					{
						SomePlayer.AddRations(5);
					}
					else
					if ((strstr(formData[2],"4 ration")) || (strstr(formData[2],"4 Ration"))) // ration
					{
						SomePlayer.AddRations(4);
					}
					else
					if ((strstr(formData[2],"3 ration")) || (strstr(formData[2],"3 Ration"))) // ration
					{
						SomePlayer.AddRations(2);
					}
					else
					if ((strstr(formData[2],"2 ration")) || (strstr(formData[2],"2 Ration"))) // ration
					{
						SomePlayer.AddRations(2);
					}
					else
					if ((strstr(formData[2],"ration")) || (strstr(formData[2],"Ration"))) // ration
					{
						SomePlayer.AddRations(1);
					}

					PutByGuid(formData[1],SomePlayer);

					sprintf(N,"%s?action=pack&guid=%s",me,formData[1]);
					RedirectTo(N);
					return 0;
				}


				if (!strcmp(formData[0],"title"))
				{
					TitlePage();
					log("Site visited by %s",getenv("REMOTE_ADDR"));
					return 0;
				}

				if (!strcmp(formData[0],"login"))
				{
					bool display = true;
					/*if (getenv("HTTP_COOKIE"))
					{
						char cookie[1024];
						strcpy(cookie,getenv("HTTP_COOKIE"));
						if (strcmp(cookie,"credentials=INVALID_COOKIE"))
						{
							char guid[1024],user[1024],pass[1024];
							int i = 0, q = 0;
							char n[2];
							n[1] = '\0';
							user[0] = '\0';
							pass[0] = '\0';
							guid[0] = '\0';
							while (cookie[i] != ':' && i < strlen(cookie))
							{
								n[0] = cookie[i];
								strcat(guid,n);
								i++;
							}
							i++;
		                                        while (cookie[i] != ':' && i < strlen(cookie))
		                                        {
		                                                n[0] = cookie[i];
		                                                strcat(user,n);
								i++;
		                                        }
							i++;
							q = i;
							while (i < strlen(cookie))
							{
							        n[0] = cookie[i];
							        strcat(pass,n);
								i++;
							}
														
							char moo[1024];
							sprintf(moo,"%s?action=load&username=%s&password=%s&remember=1",me,user,pass);
							RedirectTo(moo);
							return 0;
						}
					}*/
					if (display)
					{
						StandardLogin();
					}
					return 0;
				}

			
				if (!strcmp(formData[0],"info"))
				{
					char info_file[1024];
					RedirectTo("/info.html");
					return 0;
				}

				if (!strcmp(formData[0],"grimoire"))
				{
					char info_file[1024];
					strcpy(info_file,DOCUMENT_DIR);
					strcat(info_file,"/grimoire.html");
					RedirectTo(info_file);
					return 0;
				}

				if (!strcmp(formData[0],"restore"))
				{
					char NewURI[1024];
					Player AP;
					ContentType();
					HtmlStart();

					int exp_cost = RestoreBackup(formData[1]);
					GetPlayer(AP,formData[1]);
					
					cout << "<b><u>Resurrection is complete!</b></u>" << CR << CR;

					if (exp_cost)
					{
						cout << "Welcome back to the land of the living, <b>" << AP.GetUsername() << "</b>! The gods have granted you another life in which to appease them, however this was not without a cost! As a payment in karma, the gods have charged you a price of <b>" << exp_cost << "</b> experience points for this rebirth!" << CR << CR;
					}
					else
					{
						cout << "Welcome back to the land of the living, <b>" << AP.GetUsername() << "</b>! The gods have granted you another life in which to appease them, and as you are young, and pure of heart with little sin, the gods have looked kindly upon you and resurrected you at no cost..." << CR << CR;
					}
					cout << "<table><tr><td style='border:solid #800000 .25pt' background='" << IMAGE_DIR << "/moon.jpg' width='157' height='280'>";
					cout << "<img src='" << IMAGE_DIR << "/death.png'>";
					cout << "</td></tr></table>";
					
					cout << CR << CR << "<b><u>Please choose an option:</b></u>" << CR << CR;
					sprintf(NewURI,"action=itempicker&guid=%s",formData[1]);
					HyperLink("Continue to spell selection, i want to pick different spells this time",NewURI);
					cout << CR;
					sprintf(NewURI,"action=oldspells&guid=%s",formData[1]);
					HyperLink("Restart with same spells as last game, the ones i had are OK",NewURI);
					cout << CR;
					
					log("User %s was resurrected.", AP.GetUsername());
					return 0;
				}

				if (!strcmp(formData[0],"rebuild"))
				{
					ContentType();
					HtmlStart();

					cout << "<b><u>Character re-creation</b></u>" << CR << CR;

					cout << "Your character died in the last game you played, so you cannot continue from ";
					cout << "your previous point. However, you can resurrect your character and start again ";
					cout << "from the starting point, to try again. <b>All possessions, herbs etc you were carrying have been droppped to the floor at the place where your character died</b>. You will be charged a number of experience points depending on your level, to be resurrected, but only if your level was 4 or over when you died." << CR << CR;
					cout << "<font size=+1>";
					strcpy(New,"action=restore&guid=");
					strcat(New,formData[1]);
					HyperLink("Click here to restore your character to its original healthy self...",New);
					// clear all chat html, should stop refreshes etc
					char moo[1024];
					sprintf(moo,"DELETE FROM game_chat_events WHERE target_location='%s'",formData[1]);
					mysql_query(&my_connection,moo);
					return 0;
				}

				if (!strcmp(formData[0],"forget"))
				{
					cout << "Status: 302 Found\n";
					cout << "Location: " << me << "?action=title\n";
					cout << "Set-Cookie: credentials=INVALID_COOKIE; expires=Tue, 30-Jun-2099 00:00:00 GMT; path=/cgi-bin; domain=www.ssod.org;\n";
					cout << "Content-Type: text/html\n\n";
					cout << "<html>Redirects not supported?</html>";
					return 0;
				}

				if (!strcmp(formData[0],"load"))
				{
					Player APlayer;
					char TheFile[1024];
					LpToGuid(TheFile,formData[1],formData[2]);
					GetPlayer(APlayer, TheFile);
					if (APlayer.Auth(formData[1],formData[2]))
					{
						if (!APlayer.Validated)
						{
							ContentType();
							HtmlStart();
							log("%s tried to log in before authorizing character by email",APlayer.GetUsername());
							cout << "You have not yet <b>activated your account</b>. Please follow the instructions in the email you will have received, and then try again.";
							return 0;
						}
						// found a valid user
						char NewURI[1024];
						log("%s has logged in", formData[1]);

                                                strcpy(NewURI,me);
                                                strcat(NewURI,"?action=select&guid=");
                                                strcat(NewURI,TheFile);
					        cout << "Status: 302 Found\n";
					        cout << "Location: " << NewURI << "\n";
                                                if (!strcmp(formData[3],"1"))
                                                {
		                                        cout << "Set-Cookie: credentials=" << TheFile << ":" << formData[1] << ":" << formData[2] << "; expires=Tue, 30-Jun-2099 00:00:00 GMT; path=/cgi-bin; domain=www.ssod.org;\n";
		                                }

                                                cout << "Content-Type: text/html\n\n";
                                                cout << "<html><META HTTP-EQUIV=\"Expires\" CONTENT= \"Tue, 20 Aug 1900 14:25:27 GMT\">";
                                                cout << "<body bgcolor=black><font color=red>";
                                                cout << "<HTML>Redirection to <a href=\"" << NewURI << "\">" << NewURI << "</a></HTML>";
						
						
						if (getenv("REMOTE_ADDR"))
						{
							strcpy(APlayer.LastIP,getenv("REMOTE_ADDR"));
						}		
						PutByGuid(TheFile, APlayer);
						return 0;
					}
					ContentType();
					HtmlStart();
					cout << "Username or password was incorrect. Please use your browser's back button and try again.";
					log("Bad login as user %s", formData[1]);
					return 0;
				}

				if (!strcmp(formData[0],"select"))
				{
					Player ThePlayer;
					GetPlayer(ThePlayer, formData[1]);
					if (ThePlayer.GetStamina() <= 0)
					{
						// Player needs re-create their character, because it died!
						strcpy(New,me);
						strcat(New,"?action=rebuild&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						ThePlayer.ResetToSpawnPoint();
						PutByGuid(formData[1],ThePlayer);
						return 0;
					}
					else
					if (ThePlayer.Spells.Count() == 0)
					{
						// Player needs to pick spells and herbs before proceeding
						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						ThePlayer.ResetToSpawnPoint();
						PutByGuid(formData[1],ThePlayer);
						return 0;
					}
					else
					{
						if (ThePlayer.GetStamina() > 0)
						{
							// Player is completely set up, can proceed/continue from
							// last known point
							strcpy(New,me);
							strcat(New,"?action=paragraph&guid=");
							strcat(New,formData[1]);
							strcat(New,"&p=");
							char u[1024];
							ThePlayer.GetParagraph(u);
							strcat(New,u);
							RedirectTo(New);
							return 0;
						}
					}
					return 0;
				}
				if (!strcmp(formData[0],"oldspells"))
				{
					Player ThePlayer;
                                        int res;
                                        MYSQL_RES *a_res;
                                        MYSQL_ROW a_row;
                                        char query[1024];
																				
					GetPlayer(ThePlayer, formData[1]);

                                        sprintf(query,"SELECT name,flags FROM game_last_spells WHERE guid='%s'",formData[1]);
                                        res = mysql_query(&my_connection,query);
                                        if (!res)
                                        {
	                                        a_res = mysql_use_result(&my_connection);
	                                        if (a_res)
	                                        {
		                                        while (a_row = mysql_fetch_row(a_res))
		                                        {
								if (!strcmp(a_row[1],"HERB"))
								{
									ThePlayer.Herbs.Add(a_row[0],a_row[1]);
								}
								else
								{
									ThePlayer.Spells.Add(a_row[0],a_row[1]);
								}
			                                }
			                                mysql_free_result(a_res);
                                                }
                                        }
					PutByGuid(formData[1],ThePlayer);
					strcpy(New,me);
					strcat(New,"?action=paragraph&guid=");
					strcat(New,formData[1]);
					strcat(New,"&p=");
					char u[1024];
					ThePlayer.GetParagraph(u);
					strcat(New,u);
					RedirectTo(New);
					ThePlayer.ResetToSpawnPoint();
					PutByGuid(formData[1],ThePlayer);
					return 0;
					
				}
				if (!strcmp(formData[0],"savechoices"))
				{
					Player ThePlayer;
					char query[1024];
					GetPlayer(ThePlayer, formData[1]);
					if (ThePlayer.Spells.Count() == 0)
					{
						// Player needs to pick spells and herbs before proceeding
						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						return 0;
					}
					else
					{
						// save initial spell choices
						sprintf(query,"DELETE FROM game_last_spells WHERE guid='%s'",formData[1]);
						mysql_query(&my_connection,query);
						int i = 0;
						for (i = 0; i <= 30; i++)
						{
							if (strcmp(ThePlayer.Herbs.List[i].Name,"[none]"))
							{
								sprintf(query,"INSERT INTO game_last_spells VALUES('%s','%s','%s')",formData[1],ThePlayer.Herbs.List[i].Name,ThePlayer.Herbs.List[i].FlagInfo);
								mysql_query(&my_connection,query);
							}
							if (strcmp(ThePlayer.Spells.List[i].Name,"[none]"))
							{
								sprintf(query,"INSERT INTO game_last_spells VALUES('%s','%s','%s')",formData[1],ThePlayer.Spells.List[i].Name,ThePlayer.Spells.List[i].FlagInfo);
								mysql_query(&my_connection,query);
							}
						}
						// Player is completely set up, can proceed/continue from
						// last known point
						strcpy(New,me);
						strcat(New,"?action=paragraph&guid=");
						strcat(New,formData[1]);
						strcat(New,"&p=");
						char u[1024];
						ThePlayer.GetParagraph(u);
						strcat(New,u);
						RedirectTo(New);
						ThePlayer.ResetToSpawnPoint();
						PutByGuid(formData[1],ThePlayer);
						return 0;
					}
					return 0;
				}
				if (!strcmp(formData[0],"buy"))
				{

						GetPlayer(SomePlayer, formData[1]);
						int gCost = atoi(formData[3]);
						char ItemName[1024], Flags[1024];
						Base64Decode(formData[2], ItemName);
						Base64Decode(formData[4], Flags);

						gCost = ((gCost / 2) ^ 31337);
						if ((gCost > 9999) || (gCost < 0))
						{
							ContentType();
							HtmlStart();
							cout << "Uh oh, that WASNT very clever, now was it?";
							SomePlayer.Pinned = time(NULL) + 3600;
							PutByGuid(formData[1],SomePlayer);
							return 0;
						}

						char flags[1024];

						if (Flags[0] == '\0')
						{
							strcpy(flags,"[none]");
						}
						else
						{
							strcpy(flags,Flags);
						}

						if (SomePlayer.GetGold() >= gCost)
						{
							bool backpack = true;

							if (!strcmp(flags,"HERB"))
							{
								SomePlayer.Herbs.Add(ItemName,flags);
								backpack = false;
							}
							if (!strcmp(flags,"SPELL"))
							{
								SomePlayer.Spells.Add(ItemName,flags);
								backpack = false;
							}
							if (backpack)
							{

								if (!strcmp(Lower(ItemName),"scroll"))
								{
									// Don't allow a player to double-purchase any scrolls
									// that happen to be for sale!

									if (NotGotYet(formData[5],"SCROLL",SomePlayer.GotFrom))
									{
										SomePlayer.AddScroll();
										strcat(SomePlayer.GotFrom," [");
										strcat(SomePlayer.GotFrom,"SCROLL");
										strcat(SomePlayer.GotFrom,formData[5]);
										strcat(SomePlayer.GotFrom,"]");
										SomePlayer.SetGold(SomePlayer.GetGold()-gCost);
										PutByGuid(formData[1],SomePlayer);
										strcpy(New,me);
										strcat(New,"?action=paragraph&guid=");
										strcat(New,formData[1]);
										strcat(New,"&p=");
										char z[1024];
										SomePlayer.GetParagraph(z);
										strcat(New,z);
										RedirectTo(New);
										return 0;
									}
									else
									{
										strcpy(New,me);
										strcat(New,"?action=paragraph&guid=");
										strcat(New,formData[1]);
										strcat(New,"&p=");
										char z[1024];
										SomePlayer.GetParagraph(z);
										strcat(New,z);
										RedirectTo(New);
										return 0;
									}
								}

								SomePlayer.Possessions.Add(ItemName,flags);
							}

							SomePlayer.SetGold(SomePlayer.GetGold()-gCost);
						}

						PutByGuid(formData[1],SomePlayer);

						strcpy(New,me);
						strcat(New,"?action=paragraph&guid=");
						strcat(New,formData[1]);
						strcat(New,"&p=");
						char z[1024];
						SomePlayer.GetParagraph(z);
						strcat(New,z);						
						RedirectTo(New);
						return 0;
				}
				
				if (!strcmp(formData[0],"addherb"))
				{
						GetPlayer(SomePlayer, formData[1]);
						if (SomePlayer.Herbs.Count() < 5)
						{
							SomePlayer.Herbs.Add(formData[2],"HERB");
						}
						PutByGuid(formData[1],SomePlayer);

						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						return 0;
				}

				if (!strcmp(formData[0],"floorpick"))
				{
						char P[1024];
                                	        int res;
                        	                MYSQL_RES *a_res;
                	                        MYSQL_ROW a_row;
        	                                char query[1024], data1[1024],data2[1024];

						strcpy(data1,"[none]");
						strcpy(data2,"[none]");

						GetPlayer(SomePlayer,formData[1]);
                                                SomePlayer.GetParagraph(P);

						if (SomePlayer.LastUse > time(NULL)-1)
						{
							ContentType();
							HtmlStart();
							cout << "WOAH THERE! Slow down!<br><br>Youre going too fast. The rate limit for picking up items is 1 item every second, try again, and this time slower!" << CR << CR;
							sprintf(New,"action=paragraph&guid=%s&p=%s",formData[1],P);
							HyperLink("Go back",New);
							return 0;
						}

						if (strcmp(P,formData[3]))
						{
							ContentType();
							HtmlStart();
							cout << "What a lameass cheat thou art!";
							return 0;
						}

						sprintf(query,"SELECT DISTINCT item_desc,item_flags FROM game_dropped_items WHERE location_id='%s' AND item_desc=\"%s\"",P,formData[2]);
                                                res = mysql_query(&my_connection,query);
                                                if (!res)
                                                {
                                                        a_res = mysql_use_result(&my_connection);
                                                        if (a_res)
                                                        {
                                                                while (a_row = mysql_fetch_row(a_res))
                                                                {
									strcpy(data1,a_row[0]);
									strcpy(data2,a_row[1]);
                                                                }
								mysql_free_result(a_res);

                                                        }
                                                }
                                                else
                                                {
							ContentType();
							HtmlStart();
                                                        cout << "SELECT error: " << mysql_error(&my_connection);
							return 0;
                                                }

						if (!strcmp(data1,"[none]"))
						{
							sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],P);
							RedirectTo(New);
							return 0;
						}

                                                char data[1024];
                                                sprintf(data,"/m2 picks up a %s from the ground",formData[2]);
                                                SayToRoom(formData[1],data);						

						sprintf(query,"DELETE FROM game_dropped_items WHERE location_id='%s' AND item_desc=\"%s\"",P,formData[2]);
						res = mysql_query(&my_connection,query);
						
						if (!strcmp(data2,"HERB"))
 						{
	                                               SomePlayer.Herbs.Add(data1,data2);
						}
			                        else if (!strcmp(data2,"SPELL"))
						{
						       SomePlayer.Spells.Add(data1,data2);
						}
						else
						{
						       SomePlayer.Possessions.Add(data1,data2);
						}						
						PutByGuid(formData[1],SomePlayer);
						sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],P);
						RedirectTo(New);
						return 0;						
				}

				
				if (!strcmp(formData[0],"eat"))
				{
					GetPlayer(SomePlayer, formData[1]);

					if (SomePlayer.GetStamina()>0)
					{
						if (SomePlayer.GetRations() != 0)
						{
							SomePlayer.SetRations(SomePlayer.GetRations()-1);
							SomePlayer.AddStamina(2);
						}

						char data[1024];
						sprintf(data,"/m2 eats a tasty ration",formData[2]);
						SayToRoom(formData[1],data);
						PutByGuid(formData[1],SomePlayer);
					}
					char q[1024];
					SomePlayer.GetParagraph(q);
					sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],q);
					RedirectTo(New);
					return 0;
				}

				
				if (!strcmp(formData[0],"dump"))
				{
					char Flags[128],Para[128];
					GetPlayer(SomePlayer, formData[1]);
					SomePlayer.Possessions.Get(formData[2],Flags);
					SomePlayer.GetParagraph(Para);

					for (int i = 0; i < strlen(Flags); i++)
						if (Flags[i] == ' ')
							Flags[i] = '+';

					char data[1024];
					sprintf(data,"/m2 drops a %s to the ground",formData[2]);
					SayToRoom(formData[1],data);
					sprintf(query,"INSERT INTO game_dropped_items VALUES('%s',\"%s\",'%s')",Para,formData[2],Flags);

					SomePlayer.Possessions.Delete(formData[2]);					
                                        mysql_query(&my_connection,query);					

					PutByGuid(formData[1],SomePlayer);

					char q[1024];
					SomePlayer.GetParagraph(q);
					sprintf(New,"%s?action=pack&guid=%s",me,formData[1]);
					RedirectTo(New);
					return 0;
				}

				
				if (!strcmp(formData[0],"map"))
				{
					ContentType();
					HtmlStart();
					GetPlayer(SomePlayer,formData[1]);
					cout << CR << "<b><u>Map Of Utopia</u></b>" << CR << CR;
					cout << "<center><img src=\"" << IMAGE_DIR << "/map.jpg\">" << CR << CR;
					char addr[1024];
					char para[256];
					SomePlayer.GetParagraph(para);
					sprintf(addr,"action=paragraph&guid=%s&p=%s",formData[1],para);
					HyperLink("Click here to continue your game...",addr);
					return 0;
				}

				if (!strcmp(formData[0],"pick"))
				{
					GetPlayer(SomePlayer,formData[1]);
					char Para[256];
					SomePlayer.GetParagraph(Para);
					if (NotGotYet(Para,"PICKED",SomePlayer.GotFrom))
					{
						SomePlayer.Possessions.Add(formData[2],formData[3]);
						strcat(SomePlayer.GotFrom," [");
						strcat(SomePlayer.GotFrom,"PICKED");
						strcat(SomePlayer.GotFrom,Para);
						strcat(SomePlayer.GotFrom,"]");	
					}
					PutByGuid(formData[1],SomePlayer);
					sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Para);
					RedirectTo(New);
					return 0;
				}
				

                                if (!strcmp(formData[0],"pjump"))
				{
                                        Player SomePlayer;
					GetPlayer(SomePlayer, formData[1]);
					char XPara[256];
					char data[1024], OldPara[1024];
					SomePlayer.GetParagraph(XPara);
					strcpy(OldPara,formData[2]);

					if (strcmp(XPara,formData[2]))
					{
                                        	sprintf(data,"/m2 vanishes in a bright flash of light");
						SayToRoom(formData[1],data);
					}
								
					SomePlayer.SetParagraph(formData[2]); // update location
					PutByGuid(formData[1],SomePlayer);
					
                                        if (strcmp(XPara,OldPara))
					{
						sprintf(data,"/m2 appears, accompanied by a crash of thunder");
                                        	SayToRoom(formData[1],data);
					}
					
					formData[0] = "paragraph";
					// fall through to 'paragraph' type handler
				}


																
				if (!strcmp(formData[0],"paragraph"))
				{
					Player SomePlayer;
					GetPlayer(SomePlayer, formData[1]);

					if (getenv("REMOTE_ADDR"))
					{
						if (strcmp(getenv("REMOTE_ADDR"),SomePlayer.LastIP))
						{
							// IP Of player does not match last login IP, re-prompt for
							// user credentials...
							char moo[1024];
							sprintf(moo,"%s?action=login",me);
							RedirectTo(moo);
							return 0;
						}
					}

					if (SomePlayer.LastUse < time(NULL)-3600)
					{
						// Player has not been active for more than an hour, needs to re-auth
						char moo[1024];
						sprintf(moo,"%s?action=login",me);
						RedirectTo(moo);
						return 0;
					}
					
					ContentType();
					HtmlStart();
					StopCheat();
					
					char XPara[256];
					SomePlayer.GetParagraph(XPara);

					/*char AuthKey[1024];
					bool bypass = false;
					strcpy(AuthKey,"");
					if (numItems > 3)
					{
						LpToGuid(AuthKey,formData[2],formData[2]);
						if (!strcmp(AuthKey,formData[3]))
						{
							bypass = true;
						}
					}

					if (!bypass)
					{*/
						if (!ValidNextParagraph(XPara,formData[2]))
						{
							if (SomePlayer.GetDays() != -666)
							{
								cout << CR << "<B>Error!<B><BR><BR>There was an error accessing this location. Please log out and log back in to clear the error, and try again." << CR;
								log("%s attempted to cheat! [CurrentPara=%s, InvalidPara=%s, ValidList=(%s)]",SomePlayer.GetUsername(),XPara,formData[2],VPList);
								return 0;
							}
						}
					//}

					char data[1024];
					bool didntmove = false;

					if (!strcmp(formData[2],XPara))
					{
						// location same as last (login, refresh)
						PutByGuid(formData[1],SomePlayer);
						didntmove = true;
					}
					else
					{
						// if a player is pinned (value not 0) and pinning time isnt expired yet... they cant move
						if ((SomePlayer.Pinned) && (time(NULL) < SomePlayer.Pinned))
						{
							// nothing to see here, move along!
							didntmove = true;
						}
						else
						{
		                                        sprintf(data,"/m2 leaves the location and goes elsewhere");
		                                        SayToRoom(formData[1],data);
							SomePlayer.SetParagraph(formData[2]); // update location
							PutByGuid(formData[1],SomePlayer);
							sprintf(data,"/m2 wanders into the location");
							SayToRoom(formData[1],data);
						}
					}
					
					if (!NotGotYet("0","DEBUG",SomePlayer.GotFrom))
					{
						cout << "<table width=\"80%\" style='border:solid red .25pt'><tr><td><font size=-1><u>Debug panel</u> <b>Last</b>: " << XPara << " <b>Current</b>: " << formData[2] << " <b>Valid</b>: (" << VPList << ") <b>Teleport to</b>: <form action=\"" << me << "\" method=get>";
						cout << "<input type='hidden' name='action' value='pjump'>";
						cout << "<input type='hidden' name='guid' value='" << formData[1] << "'>";
						cout << "<input type='text' name='p' size=5 value='" << formData[2] << "'><input type='submit' value='Cast'></form>";

						cout << "</td></tr></table>";
					}
					
					char para[1024], LastLink[1024];
						
					strcpy(LastLink,formData[2]);
					if ((SomePlayer.Pinned) && (time(NULL) < SomePlayer.Pinned))
					{
						strcpy(LastLink,XPara);
					}

					if ((SomePlayer.GetDays()<1) && (SomePlayer.GetDays() != -666))
					{
						SomePlayer.SetDays(-666);
						SomePlayer.SetScrolls(0); // garneth nicks them all!
						strcpy(LastLink,"40");
					}

					long Words = 0;

					cout << "<table width=\"100%\" border=0><tr><td width=\"33%\" align='left'>";
					cout << "<b><u>" << SomePlayer.Username << " (";
					cout << Race(SomePlayer.GetRace()) << " ";
					cout << Profession(SomePlayer.GetProfession()) << ")</b></u>";
					cout << "</td><td width=\"33%\" valign=top>";
					cout << "<b><u><center>Location...</b></u>";
					cout << "</td><td width=\"33%\" valign=top>";
					cout << "<b><u><center>Possessions (";
					cout << SomePlayer.Possessions.Count() << "/30 items)</u> ";
					cout << "<img src='" << IMAGE_DIR << "/scroll-up.gif' border='0' onmousedown=\"backpack.scrollBy(0,-9)\" onmouseup=\"backpack.scrollBy(0,-9)\" alt='Scroll list up'> ";
					cout << "<img src='" << IMAGE_DIR << "/scroll-down.gif' border='0' onmousedown=\"backpack.scrollBy(0,9)\" onmouseup=\"backpack.scrollBy(0,9)\" alt='Scroll list down'>";
					cout << "</b></u>";
					cout << "</td></tr>";

					cout << "<tr><td valign=top>";
					// stats

					long Level = 0, Special = 0;
					while ((SomePlayer.GetExperience() >= Levels[Level]) && (Level != 20)) Level++;

					
					if (SomePlayer.GetProfession() == Warrior)
						Special+=5;
					if ((SomePlayer.GetRace() == Barbarian) || (SomePlayer.GetRace() == Orc))
						Special+=4;
					
					if (SomePlayer.GetStamina() > 22+Level+Special)
						SomePlayer.SetStamina(22+Level+Special);
					if (SomePlayer.GetSkill() > 22+(Level))
						SomePlayer.SetSkill(22+(Level));
					if (SomePlayer.GetLuck() > 22+(Level))
						SomePlayer.SetLuck(22+(Level));
					if (SomePlayer.GetSneak() > 15+(Level))
						SomePlayer.SetSneak(15+(Level));
					if (SomePlayer.GetSpeed() > 22+(Level))
						SomePlayer.SetSpeed(22+(Level));
					
					long MaxGold, MaxSilver, MaxRations;
					
					if (SomePlayer.GetExperience() >= 0)
					{
						MaxGold = 50 * Level;
						MaxSilver = 100 * Level;
						MaxRations = 30 * Level;
					}
					else
					{
						MaxGold = 10;
						MaxSilver = 10;
						MaxRations = 10;
					}
					
                                        if (SomePlayer.GetGold() > MaxGold)
                	                        SomePlayer.SetGold(MaxGold);
                                        if (SomePlayer.GetSilver() > MaxSilver)
        	                                SomePlayer.SetSilver(MaxSilver);
                                        if (SomePlayer.GetRations() > MaxRations)
	                                        SomePlayer.SetRations(MaxRations);

					PutByGuid(formData[1],SomePlayer);

					cout << "<table><tr><td valign=top width=\"62%\">";

					cout << "<table>";
					cout << "<tr><td>Stamina: </td><td><b>";
					cout << SomePlayer.GetStamina() << "/" << 22+(Level+Special) << "</b> " << Maxed(SomePlayer.GetStamina(),22+Level+Special) << Bonuses(1,SomePlayer.GetRace(),SomePlayer.GetProfession()) << "</td></tr>";
					cout << "<tr><td>Skill: </td><td><b>";
					cout << SomePlayer.GetSkill() << "/" << 22+(Level) << "</b> " << Maxed(SomePlayer.GetSkill(),22+Level) << Bonuses(2,SomePlayer.GetRace(),SomePlayer.GetProfession()) << "</td></tr>";
					cout << "<tr><td>Luck: </td><td><b>";
					cout << SomePlayer.GetLuck() << "/" << 22+(Level) << "</b> " << Maxed(SomePlayer.GetLuck(),22+Level) << Bonuses(3,SomePlayer.GetRace(),SomePlayer.GetProfession()) << "</td></tr>";
					cout << "<tr><td>Sneak: </td><td><b>";
					cout << SomePlayer.GetSneak() << "/" << 15+(Level) << "</b> " << Maxed(SomePlayer.GetSneak(),15+Level) << Bonuses(4,SomePlayer.GetRace(),SomePlayer.GetProfession()) << "</td></tr>";
					cout << "<tr><td>Speed: </td><td><b>";
					cout << SomePlayer.GetSpeed() << "/" << 22+(Level) << "</b> " << Maxed(SomePlayer.GetSpeed(),22+Level) << Bonuses(5,SomePlayer.GetRace(),SomePlayer.GetProfession())  << "</td></tr>";
					cout << "<tr><td>Gold: </td><td><b>";
					cout << SomePlayer.GetGold() << "/" << 50 * Level << "</b>" << Maxed(SomePlayer.GetGold(), MaxGold) << "</td></tr>";
					cout << "<tr><td>Silver: </td><td><b>";
					cout << SomePlayer.GetSilver() << "/" << 100 * Level << "</b> " << Maxed(SomePlayer.GetSilver(), MaxSilver);

					char e[1024];
					if (!SomePlayer.IsDead())
					{
						sprintf(e,"action=convert&guid=%s",formData[1]);
						ImageLink("conv.jpg","Convert silver to gold, 2 silver pieces = 1 gold piece",e);
					}

					cout << "</td></tr><tr><td>Rations: </td><td><b>";
					cout << SomePlayer.GetRations() << "/" << 30 * Level << "</b> " << Maxed(SomePlayer.GetRations(), MaxRations);
					sprintf(e,"action=eat&guid=%s",formData[1]);
					if (!SomePlayer.IsDead())
					{
						ImageLink("eat.jpg","Eat a ration to gain stamina in an emergency",e);
					}
					cout << CR << "</td></tr><tr><td>Experience: </td><td><b>";
					cout << SomePlayer.GetExperience() << "</b></td></tr>";
					cout << "<tr><td>Next Level: </td><td><b>";
					if (Level == 20)
					{
						cout << Levels[20];
					}
					else
					{
						cout << Levels[Level];
					}
					cout << "</b></td></tr>";
					cout << "<tr><td>Notoriety: </td><td><b>";
					cout << SomePlayer.GetNotoriety() << "</b></td></tr>";

					long MaxMana;
					bool ManaWasUp = false;
					
					if (SomePlayer.GetProfession() == Wizard)
					{
						// Wizards regain 2 mana per 15 mins, so long as theyre logged into the game.
						// *ANY* amount of time OVER an hour, while logged out, counts as one hour only.
						if (SomePlayer.ManaTick < time(NULL)-900)
						{
							SomePlayer.ManaTick = time(NULL);
							SomePlayer.Mana+=2;
							ManaWasUp = true;
						}
						if (SomePlayer.Mana > 10+(Level*6))
							SomePlayer.Mana = 10+(Level*6);
						MaxMana = 10+(Level*6);
						PutByGuid(formData[1],SomePlayer);
					}
					else
					{
						// other professions gain 1 mana point per 15 mins, with the same timing rules
						// as wizards.
						if (SomePlayer.ManaTick < time(NULL)-900)
						{
							SomePlayer.ManaTick = time(NULL);
							SomePlayer.Mana++;
							ManaWasUp = true;
						}
						if (SomePlayer.Mana > 10+(Level*2))
							SomePlayer.Mana = 10+(Level*2);
						MaxMana = 10+(Level*2);
						PutByGuid(formData[1],SomePlayer);
					}
					
					cout << "<tr><td>Mana: </td><td><b>" << SomePlayer.Mana << "/" << MaxMana << "</b>" << Maxed(SomePlayer.Mana,MaxMana);
					if ((ManaWasUp) && (SomePlayer.Mana != MaxMana))
					{
						cout << " <font color=yellow>";
						if (SomePlayer.GetProfession() == Wizard)
						{
							cout << "(+2!)";
						}
						else
						{
							cout << "(+1!)";
						}
						cout << "</font></td></tr>";
					}
					cout << "</table>";
					if (SomePlayer.Pinned)
					{
						cout << "<b><u>You cannot move!</b></u>" << CR;
						cout << "You may move again in" << CR;
						cout << SomePlayer.Pinned-time(NULL) << " seconds";
					}
					cout << "</td><td valign=top>";

					DispStamina(SomePlayer.GetStamina());
					cout << CR;
					DispScrolls(SomePlayer);

					cout << CR << "<font size=+2>Level: <b>";
					cout << Level << "</b>" << CR;

					cout << "</td></tr>";

					cout << "<tr><td colspan='2'>";
					cout << "<font size=1 color=#2F2FFF>" << UtopianDate();
					cout << "</td></tr>";

					cout << "</td></tr></table>";

					int Links = 0; // number of links on the current paragraph


					cout << "</td><td valign=top>";
					// description of location

					char tag[1024];
					char p_text[1024];
					bool last_was_link = false;
					bool display = true;

					long AfterFragment = 0; // paragraph fragment to start at
					// (each combat increments the current fragment by one)
					long CurrentFragment = 0;

					if (numItems >= 4) // has a fragment parameter
					{
						AfterFragment = atoi(formData[3]);
						if (numItems >= 5) // has gold subtract parameter
						{
							char cst[1024];
							sprintf(cst,"%d",(atoi(formData[4]) ^ 0xC01A66)/16);
							if ((atoi(cst) > 9999) || (atoi(cst) < 1))
							{
								SomePlayer.Pinned = time(NULL) + 3600;
							}
							else
							{
								SomePlayer.AddGold(-(atoi(cst)));
							}
							PutByGuid(formData[1],SomePlayer);
						}
					}
					
					bool Auto = true;
					int g_dice = 1;

				        int res;
				        MYSQL_RES *a_res;
					MYSQL_ROW a_row;
					char query[1024];
					char pdata[65536];
					char P[1024];

					//SomePlayer.GetParagraph(P);
					// UPDATE game_locations SET data=\"%s\",magic_disabled=%d,combat_disabled=%d,theft_disabled=%d,chat_disabled=%d WHERE id=%s
					sprintf(query,"SELECT data,magic_disabled,combat_disabled,theft_disabled,chat_disabled FROM game_locations WHERE id=%s",LastLink);
	
				        {
				                res = mysql_query(&my_connection,query);
				                if (!res)
				                {
				                        a_res = mysql_use_result(&my_connection);
				                        if (a_res)
				                        {
				                                a_row = mysql_fetch_row(a_res);
								{										
				                                        strcpy(pdata,a_row[0]);
									nomagic = atoi(a_row[1]);
									nocombat = atoi(a_row[2]);
									notheft = atoi(a_row[3]);
									nochat = atoi(a_row[4]);
								}
								mysql_free_result(a_res);
				                        }
				                }
				                else
				                {
				                        cout << "SELECT error: " << mysql_error(&my_connection);
							cout.flush();
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

							
						if (!strcmp(p_text,"<COMBAT"))
						{
							// combat tag
							Links++;
                                                        parafile >> p_text;

							while (p_text[strlen(p_text)-1] != '\"')
							{
				                                   char extra[256];
						                   parafile >> extra;
						                   strcat(p_text," ");
						                   strcat(p_text,extra);
						        }							
								

							char MonsterName[1024];
							ExtractValue(p_text,MonsterName);

								char MonsterSkill[1024];
								parafile >> p_text;
								ExtractValue(p_text,MonsterSkill);


								char MonsterStamina[1024];
								parafile >> p_text;
								ExtractValue(p_text,MonsterStamina);


								char MonsterArmour[1024];
								parafile >> p_text;
								ExtractValue(p_text,MonsterArmour);


								char MonsterWeapon[1024];
								parafile >> p_text;
								ExtractValue(p_text,MonsterWeapon);



								if (CurrentFragment == AfterFragment)
								{
									// when combat link is finished it goes back to the
									// paragraph it came from, but the next fragment of it.
									// fragments can only be requested on a paragraph
									// that contains at least one combat.
									cout << CR;
									strcpy(New,"action=combat&guid=");
									strcat(New,formData[1]);
									strcat(New,"&name=");
									strcat(New,AddEscapes(MonsterName));
									strcat(New,"&skill=");
									strcat(New,MonsterSkill);
									strcat(New,"&stamina=");
									strcat(New,MonsterStamina);
									strcat(New,"&armour=");
									strcat(New,MonsterArmour);
									strcat(New,"&weapon=");
									strcat(New,MonsterWeapon);
									strcat(New,"&return=");
									strcat(New,formData[2]);
									strcat(New,"&fragment=");
									char next_fragment[256];
									char tip[1024];
									itoa(CurrentFragment + 1,next_fragment,10);
									strcat(New,next_fragment);
									strcpy(tip,"Stamina: ");
									strcat(tip,MonsterStamina);
									strcat(tip," Skill: ");
									strcat(tip,MonsterSkill);
									strcat(tip," Armour: ");
									strcat(tip,MonsterArmour);
									strcat(tip," Weapon: ");
									strcat(tip,MonsterWeapon);
									ImageLink("combat.jpg",tip,New);
									cout << "<b>" << MonsterName << "</b>" << CR;
									Words++;
									break;
								}
								
								CurrentFragment++;
								continue;
							}
							
							if (CurrentFragment < AfterFragment)
							{
								// nothing should be displayed that comes before the
								// desired fragment!
								continue;
							}

							if (!strcmp(p_text,"<else>"))
							{
								// simply invert the display flag for anything inside an
								// <else> tag...
								display = !display;
								continue;
							}

							if (!strcmp(p_text,"<endif>"))
							{
								display = true;
								continue;
							}


							
							
							if (!strcmp(p_text,"<SNEAKTEST"))
							{

								// sneak test tag
								parafile >> p_text;

								while (p_text[strlen(p_text)-1] != '\"')
								{
									char extra[256];
									parafile >> extra;
									strcat(p_text," ");
									strcat(p_text,extra);
								}

								char MonsterName[1024];
								ExtractValue(p_text,MonsterName);

								long MonsterSneak;
								parafile >> p_text;
								ExtractValueNumber(p_text,MonsterSneak);

								cout << CR;
								Image("sneak.jpg","Sneak test");
								cout << "<b>" << MonsterName << "</b> <i>Sneak " << MonsterSneak << "</i>,";

								Auto = SomePlayer.SneakTest(MonsterSneak);

								if (Auto)
								{
									cout << " <b>PASSED</b>!" << CR;
								}
								else
								{
									cout << " <b>FAILED</b>!" << CR;
								}

								continue;
							}

							if (!strcmp(Lower(p_text),"<set"))
							{

								// set a state-flag
								parafile >> p_text;
								p_text[strlen(p_text)-1] = '\0';

								Lower(p_text);
								strcat(SomePlayer.GotFrom," gamestate_");
								strcat(SomePlayer.GotFrom,p_text);
								PutByGuid(formData[1],SomePlayer);
								continue;
							}

							if (!strcmp(Lower(p_text),"<setglobal"))
							{
								parafile >> p_text;
								p_text[strlen(p_text)-1] = '\0';

								Lower(p_text);
								MYSQL my_connection2;
			                                        // This requires a second connection otherwise we get "commands out of sync" :(
					                        mysql_init(&my_connection2);
					                        if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
					                        {
									sprintf(query,"INSERT INTO game_global_flags VALUES('%s')",p_text);
									mysql_query(&my_connection2,query);
								}
								mysql_close(&my_connection2);
					                                                             

							}

							if (!strcmp(Lower(p_text),"<unsetglobal"))
							{
								parafile >> p_text;
								p_text[strlen(p_text)-1] = '\0';

								Lower(p_text);
								MYSQL my_connection2;
								mysql_init(&my_connection2);
								if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
								{
									sprintf(query,"DELETE FROM game_global_flags WHERE flag='%s'",p_text);
									mysql_query(&my_connection2,query);
								}
								mysql_close(&my_connection2);
							}

							if (!strcmp(Lower(p_text),"<test"))
							{

								// test score tag
								parafile >> p_text;
								Lower(p_text);

								if (strstr(p_text,"luck>"))
								{
									cout << " Test your <u><b>luck</b></u>. ";
									Auto = SomePlayer.TestLuck();
									PutByGuid(formData[1],SomePlayer);
									continue;
								}

								if (strstr(p_text,"stamina>"))
								{
									cout << " Test your <u><b>stamina</b></u>. ";
									Auto = SomePlayer.TestStamina();
									PutByGuid(formData[1],SomePlayer);
									continue;
								}
								if (strstr(p_text,"skill>"))
								{
									cout << " Test your <u><b>skill</b></u>. ";
									Auto = SomePlayer.TestSkill();
									PutByGuid(formData[1],SomePlayer);
									continue;
								}
								if (strstr(p_text,"speed>"))
								{
									cout << " Test your <u><b>speed</b></u>. ";
									Auto = SomePlayer.TestSpeed();
									PutByGuid(formData[1],SomePlayer);
									continue;
								}
								if (strstr(p_text,"exp>"))
								{
									cout << " Test your <u><b>experience</b></u>. ";
									Auto = SomePlayer.TestExperience();
									PutByGuid(formData[1],SomePlayer);
									continue;
								}
							}

							
							
							
							
							if (!strcmp(p_text,"<time>"))
							{
								if (!didntmove)
								{
									// time advancement
									//SomePlayer.RemoveDay();
									SomePlayer.EatRation();
									PutByGuid(formData[1],SomePlayer);
								}
								continue;
							}

							if (!strcmp(p_text,"<eat>"))
							{
								if (!didntmove)
								{
									// just eat a ration, or loose stamina
									SomePlayer.EatRation();
									PutByGuid(formData[1],SomePlayer);
								}
								continue;
							}

                                                        if (!strcmp(p_text,"<dice>"))
                                                        {
								g_dice = Dice();
								continue;
							}
							if (!strcmp(p_text,"<d12>"))
							{
								g_dice = D12();
								continue;
							}
							if (!strcmp(p_text,"<2d6>"))
							{
								g_dice = twoD6();
								continue;
							}

							if (!strcmp(p_text,"<pick"))
							{

								// pick up free items (one-choice)
								parafile >> p_text;

								while (p_text[strlen(p_text)-1] != '\"')
								{
									char extra[256];
									parafile >> extra;
									strcat(p_text," ");
									strcat(p_text,extra);
								}

								char ItemName[1024];
								ExtractValue(p_text,ItemName);

								char ItemVal[1024];
								parafile >> p_text;
								ExtractValue(p_text,ItemVal);
								
								sprintf(New,"action=pick&guid=%s&item=%s&val=%s",formData[1],AddEscapes(ItemName),ItemVal);
								cout << CR;
								ImageLink("sneak.jpg",Describe(ItemVal,ItemName),New);
								cout << "<b> " << ItemName << CR;
							}




							if (!strcmp(p_text,"<if"))
							{
								char condition[15];

								parafile >> p_text;

								// -------------------------------------------------------
								//					<if item multi-word-item-name>
								// -------------------------------------------------------

								if (!strcmp(p_text,"item"))
								{

									parafile >> p_text;

									while (p_text[strlen(p_text)-1] != '>')
									{
										char extra[128];
										parafile >> extra;
										strcat(p_text," ");
										strcat(p_text,extra);
									}

									p_text[strlen(p_text)-1] = '\0';

									char Flags[1024];
									if ((SomePlayer.Spells.Get(p_text,Flags)) ||
									   (SomePlayer.Possessions.Get(p_text,Flags)) ||
									   (SomePlayer.Herbs.Get(p_text,Flags)))
									{
										display = true;
									}
									else
									{
										display = false;
									}

									continue;
								}


								// -------------------------------------------------------
								//					<if flag flagname>
								// -------------------------------------------------------

								if (!strcmp(p_text,"flag"))
								{
									char flag[1024];
									parafile >> p_text;
									p_text[strlen(p_text)-1] = '\0';
									strcpy(flag," gamestate_");
									strcat(flag,p_text);
									display = ((strstr(SomePlayer.GotFrom,flag) != 0) || (GlobalSet(p_text)));
									continue;
								}

								if (!strcmp(p_text,"!flag"))
								{
									char flag[1024];
									parafile >> p_text;
									p_text[strlen(p_text)-1] = '\0';
									strcpy(flag," gamestate_");
									strcat(flag,p_text);
									display = ((strstr(SomePlayer.GotFrom,flag) == 0) && (!GlobalSet(p_text)));
									continue;
								}


								// -------------------------------------------------------
								//					<if scorename gt|lt|eq value>
								// -------------------------------------------------------

								if (!strcmp(p_text,"exp"))
								{
									parafile >> condition;
									parafile >> p_text;
									long Q = SomePlayer.GetExperience();
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"dice"))
								{
									parafile >> condition;
									parafile >> p_text;
									display = comparison(condition,g_dice,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"stm"))
								{
									parafile >> condition;
									parafile >> p_text;
									long Q = SomePlayer.GetStamina();
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"skl"))
								{
									parafile >> condition;
									parafile >> p_text;
									long Q = SomePlayer.GetSkill();
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"arm"))
								{
									char S[1024];
									parafile >> condition;
									parafile >> p_text;
									long Q;
									SomePlayer.GetArmour(S,Q);
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"wpn"))
								{
									char S[1024];
									parafile >> condition;
									parafile >> p_text;
									long Q;
									SomePlayer.GetWeapon(S,Q);
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"day"))
								{
									char S[1024];
									parafile >> condition;
									parafile >> p_text;
									long Q = abs(14-SomePlayer.GetDays());
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"spd"))
								{
									parafile >> condition;
									parafile >> p_text;
									long Q = SomePlayer.GetSpeed();
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}

								if (!strcmp(p_text,"luck"))
								{
									parafile >> condition;
									parafile >> p_text;
									long Q = SomePlayer.GetLuck();
									display = comparison(condition,Q,p_text,g_dice);
									continue;
								}


								if (!strcmp(p_text,"race"))
								{
									// ------------------------------------------------------
									//						<if race x>
									// ------------------------------------------------------

									display = false; // if false, nothing displayed until an
													 // <endif> is reached.

									parafile >> p_text;
									if (!strcmp(p_text,"human>"))
									{
										if ((SomePlayer.GetRace() == Human) || (SomePlayer.GetRace() == Barbarian))
										{
											display = true;
											continue;
										}
									}

									if (!strcmp(p_text,"orc>"))
									{
										if ((SomePlayer.GetRace() == Orc) || (SomePlayer.GetRace() == Goblin))
										{
											display = true;
											continue;
										}
									}

									if (!strcmp(p_text,"elf>"))
									{
										if ((SomePlayer.GetRace() == Elf) || (SomePlayer.GetRace() == DarkElf))
										{
											display = true;
											continue;
										}
									}
									if (!strcmp(p_text,"dwarf>"))
									{
										if (SomePlayer.GetRace() == Dwarf)
										{
											display = true;
											continue;
										}
									}
									if (!strcmp(p_text,"lesserorc>"))
									{
										if (SomePlayer.GetRace() == LesserOrc)
										{
											display = true;
											continue;
										}
									}

								}



								if (!strcmp(p_text,"prof"))
								{
									// ------------------------------------------------------
									//						<if prof x>
									// ------------------------------------------------------

									display = false; // if false, nothing displayed until an
													 // <endif> is reached.

									parafile >> p_text;
									if (!strcmp(p_text,"warrior>"))
									{
										if ((SomePlayer.GetProfession() == Warrior) || (SomePlayer.GetProfession() == Mercenary))
										{
											display = true;
											continue;
										}
									}

									if (!strcmp(p_text,"wizard>"))
									{
										if (SomePlayer.GetProfession() == Wizard)
										{
											display = true;
											continue;
										}
									}

									if (!strcmp(p_text,"thief>"))
									{
										if ((SomePlayer.GetProfession() == Thief) || (SomePlayer.GetProfession() == Assassin))
										{
											display = true;
											continue;
										}
									}

									if (!strcmp(p_text,"woodsman>"))
									{
										if (SomePlayer.GetProfession() == Woodsman)
										{
											display = true;
											continue;
										}
								
									}


								// ---------------------------------------------------					
								}





							}

							if (!display)
							{
								continue;
							}
							if (!strcmp(p_text,"<br>"))
							{
								cout << CR;
								continue;
							}
							if (!strcmp(p_text,"<b>"))
							{
								cout << "<b>";
								continue;
							}
							if (!strcmp(p_text,"</b>"))
							{
								cout << "</b>";
								continue;
							}

							if (!strcmp(p_text,"<input")) // <input prompt="prompt" location="loc_id" value="correct_answer">
							{
	                                                        Links++;
	                                                        parafile >> p_text;
	                                                        while (p_text[strlen(p_text)-1] != '\"')
	                                                        {
									char extra[256];
									parafile >> extra;
									strcat(p_text," ");
									strcat(p_text,extra);
								}


								char Prompt[1024];
								ExtractValue(p_text,Prompt);
								char Para[1024];
								parafile >> p_text;
								ExtractValue(p_text,Para);
								char Correct[1024];
								parafile >> p_text;
								ExtractValue(p_text,Correct);
								cout << CR << CR;
								char Key[1024];
								LpToGuid(Key, Correct, Para);
								cout << "<b>" << Prompt << "</b><br><form action='" << me << "'><input type='hidden' name='action' value='riddle'><input type='hidden' name='guid' value='" << formData[1] << "'><input type='hidden' name='keycode' value='" << Key << "'><input type='text' name='q' value='""'><input type='hidden' name='p' value='" << Para << "'><input type='submit' value='Answer'></form>" << CR << CR;
								continue;
							}

							if (!strcmp(p_text,"<pickup"))
							{
								char item_name[1024], item_flags[1024], flags[1024];

								parafile >> p_text;

								if (!strcmp(p_text,"scroll>"))
								{
									if (NotGotYet(formData[2],"SCROLL",SomePlayer.GotFrom))
									{
										SomePlayer.AddScroll();
										strcat(SomePlayer.GotFrom," [");
										strcat(SomePlayer.GotFrom,"SCROLL");
										strcat(SomePlayer.GotFrom,formData[2]);
										strcat(SomePlayer.GotFrom,"]");	
									}
									PutByGuid(formData[1],SomePlayer);
									continue;
								}

								if (!strcmp(p_text,"gold"))
								{
									parafile >> p_text;
									p_text[strlen(p_text)-1] = '\0'; // remove last character, ">"
									SomePlayer.AddGold(atoi(p_text));
									PutByGuid(formData[1],SomePlayer);
									continue;
								}

								if (!strcmp(p_text,"silver"))
								{
									parafile >> p_text;
									p_text[strlen(p_text)-1] = '\0'; // remove last character, ">"
									SomePlayer.AddSilver(atoi(p_text));
									PutByGuid(formData[1],SomePlayer);
									continue;
								}

								strcpy(item_name,p_text);
								strcpy(item_flags,"[[none]]");

								while (p_text[strlen(p_text)-1] != '>')
								{
									parafile >> p_text;
									if (p_text[0] != '[')
									{
										strcat(item_name," ");
										strcat(item_name,p_text);
									}
									else
									{
										strcpy(item_flags,p_text);
										item_flags[strlen(item_flags)-1]='\0';
									}
								}
								
								if (item_name[strlen(item_name)-1] == '>')
								{
									item_name[strlen(item_name)-1]='\0';
								}

									// strip the [ and ] from the item flags...
									int q=0;
									for (unsigned int i=1; i<=(strlen(item_flags)-2); i++)
									{
										flags[q++] = item_flags[i];
									}

									flags[q] = '\0';

									if (!NotGotYet(formData[2],item_name,SomePlayer.GotFrom))
									{
										continue; // crafty player trying to get the same
									}			  // item twice! Not good if its unique!
									
									
									
									strcat(SomePlayer.GotFrom," [");
									strcat(SomePlayer.GotFrom,item_name);
									strcat(SomePlayer.GotFrom,formData[2]);
									strcat(SomePlayer.GotFrom,"]");	


									bool nabbed = false;
									if (!strcmp(flags,"SPELL"))
									{
										SomePlayer.Spells.Add(item_name,flags);
										nabbed = true;
									}	
	
									if (!strcmp(flags,"HERB"))
									{
										SomePlayer.Herbs.Add(item_name,flags);
										nabbed = true;
									}	

									if (!nabbed)
									{
										SomePlayer.Possessions.Add(item_name,flags);
									}

								PutByGuid(formData[1],SomePlayer);
								continue;

							}

							
							if (!strcmp(p_text,"<drop"))
							{
								char item[1024];
								parafile >> p_text;
								strcpy(item,p_text);
								while (p_text[strlen(p_text)-1] != '>')
								{
									parafile >> p_text;
									strcat(item," ");
									strcat(item,p_text);
								}
								item[strlen(item)-1] = '\0'; // remove '>'
								SomePlayer.Possessions.Delete(item);
								SomePlayer.Spells.Delete(item);
								SomePlayer.Herbs.Delete(item);
								PutByGuid(formData[1],SomePlayer);
								continue;
							}
							
							if (!strcmp(p_text,"<mod"))
							{
								parafile >> p_text;
								char anti_cheat_flag[256],mod[256];
								parafile >> mod;
								mod[strlen(mod)-1] = '\0'; // remove '>'
								long modifier = atoi(mod);
								
								strcpy(anti_cheat_flag,"MOD");
								strcat(anti_cheat_flag,p_text);
								strcat(anti_cheat_flag,mod);
	
								// No output if the player's been here before
								// (muahahaha aren't i mean?!)
								if (NotGotYet(formData[2],anti_cheat_flag,SomePlayer.GotFrom))
								{
									strcat(SomePlayer.GotFrom," [");
									strcat(SomePlayer.GotFrom,anti_cheat_flag);
									strcat(SomePlayer.GotFrom,formData[2]);
									strcat(SomePlayer.GotFrom,"]");
								}
								else
								{
									cout << "<b><i>Make no changes to your ";
									if (!strcmp(p_text,"stm")) cout << "stamina";
									if (!strcmp(p_text,"skl")) cout << "skill";
									if (!strcmp(p_text,"luck")) cout << "luck";
									if (!strcmp(p_text,"arm")) cout << "armour";
									if (!strcmp(p_text,"wpn")) cout << "weapon";
									if (!strcmp(p_text,"spd")) cout << "speed";
									if (!strcmp(p_text,"exp")) cout << "experience";
									cout << "</i></b> ";
									continue;
								}
								PutByGuid(formData[1],SomePlayer);


								if (!strcmp(p_text,"stm"))
								{
									SomePlayer.AddStamina(modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " stamina ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " stamina ";
									}
								}

								if (!strcmp(p_text,"skl"))
								{
									SomePlayer.AddSkill(modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " skill ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " skill ";
									}
								}

								if (!strcmp(p_text,"luck"))
								{
									SomePlayer.AddLuck(modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " luck ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " luck ";
									}
								}

								if (!strcmp(p_text,"exp"))
								{
									SomePlayer.AddExperience(modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " experience ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " experience ";
									}
								}

								if (!strcmp(p_text,"arm"))
								{
									char N[1024];
									long S;
									SomePlayer.GetArmour(N,S);
									SomePlayer.SetArmour(N,S+modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " armour ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " armour ";
									}
								}

								if (!strcmp(p_text,"wpn"))
								{
									char N[1024];
									long S;
									SomePlayer.GetWeapon(N,S);
									SomePlayer.SetWeapon(N,S+modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " weapon ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " weapon ";
									}
								}

								if (!strcmp(p_text,"spd"))
								{
									SomePlayer.AddSpeed(modifier);
									if (modifier < 1)
									{
										cout << "<b><i>Subtract " << abs(modifier) << " speed ";
									}
									else
									{
										cout << "<b><i>Add " << modifier << " speed ";
									}
								}

								cout << "</b></i>";
								PutByGuid(formData[1],SomePlayer);
								continue;

							}

							if (!strcmp(p_text,"<!--"))
							{
								while (strcmp(p_text,"--!>"))
								{
									// i just know there will be one neen who'll forget to terminate
									// their comments (hmmm, place your bets)
									if (parafile.eof())
										break;

									parafile >> p_text;
								}
								continue;
							}

							if (!strcmp(p_text,"<bank>"))
							{
								cout << CR << CR;
								cout << "<table><tr><td style='border-style:solid' border='1'>";
								cout << "Deposit gold pieces (10 gold minimum deposit): " << CR;
								cout << "<form>Amount: <input type='hidden' name='action' value='depositgold'><input type='hidden' name='guid' value='" << formData[1] << "'><input type='text' name='amount' value='10'><input type='submit' value='Deposit'></form>" << CR << CR;
								cout << "</td></tr>";
								cout << "<tr><td style='border-style:solid' border='1'>";
								cout << "Deposit a possession: " << CR << CR;
								ListPossessions(SomePlayer);
								cout << "</td></tr>";
								cout << "<tr><td style='border-style:solid' border='1'>";
								cout << "Withdraw something from my account:" << CR << CR;
								int gold_in_bank =  ConsolidateBankGold(SomePlayer);
								ListBankItems(SomePlayer);
								cout << "</td></tr>";
								cout << "</table>";
								cout << CR;
								continue;
							}


							if (!strcmp(p_text,"<i"))
							{
								// purchase item tag
								parafile >> p_text; // always: NAME="ItemName"
								char ItemName[1024];
								char Value[1024];
								char Cost[1024];
								strcpy(Value,"[none]");
								ExtractValue(p_text,ItemName);

								parafile >> p_text; // may be: VALUE="Flags" / COST="cost">

								while (!strstr(p_text,"="))
								{
									strcat(ItemName," ");
									strcat(ItemName, p_text);
									parafile >> p_text;
									if (ItemName[strlen(ItemName)-1] == '\"')
									{
										ItemName[strlen(ItemName)-1] = '\0';
									}
								}

								if (p_text[strlen(p_text)-1] != '>')
								{
									// process VALUE token
									ExtractValue(p_text,Value);
									parafile >> p_text; // read COST token that MUST now follow on
								}

								// process COST token here: COST="cost">
								ExtractValue(p_text,Cost);
								char orig[1024], ItemName2[2048], Value2[2048];
								Base64Encode(ItemName, ItemName2);
								Base64Encode(Value, Value2);
								strcpy(orig,Cost);
								sprintf(Cost,"%d",(atoi(Cost) ^ 31337) * 2);

								// display it all
								strcpy(New,"action=buy&guid=");
								strcat(New,formData[1]);
								strcat(New,"&item=");
								strcat(New,AddEscapes(ItemName2));
								strcat(New,"&cost=");
								strcat(New,Cost);
								strcat(New,"&flags=");
								strcat(New,Value2);
								strcat(New,"&return=");
								strcat(New,formData[2]);
								cout << CR;
								ImageLink("buy_item.jpg",Describe(Value,ItemName),New);
								cout << " <b><font color=" << ColourCode(Value) << ">" << ItemName << "</font></b> (<i>" << orig << " gold</i>)" << CR;
							}
							else
							{
							
								strncpy(tag,p_text,20);

								if ((strstr(tag,"<paylink=")) && (!last_was_link))
								{
									strcpy(New,"action=paragraph&guid=");
									strcat(New,formData[1]);
									strcat(New,"&p=");
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
									strcat(New,pnum);
									char orig[256];
									strcpy(orig,cst);
									sprintf(cst,"%d",(atoi(cst)*16) ^ 0xC01A66);
									if (SomePlayer.GetGold() < atoi(orig))
									{
										Image("link_bad.jpg","You dont have enough gold to do this!");
									}
									else
									{
										strcat(New,"&fragment=");
										char cf[256];
										itoa(CurrentFragment,cf,10);
										strcat(New,cf);
										strcat(New,"&subgold=");
										strcat(New,cst);
										ImageLink("link.jpg","Click to pay for this option...",New);
										Links++;
									}
									cout << " ";
								}
								else
								if ((strstr(tag,"<link=")) && (!last_was_link))
								{
									strcpy(New,"action=paragraph&guid=");
									strcat(New,formData[1]);
									strcat(New,"&p=");
									int i=0, t=0;
									while (p_text[i++] != '=');
									char pnum[256];
									pnum[0] = '\0';
									while (p_text[i] != '>')
									{
										pnum[t++] = p_text[i++];
									}
									pnum[t] = '\0';
									strcat(New,pnum);
									if ((SomePlayer.GetStamina() < 1) || (!strcmp(Lower(pnum),"the")))
									{
										Image("link_bad.jpg","Unable to select this option!");
									}
									else
									{
										ImageLink("link.jpg","Click to select this option...",New);
										Links++;
										strcpy(LastLink,pnum);
									}
									cout << " ";
								}
								else
								if ((strstr(Lower(tag),"<autolink=")) && (!last_was_link))
								{
									strcpy(New,"action=paragraph&guid=");
									strcat(New,formData[1]);
									strcat(New,"&p=");
									int i=0, t=0;
									while (p_text[i++] != '=');
									char pnum[256];
									pnum[0] = '\0';
									while (p_text[i] != '>')
									{
										pnum[t++] = p_text[i++];
									}
									pnum[t] = '\0';
									strcat(New,pnum);
									if (SomePlayer.GetStamina() < 1)
									{
										Image("link_bad.jpg","Unable to select this option!");
									}
									else
									{
										if (Auto)
										{
											ImageLink("link.jpg","Click to select this option...",New);
											Links++;
										}
										else
										{
											Image("link_bad.jpg","Cannot select this option!");
										}
									}
									Auto = !Auto; // invert the next autolink...

									cout << " ";
								}
								else
								{
									if (strstr(NeatVersion,">") == 0)
									{
										if (strlen(NeatVersion) > 0)
										{
											if (NeatVersion[0] != '<')
											{
												cout << NeatVersion << " ";
												Words++;
											}
										}
									}
								}
							}

						}

						cout.flush();

						// If we hit a paragraph where there are no links, or its impossible
						// to click ALL of the links, then the player must have hit some form
						// of death paragraph and must restart...
						if (!Links)
						{
							SomePlayer.SetStamina(0);
							PutByGuid(formData[1],SomePlayer);
						}

						if ((!Words) && (Links == 1))
						{
							cout << "<b>(Loading, please wait...)</b><meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=" << me << "?action=paragraph&guid=" << formData[1] << "&p=" << LastLink << "\">";
						}

						if (SomePlayer.IsDead())
						{
							cout << CR;
							HyperLink("You're dead! Click here to return to the main menu...","action=title");
							cout.flush();
							// clear all chat html, should stop refreshes etc
							char moo[1024];
							sprintf(moo,"DELETE FROM game_chat_events WHERE target_location='%s'",formData[1]);
							mysql_query(&my_connection,moo);
							SomePlayer.GetParagraph(moo);
							DropAllItems(formData[1],SomePlayer);
							PutByGuid(formData[1],SomePlayer);
						}


                                        	int n = 0;
                                	        //MYSQL_RES *a_res;
                        	                //MYSQL_ROW a_row;
                	                        //char query[1024];
        	                                
						SomePlayer.GetParagraph(P);

						cout.flush();
						
						//mysql_free_result(a_res);

	                                        sprintf(query,"SELECT DISTINCT item_desc FROM game_dropped_items WHERE location_id='%s'",P);
						res = mysql_query(&my_connection,query);						
						a_res = mysql_store_result(&my_connection);
						int q = mysql_affected_rows(&my_connection)-1;
						mysql_free_result(a_res);
						if (!SomePlayer.IsDead())
						{
                                                	res = mysql_query(&my_connection,query);
                                                	if (!res)
                                                	{
                                                        	a_res = mysql_use_result(&my_connection);
                                                        	if (a_res)
                                               		        {
                                               		                while (a_row = mysql_fetch_row(a_res))
                                                	                {
										if (!CurrentFragment)
										{
											if (!n)
											{
												cout << CR << CR << "The following items are on the floor here:" << CR;										
											}
											char hl[1024];
											sprintf(hl,"action=floorpick&guid=%s&item=%s&p=%s",formData[1],AddEscapes(a_row[0]),P);
											HyperLink(a_row[0],hl);
											cout.flush();

											if (n != q) cout << ",";
											n++;
											cout << " ";
										}
									}

                                                        	}
								mysql_free_result(a_res);
                                                	}
       	                                        	else
       	                                        	{
       	                                                 	cout << "SELECT error: " << mysql_error(&my_connection);
	                                                }
						}

						cout.flush();

						int x = 0;

						sprintf(query,"SELECT username,guid FROM game_users WHERE lastuse > UNIX_TIMESTAMP()-600 AND paragraph=%s AND username != \"%s\" AND username != '**NONE**' AND stamina > 0",P,SomePlayer.GetUsername());
						res = mysql_query(&my_connection,query);
						if (!res)
						{
							a_res = mysql_use_result(&my_connection);
							if (a_res)
							{
								while (a_row = mysql_fetch_row(a_res))
								{
									if (!CurrentFragment)
									{
										if (!SomePlayer.IsDead())
										{
											if (x == 0)
											{
												cout << CR << CR << "The following people are here with you:" << CR;
											}
											x++;
											char moot[1024];
									                sprintf(moot,"%s?action=playeropts&guid=%s&target=%s",me,formData[1],a_row[1]);
											cout << "<font color=yellow>";
											HyperLinkNew(a_row[0],moot);
											cout << "</font> ";
										}
									}
								}
							}
							mysql_free_result(a_res);
						}

						cout << "</td><td width=\"30%\" valign=top><center>";
						cout.flush();

						char z[256];
						SomePlayer.GetParagraph(z);

						cout << "<iframe ALLOWTRANSPARENCY='true' src='" << me << "?action=pack&guid=" << formData[1] << "' frameborder=0 scrolling=yes align=center width='229' height='230' name='backpack'>";
						cout << "Gack, no IFRAME tags! oh well!";
						cout << "</iframe>";
						/*SomePlayer.Possessions.Display(z,formData[1],SomePlayer.GetStamina*/
						cout.flush();

						cout << "</td></tr></table><center>";

						if (!SomePlayer.IsDead())
						{
							cout << "<iframe ALLOWTRANSPARENCY='true' src='" << me << "?action=chat&guid=" << formData[1] << "' width=550 height=160 frameborder=0 scrolling=no align=center>";
							cout << "Your client does not support IFRAME tags. Please upgrade to one that does to see location chat";
							cout << "</IFRAME>" << CR;
						}

						cout.flush();
			
						if (SomePlayer.GetStamina() > 0)
						{
							char V[1024];
							sprintf(V,"action=map&guid=%s",formData[1]);
							ImageLink("toolbar_map.gif","Click here to view the map",V);
							sprintf(V,"action=notes&guid=%s",formData[1]);
							ImageLink("toolbar_notes.gif","Click here to edit your personal notes",V);
							if (!nomagic)
							{
								sprintf(V,"action=herbsspells&guid=%s",formData[1]);
								ImageLink("toolbar_spells.gif","Click here to edit your herb and spell lists",V);
							}
							sprintf(V,"action=logout&guid=%s",formData[1]);
							ImageLink("toolbar_logout.gif","Click here to log out of the system",V);
						}
						else
						{
							Image("toolbar_map.gif","Click here to view the map");
							Image("toolbar_notes.gif","Click here to edit your personal notes");
							Image("toolbar_spells.gif","Click here to edit your herb and spell lists");
							Image("toolbar_logout.gif","Click here to log out of the system");
						}

						cout.flush();


						cout << "</html>";

						return 0;
				}
				if (!strcmp(formData[0],"riddle"))
				{
					Player P;
					GetPlayer(P,formData[1]);
					char Par[1024], Key[1024], Key2[1024];
					P.GetParagraph(Par);
					LpToGuid(Key, formData[3], formData[4]);
					LpToGuid(Key2, formData[4], formData[4]);
					if (!strcmp(Key,formData[2]))
					{
						// correct answer, keycode matches checksum of location and data
                                                sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],formData[4]);
						P.SetParagraph(formData[4]);
						PutByGuid(formData[1],P);
						RedirectTo(New);
						return 0;
																		
					}
					else
					{
						sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],Par);
						RedirectTo(New);
						return 0;
					}
				}
				if (!strcmp(formData[0],"chat"))
				{
					ContentType();
					HtmlStart_NoBanner(formData[1],false);
					cout << "<style>" << endl << "BODY {" << endl << "background-color: transparent;" << endl << "}" << endl << "</style>";
					cout << "<script event=onload for=window language='JavaScript'>document.talk.chatline.focus();</script>";
					cout << "<font size=1 color=yellow><center>Click a characters name in the chat to interact with them</center></font>";
					cout << "<iframe src='" << me << "?action=chattext&guid=" << formData[1] << "' width=548 height=98 frameborder=0 scrolling=no align=center>";
					cout << "</iframe>";
					cout << "<form name='talk' action='" << me << "' method='post' autocomplete='off'>";
					cout << "<input type='hidden' name='action' value='saytoroom'>";
					cout << "<input type='hidden' name='guid' value='" << formData[1] << "' style='border:solid red .25pt'>";
					cout << "<center>Say: <input type='text' name='chatline' size='55'></center></form>";
					return 0;
				}

				if (!strcmp(formData[0],"pack"))
				{
					ContentType();
					Player SomePlayer;
					GetPlayer(SomePlayer,formData[1]);
					HtmlStart_NoBanner(formData[1],false);
					cout << "<style>" << endl;
					cout << "	BODY{" << endl;
					cout << "		scrollbar-face-color:#000000;" << endl;
					cout << "		scrollbar-arrow-color:#000000;" << endl;
					cout << "		scrollbar-track-color:#000000;" << endl;
					cout << "		scrollbar-shadow-color:000000;" << endl;
					cout << "		scrollbar-highlight-color:000000;" << endl;
					cout << "		scrollbar-3dlight-color:000000;" << endl;
					cout << "		scrollbar-darkshadow-Color:000000;" << endl;
					cout << "		background-attachment : fixed;" << endl;
					cout << "		background-color: transparent;" << endl;
					cout << "	}" << endl;
					cout << "	</style>";
                                        char GW[1024],GA[1024];
                                        long R;
					char z[1024];
					SomePlayer.GetParagraph(z);
                                        SomePlayer.GetWeapon(GW,R);
                                        SomePlayer.GetArmour(GA,R);
                                        SomePlayer.Possessions.Display(z,formData[1],SomePlayer.GetStamina(),GW,GA);
					return 0;
				}

				if (!strcmp(formData[0],"playeropts"))
				{
					Player Source,Dest;
					char a[1024],b[1024];
					GetPlayer(Source,formData[1]);
					GetPlayer(Dest,formData[2]);
					ContentType();
					HtmlStart_NoBanner(formData[1],false,true);
					cout << "<center>";
					cout << "<b><u>Interact with player " << Dest.GetUsername() << "...</u></b>" << CR << CR;
					if (!strcmp(formData[1],formData[2]))
					{
						cout << "You cannot interact with <b>yourself</b>! Feeling a bit schizophrenic, " << Source.GetUsername() << "?";
						return 0;
					}
					if (Dest.GetStamina() < 1)
					{
						cout << "This player is dead! Maybe you should try an oujia board?";
						return 0;
					}
					cout << Dest.GetUsername() << " is a";
					if (Dest.GetStamina() <= 6)
					{
						cout << " weak ";
					}
					if ((Dest.GetStamina() >= 7) && (Dest.GetStamina() <= 20))
					{
						cout << "n average looking ";
					}
					if (Dest.GetStamina() >= 21)
					{
						cout << " powerfully built ";
					}
					GetPerms(nomagic,nocombat,notheft,nochat,Source);
					cout << Race(Dest.GetRace());
					cout << CR << CR;
					sprintf(a,"Give item to %s",Dest.GetUsername());
					sprintf(b,"action=giveitem&source=%s&dest=%s",formData[1],formData[2]);
					HyperLink(a,b);
					cout << CR;
					if (!notheft)
					{
                                        	sprintf(a,"Steal from %s",Dest.GetUsername());
                                        	sprintf(b,"action=stealitem&source=%s&dest=%s",formData[1],formData[2]);
                                        	HyperLink(a,b);
						cout << CR;
					}
					if (!nocombat)
					{
						sprintf(a,"Attack %s",Dest.GetUsername());
						sprintf(b,"action=pattack&source=%s&dest=%s&mustwait=1",formData[1],formData[2]);
						HyperLink(a,b);
						cout << CR;					
					}
					return 0;
				}
				if (!strcmp(formData[0],"pattack"))
				{
					Player Source,Dest;
					char a[1024],b[1024], Junk[1024];
					GetPlayer(Source,formData[1]);
					GetPlayer(Dest,formData[2]);
					ContentType();
					Source.GetParagraph(a);
					Dest.GetParagraph(b);
					if (strcmp(a,b))
					{
						HtmlStart_NoBanner(formData[1],false,true);
						cout << "<center>You cannot attack " << Dest.GetUsername() << " because they have run away! Curse that cowardly scum!" << CR << CR;
						HyperLinkExt("Close this window","javascript:close()");
						return 0;
					}

					GetPerms(nomagic,nocombat,notheft,nochat,Source);
					if (nocombat)
					{
						cout << "Error!";
						return 0;
					}

					if ((numItems == 4)) // mustwait=1 parameter
					{
                                        	if (Dest.LastUse < time(NULL)-60)
					        {
							HtmlStart_NoBanner(formData[1],false,true);
							cout << "<center>You cannot attack " << Dest.GetUsername() << " because they have been inactive for over 1 minute, which probably indicates they logged off before you challenged them and the combat would not be fair!" << CR << CR;
							HyperLinkExt("Close this window","javascript:close()");
							return 0;
						}
						if (Source.LastUse < time(NULL)-60)
						{
							HtmlStart_NoBanner(formData[1],false,true);
							cout << "<center>You cannot attack " << Dest.GetUsername() << " because you have not made any moves for over 1 minute, and are therefore marked inactive. Please do something (e.g. speak or manipulate items) before attempting to initiate combat, so that you do not have an unfair advantage!" << CR << CR;
							HyperLinkExt("Close this window","javascript:close()");
							return 0;
						}
					
						HtmlStart_Combat(formData[1],formData[2],true); // 10 second meta refresh
						cout << "<center>";
						cout << "<b><u>Attack " << Dest.GetUsername() << "...</u></b>" << CR << CR;
						sprintf(Junk,"/xw Glares at you menacingly and is about to attack you! To defend yourself and fight back, click their nickname in this line.");
						SayToUser(formData[1],formData[2],Junk);
						cout << "You have made your intentions clear that you wish to attack " << Dest.GetUsername() << ". You must now wait 15 seconds before you may attack this player.";
						Source.Spells.Add(formData[2],"SPELL");
						PutByGuid(formData[1],Source);
						cout << CR << CR << "This window will update when combat can procced, please wait...";
					}
					else
					{
						// *********************************************************************************
						// 
						HtmlStart_Combat(formData[1],formData[2],false); // 5 sec meta
						cout << "<center>";
						if ((Dest.GetStamina()>0) && (Source.GetStamina())>0)
						{
							cout << "<form action='" << me << "' method='post'>";
							cout << "<table><tr><td width='50%'><center>";
							cout << "<input type='hidden' name='action' value='pattack'>";
							cout << "<input type='hidden' name='source' value='" << formData[1] << "'>";
							cout << "<input type='hidden' name='dest' value='" << formData[2] << "'>";
							cout << "Stance:<br><select name=\"stance\" size=1><option value=\"O\">Offensive</option><option value=\"D\">Defensive</option></select><br>Attack type:<br><select name=\"cut\" size=\"1\"><option value=\"C\">Cutting</option><option value=\"P\">Piercing</option></select>" << CR;
						}

						bool hitting = false;
						int Target = 0;
						char BodyLoc[256];



                                                if (Dest.GetStamina() < 1)
                                                {
                                                        HtmlStart_NoBanner(formData[1],false,true);
                                                        cout << "<center>" << Dest.GetUsername() << " is dead!" << CR << CR;

		                                        long e = Dest.GetExperience();							
							long Level = 0;
							long j = 0;
							
							while ((e >= Levels[Level]) && (Level != 19)) Level++;
							Level--;
							if (Level > 2)
							{
								j = ((Levels[Level+1]-Levels[Level])/4);
							}
							else
							{
								j = 0;
							}

							if ((Dest.GetStamina() != -666) && (Dest.GetStamina() < 1))
							{
								Source.AddExperience(j);
								PutByGuid(formData[1],Source);
								Dest.SetStamina(-666);
								PutByGuid(formData[2],Dest);
							
								cout << Dest.GetUsername() << " was a level " << Level+1 << " player, with " << e << " experience points. <b>You gain " << j << " experience points</b> for your victory." << CR << CR;
                                                        	HyperLinkExt("Close this window","javascript:close()");
                                                        	InjectHTML(formData[2],"<script language='JavaScript'>parent.parent.location.reload(true);</script>");
							}
							else
							{
								cout << Dest.GetUsername() << " is dead!" << CR <<CR;
								HyperLinkExt("Close this window","javascript:close()");
							}


                                                        long NumDeaths = 0;
                                                        char DeathMessage[1024],DM[1024];
                                                        long DeathToUse = (rand() % DEATH_MESSAGES) + 1;
                                                        NumDeaths = 0;
                                                        ifstream Deaths("death.msg");
                                                        while (DeathToUse > NumDeaths)
                                                        {
                                                                Readline(Deaths,DeathMessage);
                                                                NumDeaths++;
                                                        }
                                                        Deaths.close();

                                                        snprintf(DM,1020,DeathMessage,Dest.GetUsername(),Source.GetUsername());
							char D2[1024];
							strcpy(D2,"/x2 ");
							strcat(D2,DM);
							SayToRoom(formData[1],D2);
                                                 
                                                        return 0;
                                                }
                                                if (Source.GetStamina() < 1)
                                                {
                                                        HtmlStart_NoBanner(formData[1],false,true);
                                                        cout << "<center>You're dead! Click here to close this window!" << CR << CR;
                                                        HyperLinkExt("Close this window","javascript:close()");
                                                        InjectHTML(formData[1],"<script language='JavaScript'>parent.parent.location.reload(true);</script>");
                                                        return 0;
                                                }

						// numItems = 3... standard number of items for no hit made
						// numItems = 7... standard number of items for hit made by clicking body icon
						if (numItems == 7)
						{
							if (Source.LastStrike > time(NULL)-5)
							{
								cout << "You cannot strike again yet!" << CR;
							}
							else
							{
								// calculate hit here
								hitting = true;
								Source.Strike();
								PutByGuid(formData[1],Source);
								// *** CONTACT POINTS ***
								// 0 = miss
								// 1 = 53,6 -> 87,36 : Head
								// 2 = 11,39 -> 49,109 : Left Arm
								// 3 = 84,39 -> 130,109 : Right Arm
								// 4 = 54,47 -> 78,75 : Body Centre
								// 5 = 51,91 -> 87,109 : Groin
								// 6 = 47,112 -> 63,200 : Left Leg
								// 7 = 68,112 -> 86,200 : Right Leg
								//
								int X = atoi(formData[5]), Y = atoi(formData[6]);

								if ((X>53) && (Y>6) && (X<87) && (Y<36))
									Target = 1;
								if ((X>11) && (Y>39) && (X<49) && (Y<109))
									Target = 2;
								if ((X>84) && (Y>39) && (X<130) && (Y<109))
									Target = 3;
								if ((X>39) && (Y>47) && (X<84) && (Y<112))
									Target = 4;
								if ((X>51) && (Y>91) && (X<87) && (Y<109))
									Target = 5;
								if ((X>47) && (Y>112) && (X<63) && (Y<200))
									Target = 6;
								if ((X>68) && (Y>112) && (X<86) && (Y<200))
									Target = 7;

								switch(Target)
								{
									case 0:
										strcpy(BodyLoc,"Miss");
									break;
									case 1:
										strcpy(BodyLoc,"Head");
									break;
									case 2:
										strcpy(BodyLoc,"Left Arm");
									break;
									case 3:
										strcpy(BodyLoc,"Right Arm");
									break;
									case 4:
										strcpy(BodyLoc,"Body");
									break;
									case 5:
										strcpy(BodyLoc,"Groin");
									break;
									case 6:
										strcpy(BodyLoc,"Left Leg");
									break;
									case 7:
										strcpy(BodyLoc,"Right Leg");
									break;
								}
								cout << CR;
							}
						}
						cout << CR << "<b><u>" << Source.GetUsername() << "</b></u>" << CR;
						cout << "Stamina: " << Source.GetStamina() << CR;
						cout << "Skill: " << Source.GetSkill() << CR;
						char D[1024],WD[1024];
						long R1,R2,E1,E2;
						Source.GetArmour(D,R1);
						cout << "Armour: " << R1 << CR;
						Source.GetWeapon(WD,R2);
						cout << "Weapon: " << R2 << CR << CR;
						cout << "<b><u>" << Dest.GetUsername() << "</b></u>" << CR;
						cout << "Stamina: " << Dest.GetStamina() << CR;
						cout << "Skill: " << Dest.GetSkill() << CR;
						Dest.GetArmour(D,E1);
						cout << "Armour: " << E1 << CR;
						Dest.GetWeapon(D,E2);
						cout << "Weapon: " << E2 << CR;
						
						cout << "</td><td>";
						cout << "<input type=\"image\" height='210' width='140' style=\"cursor:crosshair\" src=\"" << IMAGE_DIR << "/body.jpg\"><br>";

						if (hitting)
						{
							int SLoss = 0, KLoss = 0;
							if (Target)
							{
								if (!strcmp(formData[4],"C")) // cutting attack types
								{
									SLoss = 1; // base stamina damage
									KLoss = 0; // base skill damage
									
									if ((Target == 2) || (Target == 3)) // cutting attack to the arm
									{
										SLoss += 2;
										if (Target = 3) // assume players are right handed, skill loss for damage to right arm
										{
											KLoss += 1;
										}
									}

									if ((Target == 6) || (Target == 7)) // hits to leg may take off skill at random
									{
										SLoss+=1;
										if (Dice() > 3)
										{
											KLoss+=1;
										}
									}

									if (Target == 5) // hits to groin may take off a lot of skill, if youre lucky
									{
										SLoss += 1;
										if (Dice() == 6)
										{
											SLoss += Dice();
										}
									}

									if (Target == 1) // Hits to head always take a lot of stm
									{
										SLoss+=2;
										if (Dice() > 4) // theres a chance you could disorientate them and take skill
										{
											KLoss += (Dice() / 2);
										}
									}

									if (Target == 4) // body... big and easy to hit, nothing special for hitting it
									{
										SLoss+=1;
									}
								}
								else // piercing attack types
								{
									SLoss = 1;
									KLoss = 0;

									if ((Target == 2) || (Target == 3)) // stabbing someones arms may sever an atery
									{
										SLoss += 1;
										int foo = Dice();
										if (foo == 1)
										{
											SLoss += 3; // severed an artery
											KLoss += 2;
										}
										if (foo == 6)
										{
											KLoss += 2; // damage to hand
										}
									}

									if ((Target == 6) || (Target == 7)) // piercing attacks to leg, stm loss minimal but skl risk
									{
										SLoss = 2;
										if (Dice() > 3)
										KLoss += 2;
									}

									if (Target == 5) // stabbed in the groin. Major ouch.
									{
										SLoss+=3;
										KLoss+=2;
										if (Dice() > 4)
										{
											SLoss+=2;
											KLoss+=Dice();
										}
									}

									if (Target == 1) // stabbed in the head. not as good as cutting it.
									{
										KLoss+=1;
										KLoss = Dice();
									}

									if (Target == 4) // stabbed in body, quite a powerful attack (better than cutting anyway)
									{
										KLoss += Dice() / 2;
										SLoss += Dice() / 2;
									}
								}
								cout << "Hit to the <b>" << BodyLoc << "</b>!" << CR;
								if (Dice()+R2 > E1) // your weapon score effects your ability to pierce their armour
								{			   // (plus a random element)
									if (!strcmp(formData[3],"O"))
									{
										// an offensive attack causes more damage
										SLoss+=1;
										if (Dice() == 6)
										{
											KLoss+=1;
										}
										SLoss += R2 / 2;
									}
									if (SLoss) 
									{
										cout << "<b>" << SLoss << " points</b> Stamina loss" << CR;
									}
									if (KLoss)
									{
										cout << "<b>" << KLoss << " points</b> Skill loss" << CR;
									}

																		
									Dest.AddStamina(-SLoss);
									Dest.AddSkill(-KLoss);
									PutByGuid(formData[2],Dest);

									if ((SLoss) || (KLoss))
									{
										char m[1024];
										sprintf(m,"/m2 hits you in the %s with his %s for %d points stamina and %d points skill.",BodyLoc,WD,SLoss,KLoss);
										SayToUser(formData[1],formData[2],m);
									}
								}
								else
								{
									cout << "Enemy armour score of <b>" << E1 << "</b> blocked the attack!" << CR;
									char m[1024];
									sprintf(m,"/m2 hits you in the %s with his %s but your armour blocks the blow.",BodyLoc,WD);
									SayToUser(formData[1],formData[2],m);
								}
								// store all those owwies :)
								PutByGuid(formData[2],Dest);
							}
							else
							{
								cout << "You missed!";
							}
						}
						else
							cout << "Click an area to attack it.";
						cout << "</td></tr>";
						cout << "</table>";
						cout << "</form>";
						// if this display is result of a refresh, then dont update strike time
						// otherwise, strike time updates to present (time_t)
					}
					return 0;
				}
				if (!strcmp(formData[0],"giveitem"))
				{
					Player Source,Dest;
					char a[1024],b[1024];
					GetPlayer(Source,formData[1]);
					GetPlayer(Dest,formData[2]);
					ContentType();
					HtmlStart_NoBanner(formData[1],false,true);
					cout << "<center>";
					cout << "<b><u>Give item to " << Dest.GetUsername() << "...</u></b>" << CR << CR;
					for(int i = 0; i < 30; i++)
					{
						if (strcmp(Source.Possessions.List[i].Name,"[none]"))
						{
							sprintf(a,"action=giveto&source=%s&dest=%s&type=pos&idx=%d",formData[1],formData[2],i);
							HyperLink(Source.Possessions.List[i].Name,a);
							cout << CR;
						}
					}
					for(int i = 0; i < 30; i++)
					{
						if (strcmp(Source.Herbs.List[i].Name,"[none]"))
						{
							sprintf(a,"action=giveto&source=%s&dest=%s&type=herb&idx=%d",formData[1],formData[2],i);
							HyperLink(Source.Herbs.List[i].Name,a);
							cout << CR;
						}
					}
					cout << CR;
					cout << "<form action='" << me << "' method='post'>";
					cout << "<input type='hidden' name='action' value='giveto'>'";
					cout << "<input type='hidden' name='source' value='" << formData[1] << "'>";
					cout << "<input type='hidden' name='dest' value='" << formData[2] << "'>";
					cout << "<input type='hidden' name='type' value='gold'>";
					cout << "Gold pieces: <input type='text' name='idx' value='1'>";
					cout << " <input type='submit' value='Give Gold'>";
					cout << "</form>";
					cout << CR << CR;
					HyperLinkExt("Cancel","javascript:close();");
					return 0;
				}
				if (!strcmp(formData[0],"stealitem"))
				{
					Player Source,Dest;
					char a[1024],b[1024],P1[1024],P2[1024];
					GetPlayer(Source,formData[1]);
					GetPlayer(Dest,formData[2]);
					Source.GetParagraph(P1);
					Dest.GetParagraph(P2);
					ContentType();
					HtmlStart_NoBanner(formData[1],false,true);
					cout << "<center>";
					cout << "<b><u>Steal item from " << Dest.GetUsername() << "...</u></b>" << CR << CR;
					if (strcmp(P1,P2))
					{
						
						cout << Dest.GetUsername() << " has gone away from your location, you cannot steal from them!" << CR << CR;
						HyperLinkExt("Close","javascript:close();");
						return 0;
					}
                                        if (Dest.LastUse < time(NULL)-60)
					{
						cout << "<center>You cannot steal from " << Dest.GetUsername() << " because they have been inactive for over 1 minute, which probably indicates they logged off before you tried to steal from them or they are away from the computer!" << CR << CR;
						HyperLinkExt("Close this window","javascript:close()");
						return 0;
					}
					GetPerms(nomagic,nocombat,notheft,nochat,Source);
					if (notheft)
					{
						cout << "Error";
						return 0;
					}
					cout << "Your sneak: " << Source.GetSneak() << CR << Dest.GetUsername() << " sneak: " << Dest.GetSneak() << CR << CR;
					bool pass = Source.SneakTest(Dest.GetSneak());
					Source.AddSneak(-1);
					if (pass)
					{
						cout << "You have successfully stolen an item from " << Dest.GetUsername() << "!" << CR;
						ItemRecord a;
						bool luckydip = Dest.Possessions.RandomItem(a);
						if (luckydip)
						{
							Source.Possessions.Add(a.Name,a.FlagInfo);
							cout << "You have stolen: <b>" << a.Name << "</b>!";
						}
						else
						{
							if (Dest.GetGold())
							{
								int booty = Dice() + Dice() + Dice();
								if (booty > Dest.GetGold())
									booty = Dest.GetGold();
								Dest.AddGold(-booty);
								Source.AddGold(+booty);
								cout << "You have successfully lifted <b>" << booty << " gold pieces</b> from the purse of " << Dest.GetUsername() << ", as they had no items in their pack worth taking!";
							}
							else
							{
								cout << "It's not your lucky day! you stole from someone who has nothing worth stealing!";
							}
						}
						cout << CR << CR;
						PutByGuid(formData[1],Source);
						PutByGuid(formData[2],Dest);
						HyperLinkExt("Close","javascript:close();");
					}
					else
					{
						cout << "You failed to steal from " << Dest.GetUsername() << "! They will be informed that you attempted to steal from them!" << CR;
						char c[1024];
						sprintf(c,"/m2 dips his hand into your pack feeling for items to steal!");
						SayToUser(formData[1],formData[2],c);
						cout << CR << CR;
						PutByGuid(formData[1],Source);
						HyperLinkExt("Close","javascript:close();");
					}
					return 0;
				}

				if (!strcmp(formData[0],"giveto"))
				{
					Player Source,Dest;
					char a[1024],b[1024],c[1024];
					GetPlayer(Source,formData[1]);
					GetPlayer(Dest,formData[2]);
					Source.GetParagraph(a);
					Dest.GetParagraph(b);
					ContentType();
					HtmlStart_NoBanner(formData[1],false,true);
					cout << "<center>";
					cout << "<b><u>Give item to " << Dest.GetUsername() << "...</u></b>" << CR << CR;
					if (strcmp(a,b))
					{
						cout << "Sorry, but " << Dest.GetUsername() << " has already left your location, you cannot give him an item!" << CR << CR;
						HyperLinkExt("Close this window","javascript:close();");
						return 0;
					}
					if (!strcmp(formData[3],"gold"))
					{
						if (!atoi(formData[4]))
						{
							cout << CR;
							cout << "You must enter a number! Ya know, one of those things with the digits 0 through 9 in it!" << CR << CR;
							HyperLinkExt("Close this window","javascript:close();");
							return 0;
						}
						if (atoi(formData[4])>Source.GetGold())
						{
							cout << CR;
							cout << "You do not have this much gold!" << CR << CR;
							HyperLinkExt("Close this window","javascript:close();");
							return 0;
						}
						Source.AddGold(-(atoi(formData[4])));
						strcpy(a,"GOLD");
						strcpy(b,formData[4]);
					}
					else
					if (!strcmp(formData[3],"pos"))
					{
						strcpy(a,Source.Possessions.List[atoi(formData[4])].Name);
						strcpy(b,Source.Possessions.List[atoi(formData[4])].FlagInfo);
						strcpy(Source.Possessions.List[atoi(formData[4])].Name,"[none]");
						strcpy(Source.Possessions.List[atoi(formData[4])].FlagInfo,"[none]");
					}
					else
					{
						strcpy(a,Source.Herbs.List[atoi(formData[4])].Name);
						strcpy(b,Source.Herbs.List[atoi(formData[4])].FlagInfo);
						strcpy(Source.Herbs.List[atoi(formData[4])].Name,"[none]");
						strcpy(Source.Herbs.List[atoi(formData[4])].FlagInfo,"[none]");
					}
					if (!strcmp(formData[3],"gold"))
					{
						cout << "You have given <b>" << formData[4] << "</b> gold to " << Dest.GetUsername() << ". You will be informed if they accept this offer,";
					}
					else
					{
						cout << "The item \"" << a << "\" was offered to " << Dest.GetUsername() << ". If they turn down the item, it will be placed onto the floor. You will be notified in the chat window of their action.";
					}

					PutByGuid(formData[1],Source);
					
					char Par[1024];
					Source.GetParagraph(Par);
					sprintf(c,"/xx %s %s %s",Par,b,a);
					SayToUser(formData[1],formData[2],c);
					cout << CR;
					HyperLinkExt("Close this window","javascript:close();");
					return 0;
				}

				if (!strcmp(formData[0],"chattext"))
				{
					ContentType();
					HtmlStart_NoBanner(formData[1],true);
					//cout << "<style>" << endl << "BODY {" << endl << "background-color: transparent;" << endl << "}" << endl << "</style>";
					char query[1024];
					MYSQL my_conn2;
					MYSQL_ROW a_row;
					MYSQL_RES* a_res;
					Player P;
					int res;
					char Para[1024];
					GetPlayer(P,formData[1]);
					P.GetParagraph(Para);
					sprintf(query,"SELECT sender_guid,data FROM game_chat_events WHERE (target_location=\"%s\" OR target_location=\"%s\") AND time > %d ORDER BY TIME",Para,formData[1],time(NULL)-60);
 					mysql_init(&my_conn2);
					mysql_real_connect(&my_conn2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0);
					res = mysql_query(&my_conn2,query);
					if (!res)
					{
						a_res = mysql_use_result(&my_conn2);
						if (a_res)
						{
							while (a_row = mysql_fetch_row(a_res))
							{
								Player Source;
								char n[2048];
								strcpy(n,a_row[1]);
								if (!strncasecmp(n,"/me ",4))
								{
									GetPlayer(Source,a_row[0]);
									char* foom = n+4;
									char moot[1024];
									cout << "<font color=yellow>* ";
									sprintf(moot,"%s?action=playeropts&guid=%s&target=%s",me,formData[1],a_row[0]);
									HyperLinkNew(Source.GetUsername(),moot);
								        cout << " " << foom << CR << "</font>";
								}
								else
								if (!strncasecmp(n,"/m2 ",4))
								{
									GetPlayer(Source,a_row[0]);
									char* foom = n+4;
									cout << "<font color=#20FF20>* " << Source.GetUsername() << " " << foom << CR << "</font>";
								}
								else
								if (!strncasecmp(n,"/xw ",4))
								{
									char* foom = n+4;
									char moot[1024];
									GetPlayer(Source,a_row[0]);
									cout << "<b><font color=#20FF20>* ";
									sprintf(moot,"%s?action=pattack&guid=%s&target=%s",me,formData[1],a_row[0]);
									HyperLinkNew(Source.GetUsername(),moot);
									cout << " " << foom << CR << "</font></b>";
								}
								else
								if (!strncasecmp(n,"/x2 ",4))
								{
									char* foom = n+4;
									cout << "<font color=#20FF20>* " << foom << CR << "</font>";
								}
								else
								if (!strncasecmp(n,"/xx ",4))
								{
									GetPlayer(Source,a_row[0]);
									char* foom = n+4;
									std::stringstream data(foom);
									char itemflags[1024];
									char itemname[1024];
									char par[1024], Par[1024];
									char s[1024];
									char* in;

									P.GetParagraph(Par);

									strcpy(itemname,"");
									strcpy(itemflags,"");

									data >> par;
									data >> itemflags;
									if (!strcmp(itemflags,"ST"))
									{
										char bleh[1024];
										data >> bleh;
										strcat(itemflags,"+");
										strcat(itemflags,bleh);
									}
									if (!strcmp(itemflags,"SK"))
									{
										char bleh[1024];
										data >> bleh;
										strcat(itemflags,"+");
										strcat(itemflags,bleh);
									}
									while (!data.eof())
									{
										char blah[1024];
										data >> blah;
										strcat(itemname,blah);
										if (!data.eof())
										{
											strcat(itemname," ");
										}
									}
									if (!strcmp(Par,par))
									{
										in = AddEscapes(itemname);
										if (!strcmp(itemname,"GOLD"))
										{
											cout << "<font color=#20FF20>* " << Source.GetUsername() << " Offered you: <u>" << itemflags << " gold pieces</u> ";
										}
										else
										{
											cout << "<font color=#20FF20>* " << Source.GetUsername() << " Offered you: <u>" << itemname << "</u> ";
										}
										
										sprintf(s,"action=acceptitem&guid=%s&source=%s&itemname=%s&flags=%s",formData[1],a_row[0],in,itemflags);
										cout << "You can ";
										HyperLink("accept",s);
										cout << " the item, or ";
										sprintf(s,"action=denyitem&guid=%s&source=%s&itemname=%s&flags=%s",formData[1],a_row[0],in,itemflags);
										HyperLink("deny",s);
										cout << " it.";
										cout << CR << "</font>";
									}
								} 
								else
								{
									GetPlayer(Source,a_row[0]);
									char moot[1024];
									sprintf(moot,"%s?action=playeropts&guid=%s&target=%s",me,formData[1],a_row[0]);
									cout << "<b>&lt;</b><font color=yellow>";
									HyperLinkNew(Source.GetUsername(),moot);
								        cout << "</font><b>&gt;</b> " << n << CR;
								}
								cout << "<script language='JavaScript'>scroll(0,999999);</script>";
							}
						}
						mysql_free_result(a_res);
					}
					else
					{
						cout << mysql_error(&my_conn2);
					}
					mysql_close(&my_conn2);
					return 0;
				}

				if (!strcmp(formData[0],"acceptitem"))
				{
					Player P,Source;
					char s[1024], SP[256];
					MYSQL_RES* a_res;
					GetPlayer(P,formData[1]);
					GetPlayer(Source,formData[2]);
					Source.GetParagraph(SP);

					sprintf(s,"DELETE FROM game_chat_events WHERE data LIKE '%%%s %s%%' AND target_location=\"%s\"",formData[4],formData[3],formData[1]);
					mysql_query(&my_connection,s);
					a_res = mysql_use_result(&my_connection);
					if (!mysql_affected_rows(&my_connection))
					{
						ContentType();
						HtmlStart_NoBanner(formData[1],true,true);
						cout << "Oooh err!<br><br>This item wasn't even here, so i cant give it to you! Maybe youre trying to cheat and accept the item more than once?" << CR << "This page will automatically refresh back to chat in 10 seconds.";
						return 0;
					}
					mysql_free_result(a_res);
					
					if (!strcmp(formData[3],"GOLD"))
					{
						P.AddGold(atoi(formData[4]));
					}
					else
					if (!strcmp(formData[4],"HERB"))
					{
						P.Herbs.Add(formData[3],formData[4]);
					}
					else
					{
						P.Possessions.Add(formData[3],formData[4]);
					}
					PutByGuid(formData[1],P);
					SayToUser(formData[1],formData[2],"/m2 thanks you for the item and puts it in his backpack.");
					SayToUser(formData[2],formData[1],"/m2 hands you the item, and you place it into your backpack.");
					sprintf(s,"%s?action=chattext&guid=%s",me,formData[1]);
					RedirectTo(s);
					return 0;
				}

				if (!strcmp(formData[0],"saytoroom"))
				{
					char moo[1024];
					Player P;
					GetPlayer(P,formData[1]);
					GetPerms(nomagic,nocombat,notheft,nochat,P);
					if (((P.Muted) && (time(NULL) < P.Muted)) || (nochat))
					{
						// muted players cant speak until their muted time is up
					}
					else
					{
						if (!strncasecmp(formData[2],"/m2 ",4))
						{
							SayToRoom(formData[1],"/me would like to announce he is a nasty evil hacker!");
						}
						else
						{
							SayToRoom(formData[1],formData[2]);
						}
					}
					// redirect back to chat panel
					sprintf(moo,"%s?action=chat&guid=%s",me,formData[1]);
					RedirectTo(moo);
					return 0;
				}

				if (!strcmp(formData[0],"herbsspells"))
				{
					ContentType();
					HtmlStart();
					GetPlayer(SomePlayer,formData[1]);
					cout << "<b><u>Herbs and spells</u></b>" << CR << CR;
					cout << "<TABLE WIDTH=\"100%\"><TR><TD WIDTH=\"50%\" valign=top><b>Herbs</b></TD><TD><b>Spells</b></TD></TR>";
					cout << "<TR><TD valign=top>";
					HerbImg("hartleaf",SomePlayer);
					HerbImg("elfbane",SomePlayer);
					HerbImg("monkgrass",SomePlayer);
					HerbImg("fireseeds",SomePlayer);
					HerbImg("woodweed",SomePlayer);
					HerbImg("blidvines",SomePlayer);
					HerbImg("stickwart",SomePlayer);
					HerbImg("spikegrass",SomePlayer);
					HerbImg("hallucinogen",SomePlayer);
					HerbImg("wizardsivy",SomePlayer);
					HerbImg("orcweed",SomePlayer);

					cout << "</TD><TD valign=top>";

					SpellImgA("fire",SomePlayer);
					SpellImgA("water",SomePlayer);
					SpellImgA("light",SomePlayer);
					cout << CR;
					SpellImgA("fly",SomePlayer);
					SpellImgA("strength",SomePlayer);
					SpellImgA("x-ray",SomePlayer);
					cout << CR;
					SpellImgA("bolt",SomePlayer);
					SpellImgA("fasthands",SomePlayer);
					SpellImgA("thunderbolt",SomePlayer);
					cout << CR;
					SpellImgA("steal",SomePlayer);
					SpellImgA("shield",SomePlayer);
					SpellImgA("jump",SomePlayer);
					cout << CR;
					SpellImgA("open",SomePlayer);
					SpellImgA("spot",SomePlayer);
					SpellImgA("sneak",SomePlayer);
					cout << CR;
					SpellImgA("esp",SomePlayer);
					SpellImgA("run",SomePlayer);
					SpellImgA("invisible",SomePlayer);
					cout << CR;
					SpellImgA("shrink",SomePlayer);
					SpellImgA("grow",SomePlayer);
					SpellImgA("air",SomePlayer);
					cout << CR;
					SpellImgA("animalcommunication",SomePlayer);
					SpellImgA("weaponskill",SomePlayer);
					SpellImgA("healing",SomePlayer);
					cout << CR;
					SpellImgA("woodsmanship",SomePlayer);
					SpellImgA("nightvision",SomePlayer);
					SpellImgA("heateyes",SomePlayer);
					cout << CR;
					SpellImgA("decipher",SomePlayer);
					SpellImgA("detect",SomePlayer);
					SpellImgA("tracking",SomePlayer);
					cout << CR;
					SpellImgA("espsurge",SomePlayer);
					SpellImgA("afterimage",SomePlayer);
					SpellImgA("psychism",SomePlayer);
					cout << CR;
					SpellImgA("spiritwalk",SomePlayer);
					SpellImgA("growweapon",SomePlayer);
					cout << "</TD></TR></TABLE>" << CR << CR;

					char Name[1024],Flags[1024],WN[1024];
					long PWeapon;

					SomePlayer.GetWeapon(WN,PWeapon);

					if (!strcmp("Unknown Spell",SpExp(WN)))
					{
						cout << "<b>Current equipped spell/weapon: <u>" << WN << "</u> (Rating <u>" << PWeapon << "</u>)</b>" << CR;
					}
					else
					{
						cout << "<b>Current equipped spell/weapon: <u>" << SpExp(WN) << "</u> (Rating <u>" << PWeapon << "</u>)</b>" << CR;
					}


					long usable=0;
					for (int ii=0; ii<=29; ii++)
					{
						SomePlayer.Spells.GetItem(ii,Name,Flags);
						if (strcmp(Name,"[none]") != 0) // there's an item here...
						{
							// it's usable as a weapon if it has a combat rating and the
							// player is carrying some of its component herb so it can be
							// cast...
							if ((IsCombatSpell(Name)) && (HasComponentHerb(Name,SomePlayer)))
							{
								usable++;
							}
						}

					}
					if (!usable)
					{
						cout << "You have no combat-usable spells! Click ";
						char para[256];
						SomePlayer.GetParagraph(para);
						sprintf(New,"action=paragraph&guid=%s&p=%s",formData[1],para);
						HyperLink("here",New);
						cout << " to return to the previous page...";
						return 0;
					}

					cout << "Equip this spell ready for combat:" << CR;

					cout << "<form action=\"" << me << "\" method=get>";

					
					cout << "<input type=hidden name=action value=equipspell>";
					cout << "<input type=hidden name=guid value=" << formData[1] << ">";
					
					cout << "<select name=\"spell\" size=1>";

					for (int i=0; i<=29; i++)
					{
						SomePlayer.Spells.GetItem(i,Name,Flags);
						if (strcmp(Name,"[none]") != 0) // there's an item here...
						{
							// it's usable as a weapon if it has a combat rating and the
							// player is carrying some of its component herb so it can be
							// cast...
							if ((IsCombatSpell(Name)) && (HasComponentHerb(Name,SomePlayer)))
							{
								if (!strcmp(WN,Name))
								{
									cout << "<option selected value=\"" << Name << "\">" << SpExp(Name) << "</option>";
								}
								else
								{
									cout << "<option value=\"" << Name << "\">" << SpExp(Name) << "</option>";
								}
							}
						}
					}

					cout << "</select> <input type=submit value=\"Equip\">";
					cout << "</form>" << CR << CR;

                                        char para[256];
	                                SomePlayer.GetParagraph(para);
		                        sprintf(New,"action=paragraph&guid=%s&p=%s",formData[1],para);
			                HyperLink("Click here to return to game without equipping anything",New);			

					return 0;
				}

				if (!strcmp(formData[0],"equipspell"))
				{
					GetPlayer(SomePlayer,formData[1]);
					SomePlayer.SetWeapon(formData[2],GetSpellRating(formData[2]));
					char para[256];
					SomePlayer.GetParagraph(para);
					PutByGuid(formData[1],SomePlayer);

					sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],para);
					RedirectTo(New);

					return 0;
				}

				if (!strcmp(formData[0],"convert"))
				{
					GetPlayer(SomePlayer,formData[1]);

					if (SomePlayer.GetSilver() >= 2)
					{
						SomePlayer.AddSilver(-2);
						SomePlayer.AddGold(1);
					}

					PutByGuid(formData[1],SomePlayer);

					char v[256];
					SomePlayer.GetParagraph(v);
					sprintf(New,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],v);
					RedirectTo(New);
					return 0;
				}
				
				if (!strcmp(formData[0],"notes"))
				{
					ContentType();
					HtmlStart();
					GetPlayer(SomePlayer,formData[1]);
					cout << CR << "<b><u>Edit personal notes for character \"" << SomePlayer.GetUsername() << "\" </b></u>" << CR << CR;
					cout << "You can store any personal notes about your quest here. <b>Any personal notes you store will be erased if your character dies</b>." << CR << CR;
					cout << "<form action=\"" << me << "\" method=get>";
					cout << "<input type=hidden name=action value=savenotes>";
					cout << "<input type=hidden name=guid value=" << formData[1] << ">";
					cout << "<textarea rows=10 cols=50 name=newnotes>";
					for (int u=0; u<=SomePlayer.Notes.Count(); u++)
					{
						char Line[1024],Flags[1024];
						SomePlayer.Notes.GetItem(u,Line,Flags);
						if (strcmp(Line,"[none]"))
						{
							cout << Line << endl;
						}
					}
					cout << "</textarea>" << CR << CR;
					cout << "<input type=reset value=Clear onClick=\"OnClickHandler()\"> <input type=submit value=Save></form>";
					cout << "<script><!--" << endl;
					cout << "function OnClickHandler()" << endl;
					cout << "{" << endl;
					cout << "  alert('Your notes section has been erased. To undo this operation, click the back button on your browser and select the notes toolbar button again.')" << endl;
					cout << "}" << endl;
					cout << "--></script>";
					return 0;
				}

				if (!strcmp(formData[0],"logout"))
				{
					char data[1024];
					sprintf(data,"/m2 logs off and vanishes in a haze of smoke",formData[2]);
					SayToRoom(formData[1],data);
					char u[1024];
					sprintf(u,"%s?action=title",me);
					RedirectTo(u);
					return 0;
				}

				if (!strcmp(formData[0],"savenotes"))
				{
					GetPlayer(SomePlayer,formData[1]);
					SomePlayer.Notes.Clear();
					char current[1024];
					current[0] = '\0';
					int n = 0;
					if (!formData[2])
					{
						// erase notes section and save
						PutByGuid(formData[1],SomePlayer);
					}
					else
					{
						for (unsigned int y=0; y<=strlen(formData[2]); y++)
						{
							if ((formData[2][y] == '\x0D') || (formData[2][y] == '\x0A'))
							{
								// hit end of line
								current[n] = '\0';
								if (current[0] == '\0')
								{
									strcpy(current,"[none]");
								}
								SomePlayer.Notes.Add(current,"[none]");
								n = 0;
								current[0] = '\0';
							}
							else
							{
								current[n++] = formData[2][y];
							}
						}
					}

						
						
					current[n] = '\0';
					if (current[0] == '\0')
					{
						strcpy(current,"[none]");
					}
					SomePlayer.Notes.Add(current,"[none]");

					PutByGuid(formData[1],SomePlayer);
					char X[1024];
					char p[256];
					SomePlayer.GetParagraph(p);
					sprintf(X,"%s?action=paragraph&guid=%s&p=%s",me,formData[1],p);
					RedirectTo(X);
					return 0;
				}

				if (!strcmp(formData[0],"delherb"))
				{
						GetPlayer(SomePlayer, formData[1]);
						SomePlayer.Herbs.Delete(formData[2]);
						PutByGuid(formData[1],SomePlayer);

						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						return 0;
				}
				if (!strcmp(formData[0],"delspell"))
				{
						GetPlayer(SomePlayer, formData[1]);
						SomePlayer.Spells.Delete(formData[2]);
						PutByGuid(formData[1],SomePlayer);

						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						return 0;
				}
				if (!strcmp(formData[0],"addspell"))
				{
						GetPlayer(SomePlayer, formData[1]);
						int max_spells = 2;
						if (SomePlayer.GetProfession() == Wizard)
						{
							max_spells = 5;
						}
						if (SomePlayer.Spells.Count() < max_spells)
						{
							SomePlayer.Spells.Add(formData[2],"SPELL");
						}
						PutByGuid(formData[1],SomePlayer);

						strcpy(New,me);
						strcat(New,"?action=itempicker&guid=");
						strcat(New,formData[1]);
						RedirectTo(New);
						return 0;
				}
				if (!strcmp(formData[0],"itempicker"))
				{
						Player SomePlayer;;
						GetPlayer(SomePlayer, formData[1]);
						ContentType();
						HtmlStart();
						cout << "<b><u>Select spells and herbs</b></u>"<< CR << CR;
						cout << "Your character is not yet ready to enter the game. You must ";
						cout << "select some spells and herbs from the list below before you ";
						cout << "can enter the game - many spells require herbs to be cast. ";
						cout << "If a spell is displayed in red when you select it, you do not ";
						cout << "have the correct herbs needed to cast it. If a spell appears ";
						cout << "in yellow, you are able to cast the spell." << CR << CR;

						cout << "<a href=\"/grimoire.html\" target=\"_blank\">";
						cout << "<img src=\"" << IMAGE_DIR << "/spellbook.jpg\" alt=\"Click the spell book to open a window containing the Grimoire Utopius.\" border=0></a>" << CR << CR;
						

						cout << "<TABLE WIDTH=\"100%\"><TR><TD WIDTH=\"50%\" valign=top><b>Herbs</b></TD><TD><b>Spells</b></TD></TR>";
						cout << "<TR><TD valign=top>";
						HerbPick("hartleaf",SomePlayer);
						HerbPick("elfbane",SomePlayer);
						HerbPick("monkgrass",SomePlayer);
						HerbPick("fireseeds",SomePlayer);
						HerbPick("woodweed",SomePlayer);
						HerbPick("blidvines",SomePlayer);
						HerbPick("stickwart",SomePlayer);
						HerbPick("spikegrass",SomePlayer);
						HerbPick("hallucinogen",SomePlayer);
						HerbPick("wizardsivy",SomePlayer);
						HerbPick("orcweed",SomePlayer);

						cout << "</TD><TD valign=top>";

						SpellPick("fire",SomePlayer);
						SpellPick("water",SomePlayer);
						SpellPick("light",SomePlayer);
						cout << CR;
						SpellPick("fly",SomePlayer);
						SpellPick("strength",SomePlayer);
						SpellPick("x-ray",SomePlayer);
						cout << CR;
						SpellPick("bolt",SomePlayer);
						SpellPick("fasthands",SomePlayer);
						SpellPick("thunderbolt",SomePlayer);
						cout << CR;
						SpellPick("steal",SomePlayer);
						SpellPick("shield",SomePlayer);
						SpellPick("jump",SomePlayer);
						cout << CR;
						SpellPick("open",SomePlayer);
						SpellPick("spot",SomePlayer);
						SpellPick("sneak",SomePlayer);
						cout << CR;
						SpellPick("esp",SomePlayer);
						SpellPick("run",SomePlayer);
						SpellPick("invisible",SomePlayer);
						cout << CR;
						SpellPick("shrink",SomePlayer);
						SpellPick("grow",SomePlayer);
						SpellPick("air",SomePlayer);
						cout << CR;
						SpellPick("animalcommunication",SomePlayer);
						SpellPick("weaponskill",SomePlayer);
						SpellPick("healing",SomePlayer);
						cout << CR;
						SpellPick("woodsmanship",SomePlayer);
						SpellPick("nightvision",SomePlayer);
						SpellPick("heateyes",SomePlayer);
						cout << CR;
						SpellPick("decipher",SomePlayer);
						SpellPick("detect",SomePlayer);
						SpellPick("tracking",SomePlayer);
						cout << CR;
						SpellPick("espsurge",SomePlayer);
						SpellPick("afterimage",SomePlayer);
						SpellPick("psychism",SomePlayer);
						cout << CR;
						SpellPick("spiritwalk",SomePlayer);
						SpellPick("growweapon",SomePlayer);

						cout << "</TD></TR></TABLE>" << CR;

						cout << "Click <a href=\"";
						cout << me << "?action=savechoices&guid=" << formData[1];
						cout << "\">here</a> when you are ready to proceed..." << CR;
						
						return 0;
				}
			}

		}

		cout << "Status: 510 Internal non-fatal error.";
		cout << "Content-Type: text/plain\n\n";
		cout << "SSOD.CGI Error: parameter(s) missing or in incorrect order.\n\n";
		cout << "There are several possible causes for this error:\n\n";
		cout << "* The page you tried to open is not implemented, or,\n";
		cout << "* Your browser is sending requests in an unexpected format\n";

	}
	return 0;
}
