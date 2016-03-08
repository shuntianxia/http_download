#ifndef _HTTP_DOWNLOAD_H_
#define _HTTP_DOWNLOAD_H_

#define URL_LEN 512
#define FILENAME_LEN 512
#define PATH_LEN 512
#define DEBUG_HTTP
#define DEBUG_HTTP_RECV

typedef struct url_info {
	char proto[16];
	char host[128];
	char ip[56];
	wiced_ip_address_t ip_addr;
	int port;
	char filepath[256];
	char filename[128];
	int filesize;
}url_info_t;

int http_download(char *url);


#endif
