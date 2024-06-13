typedef struct {
	char cmd[64];
	char path[256];
	char real_path[256];
	char type[64];
	int code;
	int size;
} exp1_info_type;

int exp1_tcp_listen(int port);
int exp1_tcp_connect(const char *hostname, int port);
int exp1_udp_listen(int port);
int exp1_udp_connect(const char *hostname, int port);
double gettimeofday_sec();
int exp1_do_talk(int sock);

int exp1_http_session(int sock);
int exp1_parse_header(char* buf, int size, exp1_info_type* info);
void exp1_parse_status(char* status, exp1_info_type *pinfo);
void exp1_check_file(exp1_info_type *info);
void exp1_http_reply(int sock, exp1_info_type *info);
void exp1_send_404(int sock);
void exp1_send_file(int sock, char* filename);
