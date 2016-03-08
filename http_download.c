#include "wiced.h"
#include "http_download.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define MAX_RECV_SIZE    8192
#define TCP_PACKET_MAX_DATA_LENGTH        30
#define TCP_CLIENT_INTERVAL               2
#define TCP_CLIENT_CONNECT_TIMEOUT        500
#define TCP_CLIENT_RECEIVE_TIMEOUT        300
#define TCP_CONNECTION_NUMBER_OF_RETRIES  3
#define RX_BUFFER_SIZE    64
#define TEST_STR          "Type something! Keystrokes are echoed to the terminal\n"

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/


/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_tcp_socket_t  tcp_client_socket;
static wiced_timed_event_t sta_conn_check_event;
static int reconnected_status = 0;
static url_info_t url_info;
static char g_buf_send[10*1024];//发送数据暂存区
static char g_buf_recv[10*1024];//接收数据暂存区

static wiced_uart_config_t uart_config =
{
    .baud_rate    = 1036800,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

wiced_ring_buffer_t rx_buffer;
uint8_t             rx_data[RX_BUFFER_SIZE];

/******************************************************
 *               Function Definitions
 ******************************************************/
 int reboot()
{
    WPRINT_APP_INFO( ( "Rebooting...\n" ) );
    host_rtos_delay_milliseconds( 1000 );

    wiced_framework_reboot();

    /* Never reached */
    return 0;
}

void print_ip_address(wiced_ip_address_t *ip_addr)
{
	WPRINT_APP_INFO (("%u.%u.%u.%u\n", (unsigned char) ( ( GET_IPV4_ADDRESS(*ip_addr) >> 24 ) & 0xff ),
										(unsigned char) ( ( GET_IPV4_ADDRESS(*ip_addr) >> 16 ) & 0xff ),
										(unsigned char) ( ( GET_IPV4_ADDRESS(*ip_addr) >>  8 ) & 0xff ),
										(unsigned char) ( ( GET_IPV4_ADDRESS(*ip_addr) >>  0 ) & 0xff )
										 ));
}

static wiced_result_t sta_conn_check(void *arg)
{
	wiced_ip_address_t gw_ip_addr;
	static int conn_count;
	
	WPRINT_APP_INFO( ("sta_conn_check\n") );
	if(conn_count++ >= 20)
		reboot();
	if(wiced_network_is_up(WICED_STA_INTERFACE) == WICED_TRUE) {
		if(wiced_ip_get_gateway_address(WICED_STA_INTERFACE, &gw_ip_addr) != WICED_SUCCESS) {
			WPRINT_APP_INFO( ("get geteway address failed\n") );
			return WICED_ERROR;
		}
		WPRINT_APP_INFO ( ("geteway ip address: ") );
		print_ip_address(&gw_ip_addr);
		
		wiced_rtos_deregister_timed_event(&sta_conn_check_event);

		//http_download("http://www.baidu.com/img/bd_logo1.png");
		//host_rtos_delay_milliseconds(3000);
		//http_download("http://img30.360buyimg.com/jgsq-productsoa/jfs/t1990/14/391467317/381809/6e9933ae/5603d37eN4701ca83.jpg");
		//http_download("http://192.168.31.154/img/bd_logo1.png");
		host_rtos_delay_milliseconds( 10000 );
		//http_download("http://192.168.31.154/img/guihuada.gif");
		//http_download("http://192.168.31.154/img/IMG_2339.JPG");
		http_download("http://192.168.31.154/img/IMG_10M.jpg");
	}
	return WICED_SUCCESS;
}

static wiced_result_t sta_reconn_check(void *arg)
{
	wiced_ip_address_t ip_addr;
	wiced_ip_address_t gw_ip_addr;
	static int conn_count;
	
	WPRINT_APP_INFO( ("sta_reconn_check\n") );
	if(conn_count++ >= 60)
		reboot();
	if(reconnected_status == 1) {
		/* Register callbacks */
		//wiced_network_register_link_callback( link_up, link_down );

		/* Initialise semaphore to notify when the network comes up */
		//wiced_rtos_init_semaphore( &link_up_semaphore );
		
		/* The link_up() function sets a semaphore when the link is back up. Wait here until the semaphore is set */
		//wiced_rtos_get_semaphore( &link_up_semaphore, WICED_NEVER_TIMEOUT );

		//wiced_rtos_delay_milliseconds(1000);

		if(wiced_ip_get_gateway_address(WICED_STA_INTERFACE, &gw_ip_addr) != WICED_SUCCESS) {
			WPRINT_APP_INFO( ("get geteway address failed\n") );
			return WICED_ERROR;
		}
		WPRINT_APP_INFO ( ("geteway ip address: ") );
		print_ip_address(&gw_ip_addr);
		
		wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &ip_addr );
		if(GET_IPV4_ADDRESS(ip_addr) != 0) {
			wiced_rtos_deregister_timed_event(&sta_conn_check_event);
		}
		//wiced_rtos_register_timed_event( &process_sta_rx_event, WICED_NETWORKING_WORKER_THREAD, &process_pre_dev_packet, 1*SECONDS, 0 );
		//wiced_rtos_create_thread(&sta_receive_thread, WICED_NETWORK_WORKER_PRIORITY, "sta udp receive thread", udp_sta_thread_main, UDP_RECEIVE_STACK_SIZE, 0);
		//wiced_udp_register_callbacks(&sta_socket, socket_receive_callback);
	}
	return WICED_SUCCESS;
}

