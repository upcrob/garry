/*
 * String utility functions.
 */

#include "stddef.h"
#include "strutil.h"

/*
 * Compare two strings for equality.
 */
int streq(char* s1, char* s2)
{
	int i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0')
		i++;
	
	if (s1[i] == s2[i] && s1[i] == '\0')
		return TRUE;
	else
		return FALSE;
}