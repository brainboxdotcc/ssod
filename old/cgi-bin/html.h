// Project: SSOD
// Content: HTML-level helper functions

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gd.h>
#include <gdfontl.h>

void ContentType()
{
	cout << "Content-Type: text/html\n\n";
}


void ContentTypeEnc()
{
	cout << "Content-Type: text/html Privacy-Enhancements: encrypt\n\n";
}


void RenderImage()
{
	cout << "Content-Type: image/png\n\n";
	gdImagePtr im;
	FILE *pngout;
	int black;
	int white;
	int blue;
	int red;
	int styleDotted[2];
	long xpos = 36, maxuserstoday = 1;

        int res;
        MYSQL_RES *a_res;
        MYSQL_ROW a_row;
        char query[1024];
        char pdata[65536];

	// SELECT max(users) FROM game_hourly_stats;
	res = mysql_query(&my_connection,"SELECT max(users) FROM game_hourly_stats");
	if (!res)
	{
		a_res = mysql_use_result(&my_connection);
		if (a_res)
		{
			while (a_row = mysql_fetch_row(a_res))
			{
				maxuserstoday = atoi(a_row[0]);
			}
		}
		mysql_free_result(a_res);
	}
	if (!maxuserstoday)
		maxuserstoday++;
	
	long multiplier = 113 / maxuserstoday;

	im = gdImageCreate((17*24)+(36*2),130);
	black = gdImageColorAllocate(im, 255, 255, 255);
	white = gdImageColorAllocate(im, 0, 0, 0);
	blue = gdImageColorAllocate(im, 0, 0, 255);
	red = gdImageColorAllocate(im, 255, 0, 0);

        styleDotted[0] = gdTransparent;
	styleDotted[1] = blue;
	styleDotted[2] = gdTransparent;
	
	gdImageRectangle(im,0,0,(17*24)+(36*2)-1,129,white);
		char Max[1024];
	sprintf(Max,"%d",maxuserstoday);
	gdImageString(im, gdFontLarge, 4, 3, (unsigned char*)Max, blue);
	gdImageSetStyle(im, styleDotted, 3);
	long checked_multiplier = multiplier;
	if (checked_multiplier < 4)
		checked_multiplier = 4;
	if (!checked_multiplier)
		checked_multiplier = 114;
	for (int qq = 115; qq > 0; qq-=checked_multiplier)
	{
		gdImageLine(im,36,qq,(17*24)+36,qq,gdStyled);
	}
	styleDotted[0] = blue;
	styleDotted[2] = blue;
	gdImageSetStyle(im, styleDotted, 3);

	sprintf(query,"SELECT users,hour FROM game_hourly_stats ORDER BY hour");
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		a_res = mysql_use_result(&my_connection);
		if (a_res)
		{
			while (a_row = mysql_fetch_row(a_res))
			{
				//gdImageFilledRectangle(im,xpos,115,xpos+14,115-(atoi(a_row[0])*multiplier),red);
				gdImageRectangle(im,xpos,115,xpos+15,115-(atoi(a_row[0])*multiplier),white);
				gdImageString(im, gdFontLarge, xpos, 115, (unsigned char*)a_row[1], blue);
				xpos+=17;
			}
		}
		mysql_free_result(a_res);
	}
	//pngout = fopen("/tmp/usersbyhour.png", "w");
	gdImagePng(im, stdout);
	//fclose(pngout);
	gdImageDestroy(im);

}