static void link_up( void )
{

	WPRINT_APP_INFO( ("And we're connected again!\n") );
	/* Set a semaphore to indicate the link is back up */
	//wiced_rtos_set_semaphore( &link_up_semaphore );

	reconnected_status = 1;
}

static void link_down( void )
{
	WPRINT_APP_INFO( ("Network connection is down.\n") );

	reconnected_status = 0;

	wiced_rtos_register_timed_event( &sta_conn_check_event, WICED_NETWORKING_WORKER_THREAD, &sta_reconn_check, 1*SECONDS, 0 );
	
	//wiced_udp_delete_socket(&sta_socket);
}

static int tcp_connect(wiced_tcp_socket_t *socket, wiced_ip_address_t *ip, uint16_t port)
{
	wiced_result_t			 result;
	int 					 connection_retries;

	/* Connect to the remote TCP server, try several times */
	connection_retries = 0;
	do
	{
		result = wiced_tcp_connect( socket, ip, port, TCP_CLIENT_CONNECT_TIMEOUT );
		connection_retries++;
	}
	while( ( result != WICED_SUCCESS ) && ( connection_retries < TCP_CONNECTION_NUMBER_OF_RETRIES ) );
	if( result != WICED_SUCCESS)
	{
		WPRINT_APP_INFO(("Unable to connect to the server!\n"));
		return WICED_ERROR;
	}
	
	return WICED_SUCCESS;
}

static int tcp_disconnect(wiced_tcp_socket_t* socket)
{
	return wiced_tcp_disconnect(socket);
}

