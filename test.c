#include"http.h"
#include <stdio.h>
#include "stdafx.h"
#include <stdlib.h>
#include <WINSOCK2.H>
#include <windows.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")
#define HTTPPROTO "HTTP/1.1"
#define HTTPPROTOLEN 8
#define HTTPFLAG "\r\n"			//每行数据结束
#define HTTPFLAGLEN 2
#define HTTPFLAGEND "\r\n\r\n"	//连续两个代表信息结束，后面是数据内容
#define HTTPFLAGENDLEN 4
#define CONTENT_LENGTH "Content-Length"
#define TRANSFER_ENCODING "Transfer-Encoding"
#define CHUNKED "chunked"
//解析URL
void Parse_URL(const char*url,char*proto,char*hostname,char*hostip,int*port,char*filepath,char*filename);
//解析块写入文件
void Write_Chunk(const char*buff,int len,int*chunksize,int*countchunksize,FILE**fp);
bool HttpDownload(const char*url,const char*filename,bool utf8){
	char protoname[16]={0};
	char hostname[128]={0};
	char hostip[56]={0};
	int port=0;
	char filepath[256]={0};
	char filename2[128]={0};
	Parse_URL(url,protoname,hostname,hostip,&port,filepath,filename2);
	SOCKET sockclient=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (sockclient==SOCKET_ERROR){
		printf("Create Socket Err\n");
		return false;
	}
	int nNetTimeout = 5000; //5秒
	//发送时限
	setsockopt( sockclient, SOL_SOCKET, SO_SNDTIMEO, ( char * )&nNetTimeout, sizeof( int ) );
	//接收时限
	setsockopt( sockclient, SOL_SOCKET, SO_RCVTIMEO, ( char * )&nNetTimeout, sizeof( int ) );

	SOCKADDR_IN addServer;
	addServer.sin_addr.S_un.S_addr=inet_addr(hostip);//
	addServer.sin_family=AF_INET;
	addServer.sin_port=htons(port);	
	if (connect(sockclient,(struct sockaddr*)&addServer,sizeof(SOCKADDR_IN))==SOCKET_ERROR){
		printf("Connect ERR\n");
		return false;
	}
	FD_SET fdr;
	timeval tv;
	tv.tv_sec=3;
	tv.tv_usec=0;
	int ret;
	char buffer[10000];
	char request[2000]={0};
	char requesttemp[2000]={0};
	//特殊字符处理,这里只处理了空格
	char realPath[1000]={0};
	for (ret = 0; ret < strlen(filepath); ret++){
		if(filepath[ret]==' '){
			sprintf(realPath,"%s%%%02X",realPath,filepath[ret]);
		}else{
			sprintf(realPath,"%s%c",realPath,filepath[ret]);
		}
	} 
	sprintf(request,"GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
		//	"Accept: */*\r\n"
		// 	"Accept-Language: zh-cn\r\n"
		// 	"DontTrackMeHere: gzip, deflate\r\n"
		// 	"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.2; .NET4.0C)\r\n"			
		// 	"Connection: Keep-Alive\r\n"
			"Connection: Close\r\n"
			"\r\n",realPath,hostip);

	if(utf8){
	//编码转换
		MultiByteToWideChar( CP_ACP ,0,request,-1,(LPWSTR)requesttemp,2000); //ANSI-Unicode
		memset(request,0,2000);
		WideCharToMultiByte( CP_UTF8 ,0,(LPWSTR)requesttemp,-1,request,2000,NULL,NULL);//Unicode-UTF-8
	}
	
	ret=send(sockclient,request,strlen(request),0);

	const char*pos1=NULL;
	const char*pos2=NULL;
	const char*posm=NULL;
	const char*posend=NULL;
	const char*datastartpos=NULL;//数据开始

	int filelength=0;//文件总长
	int countfilelength=0;//累计文件长
	int chuncksize=0;//分片大小
	int countchunksize=0;//累计分片字节数
	int usechunk=0;//是否使用分片

	int checkhttp=0;
	FILE*fp=NULL;
	int  statd=0;
	char key[50];
	char value[200];

	char httpstat[20]={0};
	while(1){
	FD_ZERO(&fdr);
	FD_SET(sockclient,&fdr);
	ret=select(sockclient,&fdr,NULL,NULL,&tv);
	if (ret>0){
		if (FD_ISSET(sockclient,&fdr)){
			ret=recv(sockclient,buffer,10000,0);
			if (ret==0){
				//	printf("Link Close\n");
				closesocket(sockclient);
				if (fp!=NULL){
					fclose(fp);
					fp=NULL;
				}
				break;
		     }else{
			if (checkhttp){
				if (fp!=NULL){
					if(usechunk){
						Write_Chunk(buffer,ret,&chuncksize,&countchunksize,&fp);
				   }else{
						fwrite(buffer,1,ret,fp);	
				   }
			    }
			}else{
				memset(key,0,50);
				memset(value,0,200);
				statd=0;
				sscanf(buffer,"%s %d %s",key,&statd,httpstat);
					
				if (strncmp(key,"HTTP/1.1",strlen("HTTP/1.1"))!=0){
					printf("NOT HTTP\n");//不是HTTP会话
					return false;
			    }
				checkhttp=1;
				posend=strstr(buffer,HTTPFLAGEND);//二进制数据开始
				 if (posend==NULL){
					printf("Can't Find ByteData\n");
			    }					
/////////////////////////////////////将消息打印  格式key: value ,中间有空格
			//		printf("%s %d %s\n",key,statd,httpstat);
				pos1=buffer;
				pos2=strstr(pos1,HTTPFLAG);
				pos1=pos2+HTTPFLAGLEN;
				while(1){
				pos2=strstr(pos1,HTTPFLAG);
				posm=strstr(pos1,":");
				if (pos2==NULL||posm==NULL){
					break;
			    }
				memset(key,0,50);
				memset(value,0,200);
				memcpy(key,pos1,posm-pos1);
				memcpy(value,posm+2,pos2-posm-2);//冒号，空格
			//	printf("%s: %s\n",key,value);							
				if (strcmp(key,CONTENT_LENGTH)==0){
					sscanf(value," %d",&filelength);
			    }
				if (strcmp(key,TRANSFER_ENCODING)==0){
					if (strcmp(value,CHUNKED)==0){
						usechunk=1;
				   }else{
						printf("Warnning UnKnow %s %s\n",TRANSFER_ENCODING,value);
				   }
			     }
				if (pos2==posend){
					break;
			     }
				  pos1=pos2+HTTPFLAGLEN;//定位在\r\n之后
			  }
					
				/*	if(usechunk)
						printf("Use Chunk\n");
					else
						printf("filelength:%d\n",filelength);*/
////////////////////////////////////////////////写文件//////////////////////
				if(posend!=NULL){
					datastartpos=posend+HTTPFLAGENDLEN;
					if (strncmp(httpstat,"OK",2)!=0){
						//  fp=fopen("err.html","wb");
						//	printf("Not Find File:%s From Server\n",filename);
						return false;
				  }else
					    fp=fopen(filename,"wb");
					if (fp==NULL){
						printf("Open File Err:%s\n",filename);
						return false;
				   }else{
					   if (usechunk){
					  Write_Chunk(datastartpos,ret-(datastartpos-buffer),&chuncksize,&countchunksize,&fp);
					}else{
						fwrite(datastartpos,1,ret-(datastartpos-buffer),fp);
						}
									//fwrite(buffer,1,ret,fp);//全下载，包括头
					}
				}
//////////////////////////////////////////////////////////////////////////////////////
					}
				}
			}
		}
	}
	if(fp!=NULL)
		fclose(fp);
	return true;
}

