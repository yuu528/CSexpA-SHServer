#ifndef _H_FILEUTIL_

#define _H_FILEUTIL_

#include "session.h"

void check_file(session_info *);
int get_parent_path(char *, char *);
void find_htaccess(session_info *);

#endif
