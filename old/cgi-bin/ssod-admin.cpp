// Project: SSOD
// Content: Admin Module Main File
//
// Thanks to amnesiac (Dave) for his help with the authentication code used in this module. :-)

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <netdb.h>
#include "admin.h"
#include "config.h"
#include "globals.h"
#include "cgi_utility.h"
#include "gameobjects.h"
#include "gameutility.h"
#include "html.h"
#include "levenshtein.h"

#include "mysql.h"
#include "mysql_config.h"

using namespace std;

char New[1024];		// used for redirection

void ShowLoggedInAdmins()
{
	DIR* dir;
	dirent* de;
	char admin_name[256];

	cout << CR << "Logged in administrators:" << CR;

	if (!strcmp(formName[0],"guid"))
	{
        	char FN[1024];
		dir = opendir(ADMIN_COOKIE_DIR);

		if (dir)
		{
			de = readdir(dir);
			while (de)
			{
				if ((strcmp(de->d_name,"..")) && (strcmp(de->d_name,".")))
				{
					sprintf(FN,"%s%s",ADMIN_COOKIE_DIR,de->d_name);

					struct stat buf;
					struct stat buf2;

					ofstream xfile("XXX");
		                        xfile << " ";
					xfile.close();

       	                		stat(FN,&buf);
       	        	        	stat("XXX",&buf2);

	       	                	time_t access_time = buf.st_mtime;
	                        	time_t now = buf2.st_mtime;

        	                	unlink("XXX");

              		          	if ((now-access_time)<=(60*60))
                	        	{
       	                	 		ifstream infile(FN);
						infile >> admin_name;
						cout << "<u> "<< admin_name << "</u> ";
						infile.close();
					}
				}
				de = readdir(dir);

			}
		}
	}
}




void GetCurrentUser(char *admin_name)
{
	char FN[1024];
	sprintf(FN,"%s%s",ADMIN_COOKIE_DIR,formData[0]);
	ifstream infile(FN);
	infile >> admin_name;
	infile.close();
}





