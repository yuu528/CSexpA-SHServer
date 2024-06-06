#ifndef _H_FILEUTIL_

#define _H_FILEUTIL_

#include "session.h"

#define FILE_BUFFER_SIZE 8192

#define EXT_HTML ".html"
#define EXT_HTM ".htm"
#define EXT_CSS ".css"
#define EXT_JS ".js"
#define EXT_PNG ".png"
#define EXT_JPG ".jpg"
#define EXT_GIF ".gif"
#define EXT_TXT ".txt"

#define EXT_PHP ".php"
#define EXT_CGI ".cgi"

#define MIME_HTML "text/html"
#define MIME_CSS "text/css"
#define MIME_JS "text/javascript"
#define MIME_PNG "image/png"
#define MIME_JPG "image/jpeg"
#define MIME_GIF "image/gif"
#define MIME_TXT "text/plain"

void check_file(session_info *);
int get_file_size(char *);
void get_file_type(char *, char *);
int get_parent_path(char *, char *);
void find_htaccess(session_info *);

#endif