static int tcp_send_data(wiced_tcp_socket_t* socket, char *sendbuf, int len)
{
	wiced_packet_t* 		 packet;
	char*					 tx_data;
	uint16_t				 available_data_length;
	
	/* Create the TCP packet. Memory for the tx_data is automatically allocated */
	if (wiced_packet_create_tcp(socket, TCP_PACKET_MAX_DATA_LENGTH, &packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
	{
		WPRINT_APP_INFO(("TCP packet creation failed\n"));
		return WICED_ERROR;
	}

	printf("send :\n%s\n",sendbuf);

	/* Write the message into tx_data"	*/
	memcpy(tx_data, sendbuf, len);

	/* Set the end of the data portion */
	wiced_packet_set_data_end(packet, (uint8_t*)tx_data + len);

	/* Send the TCP packet */
	if (wiced_tcp_send_packet(socket, packet) != WICED_SUCCESS)
	{
		WPRINT_APP_INFO(("TCP packet send failed\n"));

		/* Delete packet, since the send failed */
		wiced_packet_delete(packet);

		/* Close the connection */
		wiced_tcp_disconnect(socket);
		return WICED_ERROR;
	}

	return WICED_SUCCESS;
}

static int tcp_recv_data(wiced_tcp_socket_t* socket, char *recvbuf, int len)
{
	wiced_packet_t* 		 rx_packet;
	char*					 rx_data;
	uint16_t				 rx_data_length;
	uint16_t				 available_data_length;
	wiced_result_t			 result;

	/* Receive a response from the server */
	result = wiced_tcp_receive(socket, &rx_packet, TCP_CLIENT_RECEIVE_TIMEOUT);
	if( result != WICED_SUCCESS )
	{
		WPRINT_APP_INFO(("TCP packet reception failed\n"));

		/* Delete packet, since the receive failed */
		wiced_packet_delete(rx_packet);

		/* Close the connection */
		wiced_tcp_disconnect(socket);
		return -1;
	}

	/* Get the contents of the received packet */
	wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

	/* Null terminate the received string */
	//rx_data[rx_data_length] = '\x0';
	//printf(("rx_data is %s", rx_data));

	memcpy(recvbuf, rx_data, rx_data_length<len?rx_data_length:len);

	printf("rx_data_length = %d\n", rx_data_length);
	//printf("available_data_length = %d\n", available_data_length);
	//printf("recvbuf is %s\n", recvbuf);
	
	/* Delete the packet */
	wiced_packet_delete(rx_packet);
	return rx_data_length;
}

int http_get_content_length(char *revbuf)
{
    char *p1 = NULL, *p2 = NULL;
    int HTTP_Body = 0;//内容体长度

    p1 = strstr(revbuf,"Content-Length");
    if(p1 == NULL)
        return -1;
    else
    {
        p2 = p1+strlen("Content-Length")+ 2; 
        HTTP_Body = atoi(p2);
        return HTTP_Body;
    }
}

int http_get_recv_length(char *revbuf)
{
    char *p1 = NULL;
    int HTTP_Body = 0;//内容体长度
    int HTTP_Head = 0;//HTTP 协议头长度

    HTTP_Body = http_get_content_length(revbuf);
    if(HTTP_Body == -1)
        return -1;

    p1=strstr(revbuf,"\r\n\r\n");
    if(p1==NULL)
        return -1;
    else
    {
        HTTP_Head = p1- revbuf +4;// 4是\r\n\r\n的长度
        return HTTP_Body+HTTP_Head;
    }
}

int parse_url(const char*url, url_info_t *url_info)
{
	const	char*postemp;
	const	char*postemp2;
	const	char*poshost;//IP定位
	const	char*posfilepath;//filepath定位
	postemp=strstr(url,"://");
	if (postemp==NULL){
		strcpy(url_info->proto,"http");
		poshost=url;
	}else{
		memcpy(url_info->proto,url,postemp-url);
		poshost=postemp+3;
	}
	
	postemp=strstr(poshost,"/");
	postemp2=strstr(poshost,":");
	if (postemp==NULL){
		if (postemp2==NULL){
			strcpy(url_info->host,poshost);
			url_info->port = 80;
		}else{
			strncpy(url_info->host,poshost,postemp2-poshost);
			sscanf(postemp2+1,"%d", &url_info->port);
		}
		strcpy(url_info->filepath,"/");
		strcpy(url_info->filename,"undefine");
	}else{
		posfilepath=postemp;
		if (postemp2==NULL||(postemp2>posfilepath)){
			strncpy(url_info->host,poshost,postemp-poshost);
			url_info->port = 80;
		}else{
			strncpy(url_info->host,poshost,postemp2-poshost);
			sscanf(postemp2+1,"%d",&url_info->port);
		}
		strcpy(url_info->filepath,posfilepath);
		postemp+=1;
		postemp2=strstr(postemp,"/");
		while(postemp2!=NULL){
			postemp=postemp2+1;
			postemp2=strstr(postemp,"/");
		}
		strcpy(url_info->filename,postemp);
	}

	if(wiced_hostname_lookup(url_info->host, &url_info->ip_addr, 1000) != WICED_SUCCESS) {
		printf("parse url failed\n");
		return -1;
	}
	sprintf(url_info->ip,"%hhu.%hhu.%hhu.%hhu",
		(unsigned char) (GET_IPV4_ADDRESS(url_info->ip_addr) >> 24),
		(unsigned char) (GET_IPV4_ADDRESS(url_info->ip_addr) >> 16),
		(unsigned char) (GET_IPV4_ADDRESS(url_info->ip_addr) >> 8),
		(unsigned char) (GET_IPV4_ADDRESS(url_info->ip_addr) >> 0));
	printf("PRORT:%s\nhostname:%s\nIP:%s\nPort:%d\nfilepath:%s\nfilename:%s\n\n",
		url_info->proto,url_info->host,url_info->ip,url_info->port,url_info->filepath,url_info->filename);
	return 0;
}

int http_recv(wiced_tcp_socket_t* socket, char *buf_recv)
{
    int ret;
    int recvlen=0;
    int downloadlen = 0;
	int i = 0;
    //int contentlen=0;    
    memset(g_buf_recv,0x0,sizeof(g_buf_recv)); 
	
    while(1)
    {
		printf("recv %d:\n", ++i);
        ret = tcp_recv_data(socket,g_buf_recv+recvlen,sizeof(g_buf_recv)-1);

        if(ret <= 0)//下载失败
        {
            printf("ERR:recv fail");
            return ret;
        }
    
    
        if(recvlen == 0)
        {
            #ifdef DEBUG_HTTP_RECV
            //printf("recv len = %d\n", ret);
            #endif
            //获取需要下载长度;
            downloadlen = http_get_recv_length(g_buf_recv);
            #ifdef DEBUG_HTTP_RECV
            printf("need downloadlen = %d\n",downloadlen);
            #endif
        }

        recvlen += ret;
        #ifdef DEBUG_HTTP_RECV
        printf("total recvlen = %d\n",recvlen);
        #endif

        if(downloadlen == recvlen)
            break;
    }
    return recvlen;
}

int http_get_filesize(wiced_tcp_socket_t* socket, char *path)
{
    int ret = -1;

	memset(g_buf_send, 0x0, sizeof(g_buf_send));         
    sprintf(g_buf_send, "HEAD %s", path);

    //HTTP/1.1\r\n 前面需要一个空格
    strcat(g_buf_send," HTTP/1.1\r\n");
    strcat(g_buf_send, "Host: ");
    strcat(g_buf_send, url_info.host);
    //strcat(g_buf_send, ":");
    //strcat(g_buf_send, PORT);
    strcat(g_buf_send,"\r\nConnection: Keep-Alive\r\n\r\n");
#ifdef DEBUG_HTTP
    //printf("send = %s \n",g_buf_send);
#endif
    //wiced_tcp_send_buffer(sockfd,g_buf_send,strlen(g_buf_send));
	tcp_send_data(socket,g_buf_send,strlen(g_buf_send));

    memset(g_buf_recv, 0, sizeof(g_buf_recv));
    ret = tcp_recv_data(socket,g_buf_recv,sizeof(g_buf_recv)-1);
#ifdef DEBUG_HTTP
    printf("recv len = %d\n", ret);
    //printf("recv = %s\n", g_buf_recv);
#endif
    if(ret <= 0)
    {
        printf("ERR:recv fail GetFileSize()");
        return -1;

    }	
    ret = http_get_content_length(g_buf_recv);
    if(ret <= 0)
        return -1;
    else
        return ret;
}

int http_get_file(wiced_tcp_socket_t* socket, url_info_t *url_info)
{
    int count;
    char range[32];
	char buf[64];
    int i;
    int j = 0;//成功下载次数
    int ret = -1;
    char *p = NULL;

	//Uart file transfer start
	memset(g_buf_send, 0x0, sizeof(g_buf_send));		   
	sprintf(g_buf_send, "Start\nFile-Name: %s\n", url_info->filename);
	sprintf(g_buf_send + strlen(g_buf_send), "File-Size: %d\n", url_info->filesize);
	printf("%s", g_buf_send);
	wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 64);
	host_rtos_delay_milliseconds( 3000 );

    count = (url_info->filesize%MAX_RECV_SIZE)?(url_info->filesize/MAX_RECV_SIZE +1):(url_info->filesize/MAX_RECV_SIZE);
	printf("count = %d\n", count);

    for(i=0;i<count;i++)
    {
        if((i == (count-1))&&(url_info->filesize%MAX_RECV_SIZE))
            sprintf(range,"%d-%d",i*MAX_RECV_SIZE,url_info->filesize-1);
        else
            sprintf(range,"%d-%d",i*MAX_RECV_SIZE,(i+1)*MAX_RECV_SIZE-1);

		memset(g_buf_send, 0x0, sizeof(g_buf_send));		   
		sprintf(g_buf_send, "GET %s",url_info->filepath);
		
		strcat(g_buf_send," HTTP/1.1\r\n");
		strcat(g_buf_send, "Host: ");
		strcat(g_buf_send, url_info->host);
		//strcat(g_buf_send, ":");
		//strcat(g_buf_send, PORT);
		
		sprintf(buf, "\r\nRange: bytes=%s",range);
		strcat(g_buf_send,buf);
		strcat(g_buf_send, "\r\nKeep-Alive: 200");
		strcat(g_buf_send,"\r\nConnection: Keep-Alive\r\n\r\n");
		
        #ifdef DEBUG_HTTP
		//printf("send :\n%s\n",g_buf_send);
        #endif
		
		//wiced_tcp_send_buffer(socket, g_buf_send, strlen(g_buf_send));
		tcp_send_data(socket,g_buf_send,strlen(g_buf_send));

        ret = http_recv(socket, g_buf_recv);

        if(ret < 0 )
        {
            tcp_connect(&tcp_client_socket, &url_info->ip_addr, url_info->port);
             i--;
            continue;
        }
        p = strstr(g_buf_recv,"\r\n\r\n");
        if(p == NULL)
        {
            printf("ERR:g_buf_recv not contain end flag\n");
            //break;
            return -1;
        }
		else
        {
			//memcpy(filebuf+j*MAX_RECV_SIZE,p+4,ret -(p+4-g_buf_recv));
			WPRINT_APP_INFO(("send download data via uart ...\n\n"));
#if 0
			memset(g_buf_send, 0x0, sizeof(g_buf_send));
			sprintf(g_buf_send, "Length: %d\n", ret -(p+4-g_buf_recv));
			printf("%s", g_buf_send);
			//wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 32);
			memcpy(g_buf_send + 32, p + 4, ret -(p+4-g_buf_recv));
			wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 32 + ret -(p+4-g_buf_recv));
			//memset(g_buf_send, 0x0, sizeof(g_buf_send));
			//wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 2000);
			
			//wiced_uart_transmit_bytes( WICED_UART_2, p+4, ret -(p+4-g_buf_recv));
			printf("ret -(p+4-g_buf_recv) = %d\n", ret -(p+4-g_buf_recv));
            j++;
#endif
			memset(g_buf_send, 0x0, sizeof(g_buf_send));
			sprintf(g_buf_send, "Length: %d\n", ret -(p+4-g_buf_recv));
			printf("%s", g_buf_send);
			//wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 32);
			memcpy(g_buf_send + 32, p + 4, ret -(p+4-g_buf_recv));
			wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send + 32, ret -(p+4-g_buf_recv));
			//memset(g_buf_send, 0x0, sizeof(g_buf_send));
			//wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, 2000);
			
			//wiced_uart_transmit_bytes( WICED_UART_2, p+4, ret -(p+4-g_buf_recv));
			printf("ret -(p+4-g_buf_recv) = %d\n", ret -(p+4-g_buf_recv));
			j++;
        }
    }
	//Uart file transfer Finished
	/*memset(g_buf_send, 0x0, sizeof(g_buf_send));		   
	sprintf(g_buf_send, "End\n");
	printf("%s", g_buf_send);
	wiced_uart_transmit_bytes( WICED_UART_2, g_buf_send, strlen(g_buf_send));*/
    return 0;
}

