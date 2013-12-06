#include "stdafx.h"
#include "strings.h"

// upper case
char* ucase(char* s,char* d) 
{
	char* t = d;
	while (*s) {
		if (*s >= 'a' && *s <= 'z') *d++ = *s -32; 
		  else if (*s == (int)'�') *d++ = '�';
		  else if (*s == (int)'�') *d++ = '�';
		  else if (*s == (int)'a') *d++ = '�';
		  else if (*s == (int)'�') { *d++ = 'S'; *d++ = 'S'; } else *d++ = *s;
		s++;
	}
	*d = 0;
	return t;
}

// find next word in string, set *s to beginning of the next word and return the old *s
char* getword(char** s)
{
	while (**s == ' ') (*s)++;
	char* r = *s;

	while (**s && **s != ' ') (*s)++;
	*(*s)++ = '\0';
	while (**s && **s == ' ') (*s)++;
	return r;
}

// check if string is a number
bool isint(char* s)
{
	if (!s || !*s) return false;
	if (*s != '-' && (*s<'0' || *s>'9')) return false;
	s++;
	while (*s) {
		if (*s < '0' || *s > '9') return false;
		s++;
	}
	return true;
}

// check if string is a positive number
bool isposint(char* s)
{
	if (!s || !*s) return false;
	while (*s) {
		if (*s < '0' || *s > '9') return false;
		s++;
	}
	return true;
}

// alloc only *path before!!!
void splitpathname(char* p, char** f, char** e, char** path)
{
	*e = NULL; *f = NULL;

	for (int i=strlen(p);i>=0 && (!*e || !*f);i--) {
		if (*(p+i)=='.' && !*e) *e = p+i+1;
		if (*(p+i)=='\\' && !*f) *f = p+i+1;
	}

	if (path) {
		strcpy(*path,p);
		i=strlen(p)-1;
		while (*(p+i) != '\\' && i) i--;
		(*path)[i]=0;
	}

}
