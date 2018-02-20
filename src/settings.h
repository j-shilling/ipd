#ifndef __SETTINGS_H__
#define __SETTINGS_H__

struct settings
{
  char *smtp_server;
  char *port;
  char *username;
  char *password;
  char *from;

  char **recipients;

  double freq;
};

struct settings *get_settings (void);
void destroy_settings (struct settings *ptr);

#endif /* __SETTINGS_H__ */