int http_download(char *url)
{
	int file_size;

    parse_url(url,&url_info);

	/* Create a TCP socket */
    if (wiced_tcp_create_socket(&tcp_client_socket, WICED_STA_INTERFACE) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP socket creation failed\n"));
		return WICED_ERROR;
    }
	
    //建立连接
    if(tcp_connect(&tcp_client_socket, &url_info.ip_addr, url_info.port)!= WICED_SUCCESS) {
		return WICED_ERROR;
    }

    //获取下载文件的大小
    file_size = http_get_filesize(&tcp_client_socket, url_info.filepath);
    if(file_size == -1)
        return -1;
    #ifdef DEBUG_HTTP
    printf("target file size is %d\n",file_size);
    #endif
	url_info.filesize = file_size;

    //下载文件
    if(http_get_file(&tcp_client_socket,&url_info) == 0) {
		printf("file download finished\n\n");
    }

	//断开连接
    tcp_disconnect(&tcp_client_socket);

	/* Delete a TCP socket */
	wiced_tcp_delete_socket(&tcp_client_socket);
	return WICED_SUCCESS;
}

void application_start(void)
{
    /* Initialise the device and WICED framework */
    wiced_init( );

	/* Initialise ring buffer */
    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );

    /* Initialise UART. A ring buffer is used to hold received characters */
    wiced_uart_init( WICED_UART_2, &uart_config, &rx_buffer );

	/* Register callbacks */
	wiced_network_register_link_callback( link_up, link_down );
	
	/* Bring up the network interface */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
	wiced_rtos_register_timed_event( &sta_conn_check_event, WICED_NETWORKING_WORKER_THREAD, &sta_conn_check, 1*SECONDS, 0 );
	/*while(1)
	{
		wiced_uart_transmit_bytes( WICED_UART_2, TEST_STR, sizeof( TEST_STR ) - 1 );
		host_rtos_delay_milliseconds( 5000 );
	}
	for(i = 0; i < 256; i++) {
		buf[0] = i;
		wiced_uart_transmit_bytes( WICED_UART_2, buf, 1);
		printf("buf[0] is %d\n", buf[0]);
	}*/
}
