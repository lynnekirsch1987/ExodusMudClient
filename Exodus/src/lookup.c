#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
int oflag_lookup (const char *name, const struct flag_type *flag_table)
{
  int flag;
  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (LOWER (name[0]) == LOWER (flag_table[flag].name[0])
	  && !str_prefix (name, flag_table[flag].name))
	return flag_table[flag].bit;
    }
  return 0;
}

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
  int flag;
  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (LOWER (name[0]) == LOWER (flag_table[flag].name[0])
	  && !str_cmp (name, flag_table[flag].name))
	return flag_table[flag].bit;
    }
  return NO_FLAG;
}

int position_lookup (const char *name)
{
  int pos;
  for (pos = 0; position_table[pos].name != NULL; pos++)
    {
      if (LOWER (name[0]) == LOWER (position_table[pos].name[0])
	  && !str_prefix (name, position_table[pos].name))
	return pos;
    }
  return -1;
}

int sex_lookup (const char *name)
{
  int sex;
  for (sex = 0; sex_table[sex].name != NULL; sex++)
    {
      if (LOWER (name[0]) == LOWER (sex_table[sex].name[0])
	  && !str_prefix (name, sex_table[sex].name))
	return sex;
    }
  return -1;
}

int size_lookup (const char *name)
{
  int size;
  for (size = 0; size_table[size].name != NULL; size++)
    {
      if (LOWER (name[0]) == LOWER (size_table[size].name[0])
	  && !str_prefix (name, size_table[size].name))
	return size;
    }
  return -1;
}
