#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include <curl/curl.h>

#include "asprintf.h"
#include "settings.h"

struct message
{
  char **payload_text;
  int lines_read;
};

static struct message *
build_message (char *from, char *recipient, char *addr)
{
  char datebuf[100];
  time_t now = time (0);
  struct tm tm = *gmtime (&now);
  strftime (datebuf, sizeof (datebuf), "%a, %d %b %Y %H:%M:%S %Z", &tm);

  struct message *ret = malloc (sizeof (struct message));
  ret->lines_read = 0;

  ret->payload_text = malloc (sizeof (char *) * 9);
  asprintf (&ret->payload_text[0], "Date: %s\r\n", datebuf);
  asprintf (&ret->payload_text[1], "To: %s\r\n", recipient);
  asprintf (&ret->payload_text[2], "From: %s\r\n", from);
  asprintf (&ret->payload_text[3], "Message-ID: <%d-%d@ipd.com>\r\n", getpid(), datebuf);
  asprintf (&ret->payload_text[4], "Subject: Public IP Address has changed\r\n");
  asprintf (&ret->payload_text[5], "\r\n");
  asprintf (&ret->payload_text[6], "It is now %s\r\n", addr);
  asprintf (&ret->payload_text[7], "\r\n");
  ret->payload_text[8] = NULL;

  return ret;
}

static size_t
payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct message *message = userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = message->payload_text[message->lines_read];

  if (data)
    {
      size_t len = strlen(data);
      memcpy(ptr, data, len);
      message->lines_read++;

      return len;
    }

  return 0;
}

int
mail_change (char *recipient, char *addr)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct settings *settings = get_settings ();
  if (!settings)
    return -1;
  if (!settings->smtp_server)
    return 0;

  struct message *message = build_message (settings->from, recipient, addr);
 
  curl = curl_easy_init();
  if(curl)
    {
      char *url = NULL;
      if (settings->port)
	asprintf (&url, "smtp://%s:%s", settings->smtp_server, settings->port);
      else
	asprintf (&url, "smtp://%s", settings->smtp_server);

      curl_easy_setopt(curl, CURLOPT_URL, url);
      free (url);

      if (settings->username)
	curl_easy_setopt(curl, CURLOPT_USERNAME, settings->username);
      if (settings->password)
	curl_easy_setopt(curl, CURLOPT_PASSWORD, settings->password);

      curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
   
      if (settings->from)
	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, settings->from);
   
      recipients = curl_slist_append(recipients, recipient);
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
   
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
      curl_easy_setopt(curl, CURLOPT_READDATA, message);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
   
      res = curl_easy_perform(curl);
   
      if(res != CURLE_OK)
	fprintf(stderr, "curl_easy_perform() failed: %s\n",
		curl_easy_strerror(res));
   
      curl_slist_free_all(recipients);
   
      curl_easy_cleanup(curl);

      for (char **cur = message->payload_text; cur && *cur; cur++)
	free (*cur);
      free (message->payload_text);
      free (message);
      destroy_settings (settings);
    }
 
  return (int)res;
}