void HtmlStart()
{
	cout << "<html xmlns=\"http://www.w3.org/TR/REC-html40\">" << endl;
	cout << "<!-- Dynamic HTML page: Created by SSOD.CGI By C.J.Edwards -->" << endl;

	cout << "<style type='text/css'>" << endl;


                   int res;
                   MYSQL_RES *a_res;
                   MYSQL_ROW a_row;
                   char query[1024];
                   char pdata[65536];

                   sprintf(query,"SELECT data FROM game_css WHERE id=1");
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
                                                                   cout << "ERROR: SELECT returned no data, missing css";
                                                         }
                                                         while (field_count < mysql_field_count(&my_connection))
                                                         {
                                                                   strcpy(pdata,a_row[field_count++]);
                                                         }
                                                         a_row = mysql_fetch_row(a_res);
                                                  }
                                          }
					  mysql_free_result(a_res);
                                    }
                                    else
                                    {
                                          cout << "SELECT error: " << mysql_error(&my_connection);
                                    }
                       }
	cout << pdata;


	cout << "</STYLE>";
	cout << "<META NAME=\"Generator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	cout << "<META NAME=\"Originator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	cout << "<body bgcolor=black background=\"" << IMAGE_DIR << "/backdrop.jpg\" bgproperties='fixed'><title>The Seven Spells Of Destruction</title>";
	cout << "<center>";
	cout << "<a href='/'>";
	Image("title.gif","The Seven Spells Of Destruction");
	cout << "</a>";
	cout << CR;
}

void HtmlStart_NoBanner(char* GUID,bool dorefresh)
{
	cout << "<html xmlns=\"http://www.w3.org/TR/REC-html40\">" << endl;
	cout << "<!-- Dynamic HTML page: Created by SSOD.CGI By C.J.Edwards -->" << endl;
	cout << "<style type='text/css'>" << endl;
	int res;
	MYSQL_RES *a_res;
	MYSQL_ROW a_row;
	char query[1024];
	char pdata[65536];

	sprintf(query,"SELECT data FROM game_css WHERE id=1");
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		a_res = mysql_use_result(&my_connection);
		if (a_res)
		{
			while (a_row = mysql_fetch_row(a_res))
			{
				cout << a_row[0];
			}
			
		}
		mysql_free_result(a_res);
	}
	cout << "</STYLE>";
	cout << "<META NAME=\"Generator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	cout << "<META NAME=\"Originator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	if (dorefresh)
	{
		cout << "<META HTTP-EQUIV=\"refresh\" CONTENT=\"10; URL=" << me << "?action=chattext&guid=" << GUID << "\">";
		cout << "<body bgcolor='#1E0000'>";
	}
	else
	{
		cout << "<body><title>The Seven Spells Of Destruction</title>";
	}
}

void HtmlStart_NoBanner(char* GUID,bool dorefresh,bool wantbg)
{
	cout << "<html xmlns=\"http://www.w3.org/TR/REC-html40\">" << endl;
	cout << "<!-- Dynamic HTML page: Created by SSOD.CGI By C.J.Edwards -->" << endl;
	cout << "<style type='text/css'>" << endl;
	int res;
	MYSQL_RES *a_res;
	MYSQL_ROW a_row;
	char query[1024];
	char pdata[65536];

	sprintf(query,"SELECT data FROM game_css WHERE id=1");
	res = mysql_query(&my_connection,query);
	if (!res)
	{
		a_res = mysql_use_result(&my_connection);
		if (a_res)
		{
			while (a_row = mysql_fetch_row(a_res))
			{
				cout << a_row[0];
			}
		}
		mysql_free_result(a_res);
	}
	cout << "</STYLE>";
	cout << "<META NAME=\"Generator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	cout << "<META NAME=\"Originator\" CONTENT=\"SSOD.CGI By C.J.Edwards\">";
	if (dorefresh)
	{
		cout << "<META HTTP-EQUIV=\"refresh\" CONTENT=\"10; URL=" << me << "?action=chattext&guid=" << GUID << "\">";
		cout << "<body bgcolor=black background=\"" << IMAGE_DIR << "/backdrop.jpg\" bgproperties='fixed'><title>The Seven Spells Of Destruction</title>";
	}
	else
	{
		cout << "<body bgcolor=black background=\"" << IMAGE_DIR << "/backdrop.jpg\" bgproperties='fixed'><title>The Seven Spells Of Destruction</title>";
	}
}


char __box[1024];

char* Box(char* name)
{
	sprintf(__box,"<INPUT TYPE=\"text\" NAME=\"%s\" VALUE=\"0\">",name);
	return __box;
}

char* HBox(char* name, long value)
{
	sprintf(__box,"<INPUT TYPE=\"hidden\" NAME=\"%s\" VALUE=\"%d\">",name,value);
	return __box;
}

