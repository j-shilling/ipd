#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "settings.h"
#include "asprintf.h"

struct slist
{
  char *value;
  struct slist *next;
};

struct settings *
get_settings (void)
{
  struct settings *ret = calloc (1, sizeof (struct settings));
  ret->freq = 0;
  int recipients_num = 0;
  struct slist *recipients = NULL;
  struct slist *cur_node = NULL;
  
  FILE *file = fopen (SYSCONFDIR "/ipd.conf", "r");
  if (!file)
    {
      fprintf (stderr, "WARNING cannot open /etc/ipd.config");
    }
  else
    {
      size_t len = 0;
      char *line = NULL;
      while (getline (&line, &len, file) != -1)
	{
	  char key[15] = "";
	  char value[100] = "";
	  
	  char *cur = key;
	  int index = 0;

	  for (int i = 0; i < strlen (line); i ++)
	    {
	      if (line[i] == '\n')
		{
		  /* Done with this line */
		  break;
		}

	      else if (isspace (line[i]))
		{
		  /* Ignore whitespace */
		  continue;
		}

	      else if (index == 0 && cur == key && line[i] == '#')
		{
		  /* This line is commented out */
		  break;
		}

	      else if (line[i] == '=')
		{
		  /* Switching from key to value */
		  cur[index] = '\0';
		  cur = value;
		  index = 0;
		}

	      else
		{
		  /* Append char to cur */
		  cur[index] = line[i];
		  index ++;
		}
	    }

	  if (0 == strcmp (key, "smtp_server"))
	    asprintf (&ret->smtp_server, "%s", value);
	  else if (0 == strcmp (key, "port"))
	    asprintf (&ret->port, "%s", value);
	  else if (0 == strcmp (key, "username"))
	    asprintf (&ret->username, "%s", value);
	  else if (0 == strcmp (key, "password"))
	    asprintf (&ret->password, "%s", value);
	  else if (0 == strcmp (key, "from"))
	    asprintf (&ret->from, "%s", value);
	  else if (0 == strcmp (key, "recipient"))
	    {
	      if (cur_node)
		{
		  cur_node->next = malloc (sizeof (struct slist));
		  cur_node->next->next = NULL;
		  cur_node = cur_node->next;
		  
		}
	      else
		{
		  recipients = malloc (sizeof (struct slist));
		  recipients->next = NULL;
		  cur_node = recipients;
		}

	      asprintf (&cur_node->value, "%s", value);
	      recipients_num ++;
	    }
	  else if (0 == strcmp (key, "frequency"))
	    {
	      ret->freq = atof (value);
	    }

	}

      fclose (file);
      if (line)
	free (line);
    }

  ret->recipients = calloc (recipients_num + 1, sizeof (char *));
  int i = 0;
  cur_node = recipients;
  while (cur_node)
    {
      ret->recipients[i] = cur_node->value;
      i ++;

      struct slist *tmp = cur_node->next;
      free (cur_node);

      cur_node = tmp;
    }

  return ret;
}

void
destroy_settings (struct settings *ptr)
{
  if (ptr)
    {
      if (ptr->smtp_server)
	free (ptr->smtp_server);
      if (ptr->port)
	free (ptr->port);
      if (ptr->username)
	free (ptr->username);
      if (ptr->password)
	free (ptr->password);
      if (ptr->from)
	free (ptr->from);

      if (ptr->recipients)
	{
	  for (char **cur = ptr->recipients; cur && *cur; cur++)
	    free (*cur);
	  free (ptr->recipients);
	}

      free (ptr);
    }
}