void Parse_URL(const char*url,char*proto,char*hostname,char*hostip,int*port,char*filepath,char*filename){
	const	char*postemp;
	const	char*postemp2;
	const	char*poshost;//IP定位
	const	char*posfilepath;//filepath定位
	postemp=strstr(url,"://");
	if (postemp==NULL){
		strcpy(proto,"http");
		poshost=url;
	}else{
		memcpy(proto,url,postemp-url);
		poshost=postemp+3;
	}
	
	postemp=strstr(poshost,"/");
	postemp2=strstr(poshost,":");
	if (postemp==NULL){
		if (postemp2==NULL){
			strcpy(hostname,poshost);
			*port=80;
		}else{
			strncpy(hostname,poshost,postemp2-poshost);
			sscanf(postemp2+1,"%d",port);
		}
		strcpy(filepath,"/");
		strcpy(filename,"undefine");
	}else{
		posfilepath=postemp;
		if (postemp2==NULL||(postemp2>posfilepath)){
			strncpy(hostname,poshost,postemp-poshost);
			*port=80;
		}else{
			strncpy(hostname,poshost,postemp2-poshost);
			sscanf(postemp2+1,"%d",port);
		}
		strcpy(filepath,posfilepath);
		postemp+=1;
		postemp2=strstr(postemp,"/");
		while(postemp2!=NULL){
			postemp=postemp2+1;
			postemp2=strstr(postemp,"/");
		}
		strcpy(filename,postemp);
	}
	
	struct hostent *pHostEnt = gethostbyname(hostname);
	if (pHostEnt!=NULL){
	    sprintf(hostip,"%hhu.%hhu.%hhu.%hhu",
			(unsigned char) pHostEnt->h_addr_list[0][0],
			(unsigned char) pHostEnt->h_addr_list[0][1],
			(unsigned char) pHostEnt->h_addr_list[0][2],
			(unsigned char) pHostEnt->h_addr_list[0][3]);
		
	}
	
	//printf("PRORT:%s\nhostname:%s\nIP:%s\nPort:%d\nfilepath:%s\nfilename:%s\n\n",
	//	proto,hostname,hostip,*port,filepath,filename);
}

void Write_Chunk(const char*buff,int len,int*chunksize,int*countchunksize,FILE**fp)
{
	if (*fp==NULL){
		printf("Unable Write to File\n");
		return;
	}
	const char*pos=buff;
	const char*pos2=NULL;
	int tempchunksize=*chunksize;
	int tempcountchunksize=*countchunksize;
	while((pos-buff)<len){
		if (tempchunksize==0){
			sscanf(pos," %x",&tempchunksize);
//			printf("chunksize:%d\n",tempchunksize);
			pos2=strstr(pos,HTTPFLAG);
			pos=pos2+HTTPFLAGLEN;
			if (tempchunksize==0){
				printf("Get End\n");
				fclose(*fp);
				*fp=NULL;
				return;
			}
		}
		int needsize=tempchunksize-tempcountchunksize;//需要的字节数
		int cangetsize=len-(pos-buff);//能给的字节数
		int realsize=needsize<cangetsize?needsize:cangetsize;
		fwrite(pos,1,realsize,*fp);
		tempcountchunksize+=realsize;
		pos+=realsize;
		if (tempcountchunksize==tempchunksize){
			tempchunksize=0;
			tempcountchunksize=0;
		}
	}
	*chunksize=tempchunksize;
	*countchunksize=tempcountchunksize;
}