void PoweredBy()
{
	cout << CR << CR << CR << CR << CR << "<center><table width=22%><tr><center><td style='border:solid red .25pt'><center>";
	cout << "<font size=1 face=\"Arial\"><center>Powered by" << CR;
	ExternalImageLink("cryptlogo.gif\" width=\"140\" height=\"37","A Crypt Software Production","http://brainbox.cc");
	cout << CR << "<i><u>A</u>dventure <u>R</u>oleplay <u>W</u>eb <u>E</u>ngine</i>" << CR;
	cout << "Programmed by C.J.Edwards (BRAiN)";
	cout << "<script type='text/javascript'>\
\n\
		  var _gaq = _gaq || [];\n\
	  _gaq.push(['_setAccount', 'UA-22544571-1']);\n\
	    _gaq.push(['_trackPageview']);\n\
\n\
	      (function() {\n\
	           var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;\n\
		       ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';\n\
		           var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);\n\
			     })();\n\
\n\
	      </script>\
		";
	cout << "</td></tr></table></html>";
}

void TitlePage()
{
	ContentType();
	HtmlStart();
	Stats ServerStats;
	Player Newest;

        cout << "<script language='JavaScript'>" << endl;
        cout << "function confirmSubmit()" << endl;
        cout << "{" << endl;
        cout << "       var agree=confirm(\"Are you sure you want to clear your remembered password?\");" << endl;
        cout << "       if (agree)" << endl;
        cout << "               return true;" << endl;
        cout << "       else" << endl;
        cout << "               return false;" << endl;
        cout << "}" << endl << endl;
	cout << "function movepic(img_name,img_src)" << endl;
	cout << "{" << endl;
	cout << "	document[img_name].src='" << IMAGE_DIR << "/' + img_src;" << endl;
	cout << "}" << endl;
        cout << "</script>";

	GetPlayer(Newest,ServerStats.NewestPlayerGuid);
	
	cout << "<table width='635'><tr><td style='border:solid #800000 .25pt' background='" << IMAGE_DIR << "/titlebg.jpg'><center>";
	ImageLink("1.gif\" name='one' onmouseover=\"movepic('one','1a.gif')\" onmouseout=\"movepic('one','1.gif')","Create a new character","action=create");
	cout << CR;
	ImageLink("2.gif\" name='two' onmouseover=\"movepic('two','2a.gif')\" onmouseout=\"movepic('two','2.gif')","Continue your game with an existing character","action=login");
	cout << CR;
	ImageLink("3.gif\" name='three' onmouseover=\"movepic('three','3a.gif')\" onmouseout=\"movepic('three','3.gif')","View game information pages","action=info");
	cout << CR;
        ExternalImageLink("4.gif\" name='four' onmouseover=\"movepic('four','4a.gif')\" onmouseout=\"movepic('four','4.gif')","Go to the SSOD forums","/forums");
        cout << CR;
	ImageLink("5.gif\" name='five' onmouseover=\"movepic('five','5a.gif')\" onmouseout=\"movepic('five','5.gif')","View credits page","action=credits");
	cout << CR;
	ImageLink("6.gif\" name='six' onmouseover=\"movepic('six','6a.gif')\" onmouseout=\"movepic('six','6.gif')","Hall Of Fame","action=topten");
	cout << CR;
	ExternalImageLink("7.gif\" name='sev' onmouseover=\"movepic('sev','7a.gif')\" onmouseout=\"movepic('sev','7.gif')","Administrate the service (restricted access)","admin/ssod-admin.cgi");
	cout << "</td></tr></table>";
	cout << CR;
	cout << "<i><b>" << CountLogins() << "</b> out of a total <b>" << CountUsers() << "</b> users are currently logged into the game.</i>" << CR;
	if (CountLogins())
	{
		cout << "Recently active players: ";
		DispLogins();
	}
	
	if (strcmp(Newest.GetUsername(),"**NONE**"))
	{
		cout << CR << CR << "A warm welcome to our newest player: <b>" << Newest.GetUsername() << "</b><font size='-1'>" << CR << CR;
	}
	else
	{
		cout << CR << CR;
	}
	cout << "<table><tr><td style='border:solid #800000 .25pt'><center>";
	HyperLink("Clear my game cookies","action=forget\" onclick=\"return confirmSubmit()");
	cout << "<font color=#500000> | </font>";
	HyperLinkExt("Help availabe on our ","irc://irc.chatspike.net/ssod");
	HyperLinkExt("ircd","http://www.inspircd.org");
	cout << "<font color=#500000> | </font>";
	HyperLinkExt("A brainbox.cc Production","http://brainbox.cc/");
	cout << "</td></tr></table>\
		<script type='text/javascript'>\n\
\n\
		  var _gaq = _gaq || [];\n\
	  _gaq.push(['_setAccount', 'UA-22544571-1']);\n\
	    _gaq.push(['_trackPageview']);\n\
\n\
	      (function() {\n\
	           var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;\n\
		       ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';\n\
		           var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);\n\
			     })();\n\
\n\
	      </script>\
		\
		" << CR;
}