int main()
{
	chdir("..");
	InitGameEngine();
	Player SomePlayer;

	if (numItems == 0)
	{
		AdminLogin();
		return 0;
	}
	else
	{
		if (numItems > 0)
		{
	
			if ((!strcmp(formName[0],"action")) || (!strcmp(formName[0],"guid")))
			{

				if (!strcmp(formData[0],"login"))
				{
				        MYSQL_ROW a_row;
				        int res;
				        MYSQL_RES *a_res;
					bool found_match = false;
					char User[1024],Pass[1024], GUID[1024], K[1024];

					strcpy(User,formData[1]);
					strcpy(Pass,formData[2]);
					
				        {
						char query[1024];
						sprintf(query,"SELECT * from game_admins where username='%s' and password=password('%s')",formData[1],formData[2]);
				                res = mysql_query(&my_connection,query);
				                if (!res)
				                {
				                        a_res = mysql_use_result(&my_connection);
				                        if (a_res)
				                        {
				                                while (a_row = mysql_fetch_row(a_res))
				                                {
				                                        found_match = true;
				                                }
				                        }
				                }
					}
					if (found_match)
					{
						strcat(User,"@");
						strcat(Pass,"@");
						strcat(Pass,getenv("REMOTE_ADDR"));
						strcat(User,getenv("REMOTE_ADDR"));
						LpToGuid(GUID,User,Pass);
						itoa(rand(),K,16);
						strcat(GUID,K);
						itoa(rand(),K,16);
						strcat(GUID,K); // random element for expiry
						sprintf(New,"%s?guid=%s&action=adminpage",me,GUID);
						RedirectTo(New);
						char FN[1024];
						strcpy(FN,ADMIN_COOKIE_DIR);
						strcat(FN,GUID);
						unlink(FN); // remove any previous uses
						ofstream Crap(FN);
						Crap << User;
						Crap.close();
						log("security tag GIVEN, logged in successfully as %s.",User);
						return 0;
					}
					else
					{
						log("security tag NOT GIVEN, user/password not matched in DB.");
						AdminLogin();
						return 0;
					}
				}


				// Anything below this line cannot be reached without a valid auth tag...

				if (numItems > 1)
				{
					if (!strcmp(formName[0],"guid"))
					{
						char FN[1024];
						strcpy(FN,ADMIN_COOKIE_DIR);
						strcat(FN,formData[0]);
						ifstream infile(FN);

						if (!infile.fail()) // tag exists, so check its time to see if its over 30 mins old
						{
							infile.close();
							struct stat buf;
							struct stat buf2;

							ofstream xfile("XXX");
							xfile << " ";
							xfile.close();

							stat(FN,&buf);
							stat("XXX",&buf2);

							time_t access_time = buf.st_mtime;
							time_t now = buf2.st_mtime;

							unlink("XXX");

							if ((now-access_time)<=(60*60))
							{
								// ok
							}
							else // tag is over 30 minutes old...
							{
								log("admin access DENIED, security tag has expired (age is %d seconds).",now-access_time);
								unlink(FN); // kill the invalid tag...
								AdminLogin();
								return 0;
							}
							
						}
						else
						{
							log("admin access DENIED, no security tag for this address.");
							AdminLogin();
							return 0;    // Tag number provided but doesnt exist in directory
						}
					}
				}
				else
				{

					// No tag supplied, exit...
					log("admin access DENIED, security tag was not provided with HTTP request.");
					AdminLogin();
					return 0;
				}



				if (!strcmp(formData[1],"adminpage"))
				{
					ContentType();
					HtmlStart();
					cout << "<center><b><u><font size=+1>Administration Page</font></b></u>" << CR << CR;
					sprintf(New,"guid=%s&action=addadmin",formData[0]);
					HyperLink("Add administrator",New);
					cout << CR;
					sprintf(New,"guid=%s&action=contenteditor",formData[0]);
					HyperLink("Edit Locations",New);
					cout << CR;
					sprintf(New,"guid=%s&action=findlocs",formData[0]);
					HyperLink("Find locations",New);
					cout << CR;
					sprintf(New,"guid=%s&action=fsearch",formData[0]);
					HyperLink("Check for a &lt;set&gt; flag",New);
					cout << CR;
                                        sprintf(New,"guid=%s&action=itemlist",formData[0]);
                                        HyperLink("Manage dropped items",New);
                                        cout << CR;
					sprintf(New,"guid=%s&action=nedit",formData[0]);
					HyperLink("Manage NPCs",New);
					cout << CR;
					sprintf(New,"guid=%s&action=babble",formData[0]);
					HyperLink("Manage NPC babble lists",New);
					cout << CR;
					sprintf(New,"guid=%s&action=descedit",formData[0]);
					HyperLink("Manage item descriptions",New);
					cout << CR;
					sprintf(New,"guid=%s&action=itemmap",formData[0]);
					HyperLink("View item map",New);
					cout << CR;
					sprintf(New,"guid=%s&action=cleartemp",formData[0]);
					HyperLink("Clear temporary character data",New);
					cout << CR;
					sprintf(New,"guid=%s&action=viewusers&lastuse=0&start=0&end=10",formData[0]);
					HyperLink("View user list",New);
					cout << CR;
					sprintf(New,"guid=%s&action=viewusers&lastuse=-1&start=0&end=10",formData[0]);
					HyperLink("View user list (Unvalidated only)",New);
					cout << CR;
					sprintf(New,"guid=%s&action=viewusers&lastuse=-2&start=0&end=10",formData[0]);
					HyperLink("View user list (Validated only)",New);
					cout << CR;
					sprintf(New,"guid=%s&action=viewusers&lastuse=-3&start=0&end=10",formData[0]);
					HyperLink("View user list (Inactive only)",New);
					cout << CR;
					sprintf(New,"guid=%s&action=viewlogs",formData[0]);
					HyperLink("View logs",New);
					cout << CR;
					sprintf(New,"guid=%s&action=chpass",formData[0]);
					HyperLink("Change password",New);
					cout << CR;
					sprintf(New,"guid=%s&action=logout",formData[0]);
					HyperLink("Log out",New);
					cout << CR << CR;
					cout << "Dropped items: <b>" << CountDroppedItems() << CR;
					cout << "</b>Pending chat events: <b>" << CountPendingChatEvents() << CR;
					cout << "</b>Admins: <b>" << CountAdmins() << CR;
					cout << "</b>Owned Items: <b>" << CountOwnedItems() << CR;
					cout << "</b>Banked Items: <b>" << CountBankItems() << CR;
					cout << "</b>" << CR;
					ShowLoggedInAdmins();
					return 0;
				}



				if (!strcmp(formData[1],"logout"))
				{
					// set the creation date of the security tag so that it's invalid (timed out)
					char FN[1024];
					sprintf(FN,"%s%s",ADMIN_COOKIE_DIR,formData[0]);
					unlink(FN);
					sprintf(New,"/cgi-bin/ssod.cgi"); // send them back to the main page...
					RedirectTo(New);
					return 0;
				}

				if (!strcmp(formData[1],"itemmap"))
				{
					RedirectTo("/images/item-map.jpg");
					return 0;
				}

				if (!strcmp(formData[1],"new"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>New administrator creation</b></u>" << CR << CR;
                                        int res;

                                        {
                                                char query[1024];
                                                sprintf(query,"INSERT INTO game_admins VALUES('','%s',password('%s'),'')",formData[2],formData[3]);
                                                res = mysql_query(&my_connection,query);
                                                if (!res)
                                                {
							cout << "New administrator created: " << formData[2];
							log("New administrator created: %s",formData[2]);
							return 0;
						}
						else
						{
							cout << "There was a mysql error when adding the new administrator.";
							return 0;
						}
					}
				}

				if (!strcmp(formData[1],"descedit"))
				{
					ContentType();
					HtmlStart();
					MYSQL_RES *myres;
					MYSQL_ROW row;
					cout << "<b><u>Edit item descriptions</b></u>" << CR << CR;
					int res;
					char query[1024];
					sprintf(query,"SELECT DISTINCT name FROM game_item_descs ORDER BY name");
					cout << "<form action='" << me << "' method='post'>";
					cout << "<input type='hidden' name='guid' value='" << formData[0] << "'>";
					cout << "<input type='hidden' name='action' value='editd'>";
					cout << "<select name='name'>";
					cout << "<option>(Add New Description)</option>";
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						myres = mysql_use_result(&my_connection);
						while (row = mysql_fetch_row(myres))
						{
							cout << "<option>" << row[0] << "</option>";
						}
						mysql_free_result(myres);
					}
					cout << "</select>";
					cout << "<input type='submit' value='Edit'>";
					cout << "</form>";
					return 0;
				}
				if (!strcmp(formData[1],"nedit"))
				{
					ContentType();
					HtmlStart();
					MYSQL_RES *myres;
					MYSQL_ROW row;
					cout << "<b><u>Edit NPCs</b></u>" << CR << CR;
					int res;
					char query[1024];
					sprintf(query,"SELECT DISTINCT username FROM game_npcs ORDER BY username");
					cout << "<form action='" << me << "' method='post'>";
					cout << "<input type='hidden' name='guid' value='" << formData[0] << "'>";
					cout << "<input type='hidden' name='action' value='editn'>";
					cout << "<select name='name'>";
					cout << "<option>(Add New NPC)</option>";
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						myres = mysql_use_result(&my_connection);
						while (row = mysql_fetch_row(myres))
						{
							cout << "<option>" << row[0] << "</option>";
							
						}
						mysql_free_result(myres);
					}
					cout << "</select>";
					cout << "<input type='submit' value='Edit'>";
					cout << "</form>";
					return 0;
					
					
				}
				if (!strcmp(formData[1],"editd"))
				{
					ContentType();
					HtmlStart();
					MYSQL_RES *myres;
					MYSQL_ROW row;
					cout << "<b><u>Edit item descriptions</b></u>" << CR << CR;
					int res;
					char query[1024];
					cout << "<table><tr><td width='25%'>";
					sprintf(query,"SELECT DISTINCT name FROM game_item_descs ORDER BY name");
					cout << "<form action='" << me << "' method='post'><input type='hidden' name='guid' value='" << formData[0] << "'><input type='hidden' name='action' value='editd'><select name='name'><option>(Add New Description)</option>";
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						myres = mysql_use_result(&my_connection);
						while (row = mysql_fetch_row(myres))
						{
							cout << "<option>" << row[0] << "</option>";
						}
						mysql_free_result(myres);
					}
					cout << "</select><br><input type='submit' value='Edit Another'></form>";
					cout << "</td><td width='75%'>";
					cout << "<form action='" << me << "' method='post'><input type='hidden' name='guid' value='" << formData[0] << "'><input type='hidden' name='action' value='saved'>";
					if (!strcmp(formData[2],"(Add New Description)"))
					{
						cout << "<textarea name='data' rows='20' cols='90'></textarea>";
					}
					else
					{
						sprintf(query,"SELECT idesc FROM game_item_descs WHERE name=\"%s\"",formData[2]);
						res = mysql_query(&my_connection,query);
						if (!res)
						{
							myres = mysql_use_result(&my_connection);
							while (row = mysql_fetch_row(myres))
							{
								cout << "<textarea name='data' rows='20' cols='90'>" << row[0] << "</textarea>" << CR;
							}
							mysql_free_result(myres);
						}
					}
					if (!strcmp(formData[2],"(Add New Description)"))
					{
						cout << CR << "<input type='text' name='item' value='Enter item name here' width='50'>" << CR;
						cout << "<input type='hidden' name='junk' value='1'>";
					}
					else
					{
						cout << "<input type='hidden' name='item' value='" << formData[2] << "'>";
					}
					cout << "<input type='submit' value='Save'></form>";

					cout << "</td></tr></table>";
					return 0;
				}
				if (!strcmp(formData[1],"editn"))
				{
					ContentType();
					HtmlStart();
					MYSQL_RES *myres;
					MYSQL_ROW row;
					cout << "<b><u>Edit NPCs</b></u>" << CR << CR;
					int res;
					char query[1024];
					cout << "<table><tr><td width='25%'>";
					sprintf(query,"SELECT DISTINCT username FROM game_npcs ORDER BY username");
					cout << "<form action='" << me << "' method='post'><input type='hidden' name='guid' value='" << formData[0] << "'><input type='hidden' name='action' value='editn'><select name='name'><option>(Add New NPC)</option>";
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						myres = mysql_use_result(&my_connection);
						while (row = mysql_fetch_row(myres))
						{
							cout << "<option>" << row[0] << "</option>";
							
						}
						mysql_free_result(myres);
					}
					cout << "</select><br><input type='submit' value='Edit Another'></form>";
					cout << "</td><td width='75%'>";
					cout << "<form action='" << me << "' method='post'><input type='hidden' name='guid' value='" << formData[0] << "'><input type='hidden' name='action' value='saven'>";
					if (!strcmp(formData[2],"(Add New NPC)"))
					{
						cout << CR << "NPC Name: <input type='text' name='name' value='Enter NPC name here' width='50'>" << CR;
						cout << "Stamina: <input type='text' name='stamina' value='12' width='50'>" << CR;
						cout << "Skill: <input type='text' name='skill' value='12' width='50'>" << CR;
						
					}
					else
					{
						sprintf(query,"SELECT DISTINCT id,stamina,skill,luck,sneak,speed,gold,experience,paragraph,armour,weapon,username,armour_rating,weapon_rating,laststrike,pinned,mana,manatick,can_resurrect,will_autoheal,aggressive,babble_list,def_stamina,def_skill,def_luck,def_sneak,def_speed,def_gold,def_mana FROM game_npcs WHERE username=\"%s\"",formData[2]);
						res = mysql_query(&my_connection,query);
						if (!res)
						{
							myres = mysql_use_result(&my_connection);
							while (row = mysql_fetch_row(myres))
							{
								cout << CR << "NPC Name: <input type='text' name='name' value='" << row[11] << "' width='50'>" << CR;
								cout << "Stamina: <input type='text' name='stamina' value='" << row[1] << "' width='50'>" << CR;
								cout << "Skill: <input type='text' name='skill' value='" << row[2] << "' width='50'>" << CR;
								
							}
							mysql_free_result(myres);
						}
					}
					cout << "<input type='submit' value='Save'></form>";
					cout << "</td></tr></table>";
					return 0;
				}
					
				if (!strcmp(formData[1],"saved"))
				{
					int res;
					char query[1024];
					
					if (numItems == 5)
					{
						// adding new item (INSERT)
						sprintf(query,"INSERT INTO game_item_descs VALUES(\"\",\"%s\",\"%s\")",formData[3],formData[2]);
					}
					else
					{
						// editing existing (UPDATE)
						sprintf(query,"UPDATE game_item_descs SET idesc=\"%s\" WHERE name=\"%s\"",formData[2],formData[3]);
					}
					res = mysql_query(&my_connection,query);
					if (res)
					{
						ContentType();
						HtmlStart();
						cout << "Error saving item description: " << mysql_error(&my_connection);
					}
					else
					{
						sprintf(query,"%s?guid=%s&action=descedit",me,formData[0]);
						RedirectTo(query);
					}
					return 0;
				}
				if (!strcmp(formData[1],"saven"))
				{
					return 0;
				}

				if (!strcmp(formData[1],"chpw"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Changing password of user " << formData[2] << "</b></u>" << CR << CR;
					int res;

					{
						char query[1024];
						sprintf(query,"UPDATE game_admins SET password=password('%s') WHERE username='%s' AND password=password('%s')",formData[4],formData[2],formData[3]);
						res = mysql_query(&my_connection,query);
						if (!res)
						{
							if (mysql_affected_rows(&my_connection))
							{
								cout << "Password changed for " << formData[2];
								log("%s changed their admin password",formData[2]);
								
							}
							else
							{
								cout << "Your old password was incorrect, or your username no longer exists on the system.";
								log("Incorrect password for %s while changing password",formData[2]);
							}
							return 0;
						}
						else
						{
							cout << "SQL UPDATE command failed. Please try again later.";
							log("Update failed while changing password for %s",formData[2]);
							return 0;
						}
					}
				}

				if (!strcmp(formData[1],"chpass"))
				{
					ChPass(formData[0]);
					return 0;
				}

				if (!strcmp(formData[1],"contenteditor"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Edit location content</b></u>" << CR << CR;
					cout << "<form action=\"" << me << "\" method=post>";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << formData[0]<< "\">";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"pedit\">";
					cout << "Loction ID: <INPUT TYPE=\"TEXT\" NAME=\"p\" VALUE=\"1\">" << CR;
				        cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Edit\">";
					cout << "</FORM>";
					return 0;		
				}

				if (!strcmp(formData[1],"babble"))
				{
                                        int res;
                                        MYSQL_RES *a_res;
                                        MYSQL_ROW a_row;
                                        char query[1024];
					ContentType();
					HtmlStart();

					cout << "<b><u>Edit NPC babble lists...</b></u>" << CR << CR;
					cout << "<form action='" << me << "' method='post'>";
					cout << "<input type='hidden' name='guid' value='" << formData[0] << "'>";
					cout << "<input type='hidden' name='action' value='bedit'>";
					cout << "Select a list to edit: <select>";
					sprintf(query,"SELECT DISTINCT id FROM game_babble_lists");
					cout << "<option>(Create New)</option>";
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						a_res = mysql_use_result(&my_connection);
						if (a_res)
						{
							while (a_row = mysql_fetch_row(a_res))
							{
								cout << "<option>" << a_row[0] << "</option>";
							}
						}
						mysql_free_result(a_res);
					}
					cout << "</select>" << CR << CR;
					cout << "<input type='submit' value='Edit'>";
					cout << "</form>";
					return 0;
				}

				if (!strcmp(formData[1],"pedit"))
				{
					char para[1024], data[1024];
					
					ContentType();
					HtmlStart();
					cout << "<b><u>Edit Location ID " << formData[2] << "</b></u>" << CR << CR;
					cout << "<form action=\"" << me << "\" method=post>";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << formData[0] << "\">";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"saveloc\">";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"p\" VALUE=\"" << formData[2] << "\">";
					cout << "<textarea rows=20 cols=90 name=pdata>";

                                        int res;
                                        MYSQL_RES *a_res;
                                        MYSQL_ROW a_row;
                                        char query[1024];
					char updater[1024];
                                        char pdata[65536];
					int combat_disabled,magic_disabled,theft_disabled,chat_disabled;

                                        sprintf(query,"SELECT data,combat_disabled,magic_disabled,theft_disabled,chat_disabled,updater FROM game_locations WHERE id=%s",formData[2]);
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
                                                                        if(mysql_field_count(&my_connection) == 0)
                                                                        {
                                                                                cout << "ERROR: SELECT returned no data!";
                                                                                return 0;
                                                                        }
                                                                        strcpy(pdata,a_row[0]);
									combat_disabled = atoi(a_row[1]);
									magic_disabled  = atoi(a_row[2]);
									theft_disabled  = atoi(a_row[3]);
									chat_disabled   = atoi(a_row[4]);
									strcpy(updater,a_row[5]);
                                                                }
                                                        }
                                                }
					}
						


					cout << pdata << endl;
					
					cout << "</textarea>" << CR << "<i>Last updated by: " << updater << "</i>" << CR <<CR;
					cout << "<table><tr>";
					if (magic_disabled)
						cout << "<td>Magic: <select name='magic'><option>Enabled</option><option selected>Disabled</option></select></td>";
					else
						cout << "<td>Magic: <select name='magic'><option selected>Enabled</option><option>Disabled</option></select></td>";
					
					if (combat_disabled)
						cout << "<td>Combat: <select name='combat'><option>Enabled</option><option selected>Disabled</option></select></td>";
					else
						cout << "<td>Combat: <select name='combat'><option selected>Enabled</option><option>Disabled</option></select></td>";
					
					if (theft_disabled)
						cout << "<td>Theft: <select name='theft'><option>Enabled</option><option selected>Disabled</option></select></td>";
					else
						cout << "<td>Theft: <select name='theft'><option selected>Enabled</option><option>Disabled</option></select></td>";
					
					if (chat_disabled)
						cout << "<td>Chat: <select name='chat'><option>Enabled</option><option selected>Disabled</option></select></td>";
					else
						cout << "<td>Chat: <select name='chat'><option selected>Enabled</option><option>Disabled</option></select></td>";
					
					cout << "<tr></table>" << CR;
					cout << "<INPUT TYPE=\"submit\" VALUE=\"Save Changes\">";
					cout << "</FORM>" << CR << CR;
					int prev = atoi(formData[2])-1;
					int next = atoi(formData[2])+1;
					if (prev < 1)
						prev = 1;
					sprintf(para,"guid=%s&action=pedit&p=%i",formData[0],prev);
					HyperLink("Prev << ",para);
					cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
					sprintf(para,"guid=%s&action=pedit&p=%i",formData[0],next);
					HyperLink(">> Next",para);
					cout << CR << CR;
					sprintf(para,"guid=%s&action=crossref&p=%s",formData[0],formData[2]);
					HyperLink("Show me what locations connect to this one",para);
					cout << CR;
                                        sprintf(para,"guid=%s&action=adminpage",formData[0]);
                                        HyperLink("Return to main menu",para);
					cout << CR << CR;

                                        cout << "<b>Edit another location</b>" << CR;
                                        cout << "<form action=\"" << me << "\" method=post>";
                                        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << formData[0]<< "\">";
                                        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"pedit\">";
                                        cout << "Loction ID: <INPUT TYPE=\"TEXT\" NAME=\"p\" VALUE=\"1\">" << CR;
                                        cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Edit\">";
                                        cout << "</FORM>";

					cout << CR << CR;
					

					strcpy(para,DOCUMENT_DIR);
					strcat(para,"/taghelp.html");
					ifstream paragraph2(para);
					while (!paragraph2.eof())
					{
						paragraph2 >> data;
						cout << data << " ";
					}
					paragraph2.close();
					log("%s loaded location %s",formData[0],formData[2]);

					return 0;
				}

				if (!strcmp(formData[1],"saveloc"))
				{
					ContentType();
					HtmlStart();

					cout << "<B><U>Saving location " << formData[2] << "</B></U>" << CR << CR;
                                        int res;
                                        char query[65536],outdata[65536], indata[65536];
					strcpy(indata,formData[3]);
					strcat(indata,"\n");
			  		mysql_real_escape_string(&my_connection,outdata, indata, strlen(indata));
					int disable_magic = 0,disable_combat = 0,disable_theft = 0,disable_chat = 0;
					if (!strcmp(formData[4],"Disabled"))
						disable_magic = 1;
					if (!strcmp(formData[5],"Disabled"))
						disable_combat = 1;
					if (!strcmp(formData[6],"Disabled"))
						disable_theft = 1;
					if (!strcmp(formData[7],"Disabled"))
						disable_chat = 1;
					char currusername[1024];
					GetCurrentUser(currusername);
					sprintf(query,"UPDATE game_locations SET updater='%s',data=\"%s\",magic_disabled=%d,combat_disabled=%d,theft_disabled=%d,chat_disabled=%d WHERE id=%s",currusername,outdata,disable_magic,disable_combat,disable_theft,disable_chat,formData[2]);
					res = mysql_query(&my_connection,query);
                                        if (res)
					{
						cout << "Error in mysql UPDATE command, please try again later.";
						return 0;
					}
					cout << strlen(indata) << " bytes saved successfully." << CR << CR;

                                        cout << "<b><u>Edit another location</b></u>" << CR << CR;
					cout << "<form action=\"" << me << "\" method=post>";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << formData[0]<< "\">";
					cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"pedit\">";
					cout << "Loction ID: <INPUT TYPE=\"TEXT\" NAME=\"p\" VALUE=\"1\">" << CR;
					cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Edit\">";
					cout << "</FORM>" << CR << CR;
									
					sprintf(query,"guid=%s&action=adminpage",formData[0]);
					HyperLink("Return to main menu",query);

					log("%s updated location %s",formData[0],formData[2]);

					return 0;
				}

				if (!strcmp(formData[1],"crossref"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Locations connected to ID number " << formData[2] << "</b></u>" << CR << CR;
					cout.flush();

				        cout << "Location " << formData[2] << " is linked from locations: ";
                                        MYSQL_ROW a_row;
                                        int res;
                                        MYSQL_RES *a_res;
					char query[1024];
					sprintf(query,"select id from game_locations where data like \"%%<AUTOLINK=%s>%%\" or data like \"%%<LINK=%s>%%\"",formData[2],formData[2]);
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

                                                                        while (field_count < mysql_field_count(&my_connection))
                                                                        {
										if (field_count == 0)
										{
											char junk[1024];
                                                                			sprintf(junk,"guid=%s&action=pedit&p=%s",formData[0],a_row[field_count]);
											HyperLink(a_row[field_count],junk);
											cout << " ";
										}
                                                                                field_count++;
                                                                        }
                                                                        a_row = mysql_fetch_row(a_res);
                                                                }
                                                        }
                                                }
                                        }
					log("%s cross-referenced location %s",formData[0],formData[2]);
					return 0;
				}
				if (!strcmp(formData[1],"findlocs"))
				{
					SearchPage(formData[0]);
					return 0;
				}
				
				if (!strcmp(formData[1],"fsearch"))
				{
					SearchForFlag(formData[0]);
					return 0;
				}
				if (!strcmp(formData[1],"itemlist"))
				{
					SearchForItems(formData[0]);
					return 0;
				}
				if (!strcmp(formData[1],"ilist"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Items at location " << formData[2] << "...</u></b>" << CR << CR;

                                        MYSQL_ROW a_row;
                                        int res;
                                        MYSQL_RES *a_res;
                                        char query[1024];
					sprintf(query,"SELECT DISTINCT item_desc,item_flags FROM game_dropped_items WHERE location_id=%s",formData[2]);
                                        res = mysql_query(&my_connection,query);
                                        if (!res)
                                        {
						cout << "<table border='1'>";
						cout << "<tr><td><b>Item Description</td><td><b>Item Flags</td></tr>";
                                                a_res = mysql_use_result(&my_connection);
                                                if (a_res)
                                                {
                                                        while (a_row = mysql_fetch_row(a_res))
							{
								cout << "<tr><td>" << a_row[0] << "</td><td>" << a_row[1] << "</td></tr>";
							}
						}
						cout << "</table>";
					}
					cout << CR;

                                        cout << "<b><u>Add an item to the floor</b></u>" << CR << CR;
                                        cout << "<form action=\"" << me << "\" method=post>";
                                        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << formData[0] << "\">";
                                        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"addfloor\">";
                                        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"p\" VALUE=\"" << formData[2] << "\">";
					cout << "<INPUT TYPE='TEXT' NAME='I1'>";
					cout << " <INPUT TYPE='TEXT' NAME='I2'> <INPUT TYPE='SUBMIT' VALUE='Add'>";
					cout << "</form>";

					cout << CR << CR;		
					sprintf(query,"guid=%s&action=adminpage",formData[0]);
					HyperLink("Return to main menu",query);
					cout << CR;
                                        sprintf(query,"guid=%s&action=zap&p=%s",formData[0],formData[2]);
					HyperLink("Clear this location of items",query);

					return 0;
				
				}
				if (!strcmp(formData[1],"addfloor"))
				{
					char query[1024];
					char flags[1024];
					strcpy(flags,formData[4]);
					if (!strcmp(flags,""))
						strcpy(flags,"[none]");

					sprintf(query,"INSERT INTO game_dropped_items VALUES('%s','%s','%s')",formData[2],formData[3],flags);
					mysql_query(&my_connection,query);
					
					sprintf(query,"%s?guid=%s&action=ilist&p=%s",me,formData[0],formData[2]);
					RedirectTo(query);
					return 0;
				}
				if (!strcmp(formData[1],"zap"))
				{
					ContentType();
					HtmlStart();
					char query[1024];
					cout << "<b><u>Clearing floor of items at location " << formData[2] << "...</b></u>" << CR << CR;
					sprintf(query,"DELETE FROM game_dropped_items WHERE location_id=%s",formData[2]);
					mysql_query(&my_connection,query);
					cout << mysql_affected_rows(&my_connection) << " floor item(s) cleared." << CR << CR;
					sprintf(query,"guid=%s&action=adminpage",formData[0]);
					HyperLink("Return to main menu",query);
					return 0;
				}
				if (!strcmp(formData[1],"findflag"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Searching for existing flags called \"" << formData[2] << "\"...</u></b>" << CR << CR;
					cout.flush();

					MYSQL_ROW a_row;
					int res;
					MYSQL_RES *a_res;
					char query[1024];
					sprintf(query,"SELECT id FROM game_locations WHERE data like \"%%<set %s>%%\" or data like \"%%<setglobal %s>%%\"",formData[2]);
					int found_flag = 0;
					
					res = mysql_query(&my_connection,query);
					if (!res)
					{
						a_res = mysql_use_result(&my_connection);
						if (a_res)
						{
							while (a_row = mysql_fetch_row(a_res))
							{
								found_flag = atoi(a_row[0]);
							}
						}
					}

					if (found_flag)
					{
						char h[1024];
						cout << "Found flag <b>" << formData[2] << "</b> at location " << found_flag << CR;
						sprintf(h,"guid=%s&action=pedit&p=%i",formData[0],found_flag);
						HyperLink("Edit this location",h);
						sprintf(h,"guid=%s&action=adminpage",formData[0]);
						cout << CR;
						HyperLink("Return to main menu",h);
					}
					else
					{
						char h[1024];
						cout << "The flag <b>" << formData[2] << "</b> does not exist.";
						sprintf(h,"guid=%s&action=adminpage",formData[0]);
						cout << CR;
						HyperLink("Return to main menu",h);
					}
	
					return 0;
				}
				if (!strcmp(formData[1],"find"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Locations containing the string \"" << formData[2] << "\"...</u></b>" << CR << CR;
					cout.flush();

					MYSQL_ROW a_row;
					int res;
					MYSQL_RES *a_res;
					char query[1024];
					sprintf(query,"SELECT id,SUBSTRING(data,1,30) FROM game_locations WHERE data like \"%%%s%%\" ORDER BY id LIMIT 0,40",formData[2]);

					res = mysql_query(&my_connection,query);
					if (!res)
					{
						cout << "<table border='1'>";
						cout << "<tr><td><b>Location ID</td><td><b>Description</td><td><b>Options</td></tr>";
						a_res = mysql_use_result(&my_connection);
						if (a_res)
						{
							while (a_row = mysql_fetch_row(a_res))
							{
								char h[1024];
								cout << "<tr><td>" << a_row[0] << "</td><td><i>" << a_row[1] << "...</i></td><td>";
								sprintf(h,"guid=%s&action=pedit&p=%s",formData[0],a_row[0]);
								HyperLink("Edit",h);
								cout << "</td></tr>";
							}
						}
						cout << "</table>" << CR << CR;
						cout << mysql_num_rows(a_res) << " matching locations.";
					}
					else
					{
						cout << "SELECT error on database: " << mysql_error(&my_connection);
					}
					return 0;
				}
				if (!strcmp(formData[1],"addadmin"))
				{
					CreateAdmin(formData[0]);
					return 0;
				}
	

				if (!strcmp(formData[1],"viewlogs"))
				{
					ContentType();
					HtmlStart();
					cout << "<b><u>Server Logs</b></u></center><pre>" << CR;
					
				        MYSQL_ROW a_row;
				        int res;
				        MYSQL_RES *a_res;
				   	{

				                res = mysql_query(&my_connection,"SELECT * from game_logs");

				                if (!res)
				                {
				                        a_res = mysql_use_result(&my_connection);
				                        if (a_res)
				                        {
								cout << "<center><table border='1' align='center'>";
				                                while (a_row = mysql_fetch_row(a_res))
				                                {
									cout << "<tr>";
				                                        unsigned int field_count;
                                				        field_count = 0;
				                                        while (field_count < mysql_field_count(&my_connection))
				                                        {
										cout << "<td>";
				                                                cout << a_row[field_count++];
										cout << "</td>";
				                                        }
				                                        cout << "</tr>";
				                                }
								cout << "</table></center>";
				                        }
				                }
				                else
				                {
				                        cout << "SELECT error on database:" << mysql_error(&my_connection);
				                }
				        }

					
					return 0;
				}
	

				if (!strcmp(formData[1],"viewusers"))
				{
					ContentType();
					HtmlStart();
					long usercount = CountUsers();
					cout << "<script language='JavaScript'>" << endl;
					cout << "function confirmSubmit()" << endl;
					cout << "{" << endl;
					cout << "	var agree=confirm(\"Are you sure you want to perform this operation on this character? The operation is not reversible!\");" << endl;
					cout << "	if (agree)" << endl;
					cout << "		return true;" << endl;
					cout << "	else" << endl;
					cout << "		return false;" << endl;
					cout << "}" << endl;
					cout << "</script>";
					
					cout << "<b><u>User list</b></u>" << CR << CR;
					if (atoi(formData[2]) >= 0)
					{
						cout << "<form action='" << me << "' method='get'>";
						cout << "<input type='hidden' name='guid' value='" << formData[0] << "'>";
						cout << "<input type='hidden' name='action' value='viewusers'>";
						cout << "Only show users active in the past <input type='text' name='lastuse' value='" << formData[2] << "'> seconds ";
						cout << "<input type='hidden' name='start' value='1'>";
						cout << "<input type='hidden' name='end' value='20'>";
						cout << "<input type='submit' value='Go!'>";
						cout << "</form>" << CR << CR;
					}
					long LastPageStart = atoi(formData[3]) - 20;
					long LastPageEnd = 10;
					long NextPageStart = atoi(formData[3]) + 10;
					long NextPageEnd = 10;
					if (LastPageStart < 0)
						LastPageStart = 0;
					if (NextPageStart+NextPageEnd > usercount)
						NextPageStart = usercount-10;
					cout << "<center>";
					cout << "<table width='80%'><tr><td align='left'>";
					cout << "<td><a href='" << me << "?guid=" << formData[0] << "&action=viewusers&lastuse=" << formData[2] << "&start=" << LastPageStart << "&end=" << LastPageEnd << "'>&lt;&lt; Last</a>";
					cout << "</td><td width='70%'><center>";
					cout << "Listing entries " << formData[3] << " - " << atoi(formData[3])+10 << "...";
					cout << "</td><td align='right'>";
					cout << "<a href='" << me << "?guid=" << formData[0] << "&action=viewusers&lastuse=" << formData[2] << "&start=" << NextPageStart << "&end=" << NextPageEnd << "'>Next &gt;&gt;</a>";
					cout << "</td></tr></table>";
					cout << CR;
					cout << "<table style='border-style:solid'>";
					cout << "<tr><td><b><u>Username</td>";
					cout << "<td><center><b><u>Info</td>";
					cout << "<td><center><b><u>E-Mail</td>";
					cout << "<td><b><u>Control</td></tr>";

					char filen[1024],usern[1024],query[1024];
                                        MYSQL_ROW a_row;
                                        int res, nfiles = 0;
                                        MYSQL_RES *a_res;
					MYSQL my_connection2;

					// This requires a second connection otherwise we get "commands out of sync" :(
					mysql_init(&my_connection2);
					if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
					{
						// Pick out all permenant, active users...
						if (atoi(formData[2]))
						{
							if ((atoi(formData[2])) == -1) // show unvalidated accts
							{
								sprintf(query,"SELECT guid from game_users WHERE username != '**NONE**' AND validated=0 ORDER BY username LIMIT %s,%s",formData[3],formData[4]);
							}
							else if ((atoi(formData[2])) == -2) // show validated accts only
							{
								sprintf(query,"SELECT guid from game_users WHERE username != '**NONE**' AND validated=1 ORDER BY username LIMIT %s,%s",formData[3],formData[4]);
							}
							else if ((atoi(formData[2])) == -3) // show inactive accounts only (not used for >1 month)
							{
								sprintf(query,"SELECT guid from game_users WHERE username != '**NONE**' AND lastuse < (UNIX_TIMESTAMP() - 2592000) ORDER BY username LIMIT %s,%s",formData[3],formData[4]);
							}
							else
							sprintf(query,"SELECT guid from game_users WHERE username != '**NONE**' AND lastuse > (UNIX_TIMESTAMP() - %d) ORDER BY username LIMIT %s,%s",atoi(formData[2]),formData[3],formData[4]);
						}
						else
						{
							sprintf(query,"SELECT guid from game_default_users ORDER BY username LIMIT %s,%s",formData[3],formData[4]);
						}
                                                res = mysql_query(&my_connection2,query);

                                                if (!res)
                                                {
                                                        a_res = mysql_use_result(&my_connection2);
                                                        if (a_res)
                                                        {
                                                                while (a_row = mysql_fetch_row(a_res))
                                                                {
									Player SomePlayer;
									cout << "<tr><td>";
									GetPlayer(SomePlayer,a_row[0]);

									cout << "<b>" << SomePlayer.GetUsername() << "</b>";

									cout << "</td><td>";

									if (strcmp(SomePlayer.GetUsername(),"**NONE**"))
									{
										cout << "<b>";
										cout << Race(SomePlayer.GetRace());
										cout << " ";
										cout << Profession(SomePlayer.GetProfession());
										cout << "</b> - Last visit: ";
										tm* timeinfo;
										char b[1024];
										timeinfo = localtime(&SomePlayer.LastUse);
										strcpy(b,asctime(timeinfo));
										b[strlen(b)-1] = '\0';
										cout << b;
                                                                                cout << "</td><td><a href='mailto:" << SomePlayer.Email << "'>";
										cout << "<font size='2'>" << SomePlayer.Email;
										cout << "</a>";			
									}
									else
									{
										cout << "<i>This character is probably corrupted somehow. Click reset to fix.</i></td><td>";
									}
									cout << "</td><td>";

									sprintf(New,"%s?guid=%s&action=reset&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "' onclick='return confirmSubmit()'><img src='" << IMAGE_DIR << "/reset.gif' border='0' alt='Reset User'></a> ";
									sprintf(New,"%s?guid=%s&action=delete&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "' onclick='return confirmSubmit()'><img src='" << IMAGE_DIR << "/delete.gif' border='0' alt='Delete Account'></a> ";
									sprintf(New,"%s?guid=%s&action=details&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "'><img src='" << IMAGE_DIR << "/details.gif' border='0' alt='Full Details'></a> ";

									sprintf(New,"%s?guid=%s&action=givedebug&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "' onclick='return confirmSubmit()'><img src='" << IMAGE_DIR << "/debug.gif' border='0' alt='Give Debug Privilages'></a> ";

									char c[1024];
									sprintf(c,"http://www.whois.sc/search/?bh=A&pool=C&rows=100&bc=n&exactfirst=y&q=%s\" target=\"_blank",SomePlayer.LastIP);
									ExternalImageLink("excl.jpg","Lookup WHOIS for abuse reporting",c);
	
									cout << "</td></tr>";
									nfiles++;
								}
							}
							mysql_free_result(a_res);
						}
						mysql_close(&my_connection2);

					cout << "</table>" << CR << CR << "<center>" << nfiles;
					cout << " user(s)." << CR << CR;
					}
                                        sprintf(query,"guid=%s&action=adminpage",formData[0]);
					HyperLink("Return to Main Menu",query);						
					return 0;
				}

				if (!strcmp(formData[1],"ld"))
				{
					ContentType();
					HtmlStart();
					Player P;
					GetPlayer(P,formData[2]);
					cout << "<b><u>Levenshtien Distance (Multi check)</b></u>" << CR << CR;
					cout << "Checking against original: <b>" << P.GetUsername() << "</b> E-Mail: <b>" << P.Email << "</b> IP: <b>" << P.LastIP << "</b>" << CR << CR;
                                        
                                        char filen[1024],usern[1024],query[1024];
                                        MYSQL_ROW a_row;
                                        int res, nfiles = 0;
                                        MYSQL_RES *a_res;
                                        MYSQL my_connection2;

					cout << "<table border='1'>";
					cout << "<tr><td>Username</td><td>Reason</td><td>E-Mail</td><td>Control</td></tr>";
                                        mysql_init(&my_connection2);
					mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0);			
					res = mysql_query(&my_connection2,"SELECT guid FROM game_users WHERE username != '**NONE**'");
                                        if (!res)
                                        {
						a_res = mysql_use_result(&my_connection2);
						if (a_res)
						{
							while (a_row = mysql_fetch_row(a_res))
							{
								Player P2;
								Distance D;
								GetPlayer(P2,a_row[0]);
								int Levenshtien = D.LD(P.Email,P2.Email);
								if ((Levenshtien < 4) || (!strcmp(P.LastIP,P2.LastIP)))
								{
									char New[1024], c[1024];
									cout << "<tr><td>";
									cout << P2.GetUsername();
									cout << "</td><td>";
									if (Levenshtien < 4)
									{
										cout << "Levenshtien Distance is " << Levenshtien;
									}
									else
									if (!strcmp(P.LastIP,P2.LastIP))
									{
										cout << "IP address " << P.LastIP << " identical";
									}
									cout << "</td><td>";
									cout << P2.Email;
									cout << "</td><td>";
                                                                        sprintf(New,"%s?guid=%s&action=reset&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "' onclick='return confirmSubmit()'><img src='" << IMAGE_DIR << "/reset.gif' border='0' alt='Reset User'></a> ";
									sprintf(New,"%s?guid=%s&action=delete&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "' onclick='return confirmSubmit()'><img src='" << IMAGE_DIR << "/delete.gif' border='0' alt='Delete Account'></a> ";
									sprintf(New,"%s?guid=%s&action=details&g2=%s",me,formData[0],a_row[0]);
									cout << "<a href='" << New << "'><img src='" << IMAGE_DIR << "/details.gif' border='0' alt='Full Details'></a> ";
                                                                        sprintf(c,"http://www.whois.sc/search/?bh=A&pool=C&rows=100&bc=n&exactfirst=y&q=%s\" target=\"_blank",P2.LastIP);
                                                                        ExternalImageLink("excl.jpg","Lookup WHOIS for abuse reporting",c);
									cout << "</td></tr>";
								}
							}
							mysql_free_result(a_res);
						}
					}
					cout << "</table>" << CR << CR;
					sprintf(query,"guid=%s&action=adminpage",formData[0]);
                                        HyperLink("Return to Main Menu",query);
					return 0;
				}

				if (!strcmp(formData[1],"reset"))
				{
					ContentType();
					HtmlStart();
					RestoreBackup(formData[2]);
					cout << "<b><u>Reset character</b></u>" << CR << CR << "Character was reset. Click your browser's back button to continue.";
					return 0;
				}
				if (!strcmp(formData[1],"delete"))
				{
					ContentType();
					HtmlStart();
					char query[1024];
					int res;
					cout << "<b><u>Deleting user " << formData[2] << " ...</b></u>" << CR << CR;
					GetPlayer(SomePlayer,formData[2]);
					sprintf(query,"DELETE FROM game_users WHERE guid='%s'",formData[2]);
					res = mysql_query(&my_connection,query);
					cout << "Deleted user data for "<< formData[2] << "..." << CR;
					sprintf(query,"DELETE FROM game_default_users WHERE guid='%s'",formData[2]);
					res = mysql_query(&my_connection,query);
					cout << "Deleted default user data for " << formData[2] << "..." << CR;
					sprintf(query,"DELETE FROM game_owned_items WHERE owner_guid='%s'",formData[2]);
					res = mysql_query(&my_connection,query);
					cout << "Deleted owned items of user " << formData[2] << "... " << CR;
					sprintf(query,"guid=%s&action=viewusers&lastuse=0start=0&end=10",formData[0]);
					cout << "<center>";
					HyperLink("Return to User List",query);
					cout << "</center>";
					return 0;
				}

				if (!strcmp(formData[1],"givedebug"))
				{
					ContentType();
					HtmlStart();
					char query[1024];
					int res;
					GetPlayer(SomePlayer,formData[2]);
					cout << "<b><u>Granting debug panel privilages to " << SomePlayer.GetUsername() << " ...</b></u>" << CR << CR;
					sprintf(query,"update game_default_users set gotfrom = concat(gotfrom,\" [DEBUG0]\") WHERE guid='%s'",formData[2]);
					res = mysql_query(&my_connection,query);
					if (res)
					{
						cout << "Failed to update game_default_users!" << CR;
					}
					else
					{
						cout << "Updated game_default_users table with new privilages" << CR;
					}
					sprintf(query,"update game_users set gotfrom = concat(gotfrom,\" [DEBUG0]\") WHERE guid='%s'",formData[2]);
					res = mysql_query(&my_connection,query);
					if (res)
					{
						cout << "Failed to update game_users!" << CR;
					}
					else
					{
						cout << "Updated game_users table with new privilages" << CR;
					}
					cout << "<center><br>Operation Completed<br><br>";
					sprintf(query,"guid=%s&action=viewusers&lastuse=0&start=0&end=10",formData[0]);
					HyperLink("Return to User List",query);
					cout << "</center>";
					return 0;
				}

				if (!strcmp(formData[1],"details"))
				{
					ContentType();
					HtmlStart();
					GetPlayer(SomePlayer,formData[2]);
					cout << "<b><u>Player details: " << SomePlayer.Username << "</u></b>" << CR << CR;
					cout << "<form action='" << me << "' method='post'>";
					cout << "<input type='hidden' name='guid' value='" << formData[0] << "'>";
					cout << "<input type='hidden' name='action' value='saveplayer'>";
					cout << "<input type='hidden' name='targetguid' value='" << formData[2] << "'>";
					
					cout << "<table>";

					cout << "<tr><td>Race:</td><td><b>" << Race(SomePlayer.GetRace()) << "</td><td>(Not editable)</td></tr>";
					cout << "<tr><td>Profession:</td><td><b>" << Profession(SomePlayer.GetProfession()) << "</td><td>(Not Editable)</td></tr>";
					cout << "<tr><td>Skill:</td><td><b>" << SomePlayer.GetSkill() << "</td><td><input type='text' name='skill' value='" << SomePlayer.GetSkill() << "'></td></tr>";
					cout << "<tr><td>Stamina:</td><td><b>" << SomePlayer.GetStamina() << "</td><td><input type='text' name='stamina' value='" << SomePlayer.GetStamina() << "'></td></tr>";
					cout << "<tr><td>Experience:</td><td><b>" << SomePlayer.GetExperience() << "</td><td><input type='text' name='exp' value='" << SomePlayer.GetExperience() << "'></td></tr>";
					cout << "<tr><td>Days:</td><td><b>" << SomePlayer.GetDays() << "</td><td><input type='text' name='days' value='" << SomePlayer.GetDays() << "'></td></tr>";
					cout << "<tr><td>Luck:</td><td><b>" << SomePlayer.GetLuck() << "</td><td><input type='text' name='luck' value='" << SomePlayer.GetLuck() << "'></td></tr>";
					cout << "<tr><td>Speed:</td><td><b>" << SomePlayer.GetSpeed() << "</td><td><input type='text' name='speed' value='" << SomePlayer.GetSpeed() << "'></td></tr>";
					cout << "<tr><td>Gold:</td><td><b>" << SomePlayer.GetGold() << "</td><td><input type='text' name='gold' value='" << SomePlayer.GetGold() << "'></td></tr>";
					cout << "<tr><td>Silver:</td><td><b>" << SomePlayer.GetSilver() << "</td><td><input type='text' name='silver' value='" << SomePlayer.GetSilver() << "'></td></tr>";
					cout << "<tr><td>Rations:</td><td><b>" << SomePlayer.GetRations() << "</td><td><input type='text' name='rations' value='" << SomePlayer.GetRations() << "'></td></tr>";
					cout << "<tr><td>Notoriety:</td><td><b>" << SomePlayer.GetNotoriety() << "</td><td><input type='text' name='notor' value='" << SomePlayer.GetNotoriety() << "'></td></tr>";
					cout << "<tr><td>Scrolls:</td><td><b>" << SomePlayer.GetScrolls() << "</td><td><input type='text' name='scrolls' value='" << SomePlayer.GetScrolls() << "'></td></tr>";
					cout << "<tr><td>Sneak:</td><td><b>" << SomePlayer.GetSneak() << "</td><td><input type='text' name='sneak' value='" << SomePlayer.GetSneak() << "'></td></tr>";
					cout << "<tr><td>Mana:</td><td><b>" << SomePlayer.Mana << "</td><td><input type='text' name='mana' value='" << SomePlayer.Mana << "'></td></tr>";
					char Para[1024];
					SomePlayer.GetParagraph(Para);
					cout << "<tr><td>Paragraph:</td><td><b>" << Para << "</td><td><input type='text' name='para' value='" << Para << "'></td></tr>";
					RatedItem R;
					char name[1024],name2[1024];
					long rating;
					SomePlayer.GetWeapon(name,rating);
					cout << "<tr><td>Weapon:</td><td><b>" << name << " (rating " << rating << ")</td></tr>";
					SomePlayer.GetArmour(name2,rating);
					cout << "<tr><td>Armour:</td><td><b>" << name2 << " (rating " << rating << ")</td></tr>";

                                        long Level = 0;
                                        while ((SomePlayer.GetExperience() >= Levels[Level]) && (Level != 20)) Level++;

					cout << "<tr><td>Level:</td><td><b>" << Level << "</td></tr>";

					cout << "</table>" << CR << CR;

					cout << "<center>";
					cout << "<table><tr><td><b>Possessions</td><td><b>Spells</b></td><td><b>Herbs</b></td></tr>";
					cout << "<tr><td>";
					SomePlayer.Possessions.Display(Para,formData[2],1,name,name2);
					cout << "</td><td>";
					SomePlayer.Spells.Display(Para,formData[2],1,name,name2);
					cout << "</td><td>";
					SomePlayer.Herbs.Display(Para,formData[2],1,name,name2);
					cout << "</td></tr></table>" << CR << CR;

					cout << "<table width=\"80%\"><tr><td><b>Flags</td></tr><tr><td>";
					cout << SomePlayer.GotFrom << CR << "<br><input type='text' name='gotfrom' value=\"" << SomePlayer.GotFrom << "\" size='100'>";
					
					cout << CR << CR << "<center>Last IP: <b>" << (SomePlayer.LastIP?SomePlayer.LastIP:"0.0.0.0");
					cout << "</b> (<a target='_blank' href='http://www.whois.sc/search/?bh=A&pool=C&rows=100&bc=n&exactfirst=y&q=" << SomePlayer.LastIP << "'>Lookup</a>)</center>" << CR << CR;
					cout << "<center>User E-Mail: <a href='mailto:" << SomePlayer.Email << "'>" << SomePlayer.Email << "</a> Validated: ";
					if (SomePlayer.Validated)
					{
						cout << "<b>Yes</b>";
					}
					else
					{
						cout << "<b>No</b> <a href='http://www.ssod.org/cgi-bin/ssod.cgi?action=activate&guid=" << formData[2] << "'>Force activation</a>";
					}
					cout << CR << CR << " <a href='" << me << "?guid=" << formData[0] << "&action=ld&g2="<< formData[2] << "'>Levenshtien Distance (Multi Check)</a>";
					cout << "</center>" << CR << CR;
					cout << "</td></tr></table>" << CR << CR;
					cout << "<input type='submit' value='Save Changes'>" << CR;
					cout << "</form>";
					char query[1024];
                                        sprintf(query,"guid=%s&action=viewusers&lastuse=0&start=0&end=10",formData[0]);
                                        HyperLink("Discard changes and return to user list",query);
					cout << CR << CR;
					return 0;
				}
				if (!strcmp(formData[1],"saveplayer"))
				{
					ContentType();
					HtmlStart();
					Player P;
					char query[1024];
					GetPlayer(P,formData[2]);
					P.SetSkill(atoi(formData[3]));
					P.SetStamina(atoi(formData[4]));
					P.SetExperience(atoi(formData[5]));
					P.SetDays(atoi(formData[6]));
					P.SetLuck(atoi(formData[7]));
					P.SetSpeed(atoi(formData[8]));
					P.SetGold(atoi(formData[9]));
					P.SetSilver(atoi(formData[10]));
					P.SetRations(atoi(formData[11]));
					P.SetNotoriety(atoi(formData[12]));
					P.SetScrolls(atoi(formData[13]));
					P.SetSneak(atoi(formData[14]));
					P.Mana = atoi(formData[15]);
					P.SetParagraph(formData[16]);
					strcpy(P.GotFrom,formData[17]);
					PutByGuid(formData[2],P);

					cout << "<b><u>Save New Player Stats</b></u>" << CR << CR;
					cout << "Save successful!" << CR << CR;
					
					sprintf(query,"guid=%s&action=viewusers&lastuse=0&start=0&end=10",formData[0]);
					HyperLink("Return to user list",query);
					return 0;
				}

				if (!strcmp(formData[1],"cleartemp"))
				{
					ContentType();
					HtmlStart();

					cout << "<b><u>Delete temporary character data</u></b>" << CR << CR;

                                        char filen[1024],usern[1024];
                                        MYSQL_ROW a_row;
                                        int res, nfiles = 0;
                                        MYSQL_RES *a_res;
                                        MYSQL my_connection2;

                                        // This requires a second connection otherwise we get "commands out of sync" :(
					mysql_init(&my_connection2);
					if (mysql_real_connect(&my_connection2, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0))
					{
						res = mysql_query(&my_connection2,"SELECT guid from game_users WHERE username = '**NONE**'");
						if (!res)
						{
							a_res = mysql_use_result(&my_connection2);
							if (a_res)
							{
								while (a_row = mysql_fetch_row(a_res))
								{
									char my_query[1024];
									cout << "Deleted temporary user backpack items: " << a_row[0] << CR;
									sprintf(my_query,"DELETE FROM game_owned_items WHERE owner_guid='%s'",a_row[0]);
									res = mysql_query(&my_connection,my_query);
								}
							}
							mysql_free_result(a_res);
						}
					}
					mysql_close(&my_connection2);
					res = mysql_query(&my_connection,"DELETE FROM game_users WHERE username='**NONE**'");
					if (!res)
					{
						cout << CR << CR << mysql_affected_rows(&my_connection) << " temporary users(s) removed.";
					}
					else
					{
						cout << CR << CR << "No temporary users removed.";
					}
					cout << CR << "Click your browser's back button to continue...";
					return 0;
				}


			}

		}

		cout << "Status: 510 Internal non-fatal error.";
		cout << "Content-Type: text/plain\n\n";
		cout << "SSOD-ADMIN.CGI Error: parameter(s) missing or in incorrect order.\n\n";
		cout << "There are several possible causes for this error:\n\n";
		cout << "* The page you tried to open is not implemented, or,\n";
		cout << "* Your browser is sending requests in an unexpected format\n";

	}
	return 0;
}

