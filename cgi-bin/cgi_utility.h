// Project: SSOD
// Content: GGI-level helper functions

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>

using namespace std;

char post_buffer[65536];

const char* getQueryString()
{
	if (getenv("CONTENT_LENGTH"))
	{
			cin.read(post_buffer,atoi(getenv("CONTENT_LENGTH")));
			return post_buffer;
	}
	char* i = getenv("QUERY_STRING");
	if (i)
	{
		return i;
	}
	else
	{
		return "\0";
	}
}

void RedirectTo(char* NewURI)
{
	cout << "Status: 302 Found\n";
	cout << "Location: " << NewURI << "\n";
	cout << "Content-Type: text/html\n\n";
	cout << "<html><META HTTP-EQUIV=\"Expires\" CONTENT= \"Tue, 20 Aug 1900 14:25:27 GMT\">";
	cout << "<body bgcolor=black><font color=red>";
	cout << "<HTML>Redirection to <a href=\"" << NewURI << "\">" << NewURI << "</a></HTML>";
	return;
}


/****************************************************************************
Parse Query - Rips all of the query fields from the URL and spilts them
into a pair of arrays (name + attribute)
****************************************************************************/

void ParseQuery()
{

 const char* informationGET;
 char* c;
 char* p;
 char hexVal;
 unsigned int i,k;
 int j;

 informationGET = getQueryString();
 
 c = (char *) informationGET;

 // Figure out how many bits of data were passed via the URL
 numItems = 0;
 for(i=0;i<strlen(c);++i)
 {
	if(c[i]=='=')
	numItems++;
 }

 // whoever put these here didnt know what they were doing, because it makes the
 // CGI non-rfc compliant!!!
 //cout << c;
 //cout << strlen(c);

	// Allocate numItem pointers for the strings
	formData = new char*[numItems];
	formName = new char*[numItems];

	// For each field get the name and the data in the field
	for(j=0;j<numItems;++j) {

		// Get the name of the field
		p = c;
		while((*p != '=')&&(*p++ != '\0'));
		formName[j] = new char[p-c+2];
		for(i=0;c<p;++c,++i) {
		switch(*c) {
		case '+':
			// It's a space
			formName[j][i] = ' ';
			break;
		case '%':
			// It's an escape character
			hexVal = 0;
			++c;
			for(k=0;(k<2)&&(isxdigit(*c));++k,++c) {
			if(isdigit(*c))
				hexVal = 16*hexVal + (*c-'0');
			else
				hexVal = 16*hexVal + (10+toupper(*c)-'A');
			}
			--c;
			formName[j][i] = hexVal;
			break;
		default:
			// plain old character
			formName[j][i] = *c;
			break;
		}
		}
		formName[j][i] = '\0';

		// Get the information in the field
		++c;
		++p;
		while((*p != '&')&&(*p++ != '\0'));
		formData[j] = new char[p-c+2];
		for(i=0;c<p;++c,++i) {
		switch(*c) {
		case '+':
			// It's a space
			formData[j][i] = ' ';
			break;
		case '%':
			// It's an escape character
			hexVal = 0;
			++c;
			for(k=0;(k<2)&&(isxdigit(*c));++k,++c) {
			if(isdigit(*c))
				hexVal = 16*hexVal + (*c-'0');
			else
				hexVal = 16*hexVal + (10+toupper(*c)-'A');
			}
			--c;
			formData[j][i] = hexVal;
			break;
		default:
			// plain old character
			formData[j][i] = *c;
			break;
		}
		}
		formData[j][i] = '\0';

		c++;
	}
}


bool email_ok(const char *address) {
  int        count = 0;
  const char *c, *domain;
  static char *rfc822_specials = "()<>@,;:\\\"[]";

  /* first we validate the name portion (name@domain) */
  for (c = address;  *c;  c++) {
    if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) == 
        '\"')) {
      while (*++c) {
        if (*c == '\"') break;
        if (*c == '\\' && (*++c == ' ')) continue;
        if (*c <= ' ' || *c >= 127) return false;
      }
      if (!*c++) return false;
      if (*c == '@') break;
      if (*c != '.') return false;
      continue;
    }
    if (*c == '@') break;
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  }
  if (c == address || *(c - 1) == '.') return false;

  /* next we validate the domain portion (name@domain) */
  if (!*(domain = ++c)) return false;
  do {
    if (*c == '.') {
      if (c == domain || *(c - 1) == '.') return false;
      count++;
    }
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  } while (*++c);

  return (count >= 1);
}