void StandardLogin()
{
	ContentType();
	HtmlStart();
	cout << "<form action=\"" << me << "\" method=get>";
	cout << "Please enter your <b>username and password</b> below to continue the game from your last visited point..." << CR << CR;
	cout << "Your username and password are <b>CASE SENSITIVE</b>" << CR << CR;
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"load\">";
	cout << "Username: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"username\">" << CR;
	cout << "Password: <INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password\">" << CR;
	cout << "Remember me:" << CR;
	cout << "Yes <INPUT type=\"radio\" name=\"remember\" value=\"1\"> No <INPUT type=\"radio\" name=\"remember\" value=\"0\" CHECKED>" << CR << CR;
	cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Log In\" ACTION=\"" << me << "\" method=post></center>";
	cout << "</FORM>";
	PoweredBy();
}

void SearchPage(char* GUID)
{
        ContentType();
	HtmlStart();
	cout << "<b><u>Search for matching locations</b></u>" << CR << CR;
	cout << "<form action=\"" << me << "\" method=post>";
	cout << "Please enter your <b>search terms</b> below:" << CR << CR;
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << GUID << "\">";
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"find\">";
	cout << "Find locations matching: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"terms\">" << CR << CR;
	cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Search\" ACTION=\"" << me << "\"></center>";
	cout << "</FORM>";
	PoweredBy();
										
}

void SearchForFlag(char* GUID)
{
        ContentType();
        HtmlStart();
        cout << "<b><u>Check Flag Exists</b></u>" << CR << CR;
        cout << "<form action=\"" << me << "\" method=get>";
        cout << "Please enter the <b>name of the flag</b>:" << CR << CR;
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << GUID << "\">";
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"findflag\">";
        cout << "Flag name: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"terms\">" << CR << CR;
        cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Search\" ACTION=\"" << me << "\"></center>";
        cout << "</FORM>";
        PoweredBy();
}

void SearchForItems(char* GUID)
{
	ContentType();
	HtmlStart();
        cout << "<b><u>List items at a location</b></u>" << CR << CR;
        cout << "<form action=\"" << me << "\" method=get>";
        cout << "Please enter the <b>Location ID</b> to list:" << CR << CR;
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << GUID << "\">";
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"ilist\">";
        cout << "Location ID: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"terms\">" << CR << CR;
        cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Search\" ACTION=\"" << me << "\"></center>";
        cout << "</FORM>";
        PoweredBy();
}


void AdminLogin()
{
	ContentType();
	HtmlStart();

	cout << "<form action=\"" << me << "\" method=post>";
	cout << "Please enter your <b>Administrator login name and password</b>:" << CR << CR;
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"login\">";
	cout << "Username: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"username\">" << CR;
	cout << "Password: <INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password\">" << CR << CR;
	cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Log In\" ACTION=\"" << me << "\" method=post></center>";
	cout << "</FORM>";
	cout << CR;
	cout << CR << "<center><table width=\"33%\"><tr><center><td style='border:solid red .25pt'><center>";
	cout << "<font size=2 face=\"Arial\"><center>You are viewing part of a restricted service. ";
	cout << "This service is subject to routine and random security ";
	cout << "audits and unauthorized access is prohibited. Abuse of this system and/or service is subject to criminal, ";
	cout << "civil, and extra-legal prosecution.</font></td></tr></table>" << CR << CR;
	PoweredBy();
}

