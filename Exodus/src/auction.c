#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
int advatoi (const char *s)
{
  char string[MAX_INPUT_LENGTH];
  char *stringptr = string;
  char tempstring[2];
  int number = 0;
  int multiplier = 0;
  strcpy (string, s);
  while (isdigit (*stringptr))
    {
      strncpy (tempstring, stringptr, 1);
      number = (number * 10) + atoi (tempstring);
      stringptr++;
    }
  switch (UPPER (*stringptr))
    {
    case 'K':
      multiplier = 1000;
      number *= multiplier;
      stringptr++;
      break;
    case 'M':
      multiplier = 1000000;
      number *= multiplier;
      stringptr++;
      break;
    case '\0':
      break;
    default:
      return 0;
    }
  while (isdigit (*stringptr) && (multiplier > 1))
    {
      strncpy (tempstring, stringptr, 1);
      multiplier = multiplier / 10;
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
    }
  if (*stringptr != '\0' && !isdigit (*stringptr))
    return 0;
  return (number);
}

int parsebid (const int currentbid, const char *argument)
{
  int newbid = 0;
  char string[MAX_INPUT_LENGTH];
  char *stringptr = string;
  strcpy (string, argument);
  if (*stringptr)
    {
      if (isdigit (*stringptr))
	newbid = advatoi (stringptr);

      else if (*stringptr == '+')
	{
	  if (strlen (stringptr) == 1)
	    newbid = (currentbid * 125) / 100;

	  else
	    newbid = (currentbid * (100 + atoi (++stringptr))) / 100;
	}
      else
	{
	  if ((*stringptr == '*') || (*stringptr == 'x'))
	    {
	      if (strlen (stringptr) == 1)
		newbid = currentbid * 2;

	      else
		newbid = currentbid * atoi (++stringptr);
	    }
	}
    }
  return newbid;
}