// modified base64 encoding squence

static char* cpBase64Encoding = "0987651234MNOPQRSTFEDCBAabcdefgzxwvutsrqponmlkjihyGHIJKLZYXWVU_^*";
//static char* cpBase64Encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";



void Base64Encode(const char* cpInput, char* cpOutput )
{
 int nIdx[ 4 ];  // will contain the indices of coded letters in 
                 // _cpBase64Encoding string; valid values [0..64]; the value
                 // of 64 has special meaning - the padding symbol

 // process the data (3 bytes of input provide 4 bytes of output)
 while ( '\0' != *cpInput )
 {
   nIdx[0] = ((*cpInput) & 0xFC)>>2;
   nIdx[1] = ((*cpInput) & 0x03)<<4;
   cpInput++;
   if ( '\0' != *cpInput )
   {
     nIdx[1] |= ((*cpInput) & 0xF0)>>4;
     nIdx[2]  = ((*cpInput) & 0x0F)<<2;
     cpInput++;
     if ( '\0' != (*cpInput) )
     {
       nIdx[2] |= ((*cpInput) & 0xC0) >> 6;
       nIdx[3]  = (*cpInput) & 0x3F;
       cpInput++;
     }
     else
       nIdx[3] = 64;
   }
   else
   { // refer to padding symbol '='
     nIdx[2] = 64;
     nIdx[3] = 64;
   }

   *(cpOutput+0) = *(cpBase64Encoding + nIdx[0]);
   *(cpOutput+1) = *(cpBase64Encoding + nIdx[1]);
   *(cpOutput+2) = *(cpBase64Encoding + nIdx[2]);
   *(cpOutput+3) = *(cpBase64Encoding + nIdx[3]);
   cpOutput += 4;
 }
 
 // set this to terminate output string
 *cpOutput = '\0';

 return;
}




void Base64Decode(const char* cpInput, char* cpOutput )
{
 int nIdx[ 4 ];  // will contain the indices of coded letters in 
                 // _cpBase64Encoding string; valid values [0..64]; the value
                 // of 64 has special meaning - the padding symbol

 // process the data (3 bytes of input provide 4 bytes of output
 while ( '\0' != *cpInput )
 {
   nIdx[ 0 ] = nIdx[ 1 ] = nIdx[ 2 ] = nIdx[ 3 ] = 64;
   nIdx[0] = (strchr( cpBase64Encoding, (*cpInput) ) - cpBase64Encoding);
   cpInput++;
   if ( '\0' != *cpInput )
   {
     nIdx[1] = (strchr( cpBase64Encoding, (*cpInput) ) - cpBase64Encoding);
     cpInput++;
     if ( '\0' != (*cpInput) )
     {
       nIdx[2] = (strchr( cpBase64Encoding, (*cpInput) ) - cpBase64Encoding);
       cpInput++;
       if ( '\0' != (*cpInput) )
       {
         nIdx[3] = (strchr( cpBase64Encoding, (*cpInput) ) - cpBase64Encoding);
         cpInput++;
       }
     }
   }

   if ( nIdx[3] == 64 ) nIdx[3] = 0;
   if ( nIdx[2] == 64 ) nIdx[2] = 0;
   if ( nIdx[1] == 64 ) nIdx[1] = 0;

   *(cpOutput+0) = (char)((nIdx[0]<<2) | (nIdx[1]>>4));
   *(cpOutput+1) = (char)((nIdx[1]<<4) | (nIdx[2]>>2));
   *(cpOutput+2) = (char)((nIdx[2]<<6) | nIdx[3]);
   cpOutput += 3;
 }
 
 // set this to terminate output string
 *cpOutput = '\0';

 return;
}

