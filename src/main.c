#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <regex.h>
#include <unistd.h>

#include <curl/curl.h>

#include "asprintf.h"
#include "settings.h"

#define IP_ADDR_REGEX "(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])"

typedef struct
{
  char *data;
  size_t len;
} string_t;

int mail_change (char *recipient, char *addr);
size_t write_callback (char *ptr, size_t size, size_t nmemb, void *userdata);
void handle_ip (char *addr);

int
main (int argc, char *argv[])
{
  /* Set up CURL */
  curl_global_init ( CURL_GLOBAL_ALL );

  do
    {
      struct settings *settings = get_settings();

      CURL *handle = curl_easy_init();
      if (!handle)
	{
	  fprintf (stderr, "FATAL could not get CURL handle\n");
	  exit (EXIT_FAILURE);
	}

      string_t *string = malloc (sizeof (string_t));
      string->data = NULL;
      string->len = 0;

      /* Configure call */
      curl_easy_setopt (handle, CURLOPT_URL, "http://ipecho.net/plain");
      curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, write_callback);
      curl_easy_setopt (handle, CURLOPT_WRITEDATA, string);

      /* Get IP address */
      CURLcode res = curl_easy_perform (handle);

      /* Check for errors */
      if (res != CURLE_OK)
	{
	  fprintf (stderr, "WARNING curl_easy_perform() failed: %s\n",
		    curl_easy_strerror (res));
	  if (string)
	    {
	      if (string->data)
		free (string->data);
	      free (string);
	    }
	  continue;
	}

      /* Validate IP */
      regex_t regex;
      if (regcomp (&regex, IP_ADDR_REGEX, REG_EXTENDED))
	{
	  fprintf (stderr, "FATAL could not compile regex \"%s\"\n",
		   IP_ADDR_REGEX);
	  exit (EXIT_FAILURE);
	}

      int regexec_return = regexec (&regex, string->data, 0, NULL, 0);

      if (regexec_return == 0)
	{
	  printf ("Found valid public ip %s\n", string->data);
	  handle_ip (string->data);
	}
      else if (regexec_return == REG_NOMATCH)
	{
	  fprintf (stderr, "WARNING found invalid public ip %s\n", string->data);
	}
      else
	{
	  char buf[100];
	  regerror (regexec_return, &regex, buf, sizeof(buf));
	  fprintf (stderr, "WARNING error while validating ip address: %s\n",
		   buf);
	}
      if (string->data)
	free (string->data);
      free (string);
      regfree (&regex);
      curl_easy_cleanup (handle);

      double freq = settings->freq;
      destroy_settings (settings);

      if (freq == 0)
	exit (EXIT_SUCCESS);
      else
	{
	  printf ("sleeping for %f hours\n", freq);
	  sleep (freq * 1200);
	}
    }
  while (1);

  return EXIT_SUCCESS;
}

size_t
write_callback (char *ptr, size_t size, size_t nmemb, void *userdata)
{
  string_t *string = userdata;  
  size_t len = string->len + (size * nmemb);
  char *data = malloc (len + 1);
  if (!data)
    return 0;

  memcpy (data, string->data, string->len);
  memcpy (data + string->len, ptr, size * nmemb);
  data[len] = '\0';

  if (string->data)
    free (string->data);
  string->data = data;
  string->len = len;

  return size * nmemb;
}

void
handle_ip (char *addr)
{
  char *homedir = getenv ("HOME");
  char *ipfile = "ip";

  char *path = NULL;
  asprintf (&path, "%s/%s", homedir, ipfile);

  char old[16] = "";
  FILE *ip = fopen (path, "r");
  if (ip)
    {
      int i = 0;
      int ch = fgetc (ip);
      while (i < 16 && ch != EOF)
	{
	  old[i] = ch;
	  ch = fgetc (ip);
	  i++;
	}
      fclose (ip);
    }
  else if (errno != ENOENT)
    {
      fprintf (stderr, "FATAL could not open file %s\n", path);
      exit (EXIT_FAILURE);
    }

  if (strcmp (addr, old) == 0)
    {
      printf ("public ip has not changed\n");
    }
  else
    {
      printf ("public ip has changed\n");
      ip = fopen (path, "w");
      if (!ip)
	{
	  fprintf (stderr, "FATAL could not open file %s for writing\n", path);
	  exit (EXIT_FAILURE);
	}

      fprintf (ip, "%s", addr);
      fclose (ip);

      struct settings *settings = get_settings();
      for (char **cur = settings->recipients; cur && *cur; cur ++)
	mail_change (*cur, addr);
      destroy_settings (settings);
   }

  free (path);
}