void CreateAdmin(char* GUID)
{
        ContentType();
        HtmlStart();

        cout << "<form action=\"" << me << "\" method=post>";
        cout << "Please enter your <b>Administrator login name and password</b>:" << CR << CR;
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << GUID << "\">";
        cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"new\">";
        cout << "Username: <INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"username\">" << CR;
        cout << "Password: <INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password\">" << CR << CR;
        cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Create Admin\" ACTION=\"" << me << "\" method=post></center>";
        cout << "</FORM>";
        cout << CR;
        cout << CR << "<center><table width=\"33%\"><tr><center><td style='border:solid red .25pt'><center>";
        cout << "<font size=2 face=\"Arial\"><center>You are viewing part of a restricted service. ";
        cout << "This service is subject to routine and random security ";
        cout << "audits and unauthorized access is prohibited. Abuse of this system and/or service is subject to criminal, ";
        cout << "civil, and extra-legal prosecution.</font></td></tr></table>" << CR << CR;
        PoweredBy();
}

void ChPass(char* GUID)
{
	ContentType();
	HtmlStart();

	cout << "<form action=\"" << me << "\" method=post>";
	cout << "Please enter the your <b>new password</b>:" << CR << CR;
	cout << "<table><tr><td>";
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"guid\" VALUE=\"" << GUID << "\">";
	cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"action\" VALUE=\"chpw\">";
	cout << "Username:</td><td><INPUT TYPE=\"TEXT\" SIZE=\"30\" NAME=\"username\"></td></tr>";
	cout << "<tr><td>Old Password:</td><td><INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password1\"></td></tr>";
        cout << "<tr><td>New Password:</td><td><INPUT TYPE=\"PASSWORD\" SIZE=\"30\" NAME=\"password2\"></td></tr>";
	cout << "<table>";
	cout << "<INPUT TYPE=\"SUBMIT\" VALUE=\"Change Password\" ACTION=\"" << me << "\" method=post></center>";
	cout << "</FORM>";
}




void Credits()
{
	ContentType();
	HtmlStart();
	cout << "<b><u>Credits</b></u>" << CR << CR;
	cout << "Server-side programming: <b>C. Edwards</b> (BRAiN)" << CR;
	cout << "Plot and monster design: <b>C. Edwards & A. Mathers</b> (BRAiN & Thor)" << CR;
	cout << "Play-testing and beta-testing: <b>P. Cottrell, Azhrarn, DarkMaster, Yeti, peddaluxe</b>" << CR;
	cout << "Various pieces of in-game text: <b>P. Cottrell</b> (Angel_F)" << CR;
	cout << "Original manuscripts: <b>C. Edwards & A. Mathers</b> (BRAiN & Thor)" << CR;
	cout << "Test server admin: <b>BRAiN</b>" << CR << CR;
	cout << "Greets go to: <b>Halo Unit, The Shadow, The Scouse Mouse, Sack, X3R[]</b>." << CR << CR;
	cout << "Quote of the day: <pre>" << QuoteMe() << "</pre>" << CR;
	cout << CR << CR << "<font size=1>Version "<< VERSION << " (" << SysName() << ", " << Getmysqlver() << ")</font>" << CR << CR << CR;
	cout << "<b><u>Network Statistics for past 24 hours</u></b>" << CR << CR;
	cout << "<img src='http://mrtg.underhanded.org/localhost/localhost_00-01-80-31-81-ad-day.png'>" << CR;
	cout << "<b><u>User counts for past 24 hours</u></b>" << CR << CR;
	cout << "<img src='http://www.ssod.org/cgi-bin/ssod.cgi?action=drawstats'>" << CR;
	PoweredBy();
}


void StopCheat()
{
	// This nice little bit of javascript stops users clicking the
	// back button and using it as an escape route from dangerous
	// areas - as soon as they click the back button, the script is
	// called and sends them forwards one place in the history list,
	// back to the place they came from :-)
	cout << "<script event=onload for=window>" << endl;
	cout << "history.go(+1);" << endl;
	cout << "</script>";
}

