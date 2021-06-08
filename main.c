

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define DATABUFSIZE 1024
#define DATASIZE 1024


void *func_thread(void *arg);




int main(int argc, char *argv[]){
	
	int s, ss;
	struct sockaddr_in my, tar;

	//ポート番号:80
	unsigned short port = 80;

	char data[DATABUFSIZE];
	unsigned int len;
	int res;

    pthread_t pthread;
	
	if((ss = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
		perror("socket");
		exit(1);
	}

	memset(&my,0,sizeof(my));
	my.sin_family = AF_INET;
	my.sin_port   = htons(port);
	
	// ソケットとアプリの関連付け
	if(bind(ss, (struct sockaddr*)&my, sizeof(my)) < 0){
		perror("bind");
		close(ss);
		exit(1);
	}
	
	// 一度にコネクションを受け入れる数:SYNを受け入れる数（TCP独自）一度に通信をするクライアントの数ではない
	listen(ss, 1);
	
	len = sizeof(tar);

	
	
	while(1){
		memset(&tar, 0, sizeof(tar));

		// Webブラウザからのアクセスに反応
		s = accept(ss,(struct sockaddr*)&tar, &len);
		printf("# ブラウザからのアクセスあり\n");
		printf("--------------------------\n");
		printf("IP   : %s\n", inet_ntoa(tar.sin_addr));
		printf("port : %d\n", ntohs(tar.sin_port));
		printf("--------------------------\n");
		
		pthread_create(&pthread, NULL, &func_thread, &s);
    }


	// サーバソケットを閉じる
	close(ss);
	
	return 0;
}


void *func_thread(void *arg){

    int res,s = *(int *)arg;
    char data[DATABUFSIZE],html[DATABUFSIZE],line[DATASIZE];
	FILE *file;


    memset(data, '\0', sizeof(data));
	res = recv(s, data, sizeof(data)-1, 0);

	char path[100];
	
	//printf("%s",data);
	sscanf(data,"GET /%[^ ] HTTP/1.1",path);
	if(strcmp(path,"")==0){//もし指定されてないなら
		strcpy(path,"index.html");
	}


	if(fopen(path,"r") != NULL){//存在する場合
		sprintf(data, "HTTP/1.1 200 OK\r\n");

		file = fopen(path,"r");
		//読み込んだfileを一行ずつ変数に代入していく
		for(int i = 0;fgets(line,DATASIZE,file) != NULL;i++){//成功した場合

			if(i == 0){//初回
				strcpy(html,line);
			}else{
				strcat(html,line);
			}
		}

	}
	else{//存在しない場合
		sprintf(data, "HTTP/1.1 404 Not Found\r\n");

		//HTMLを生成
		strcpy(html, "<html>\n");
		strcat(html, "<head>\n");
		strcat(html, "<title>404 Not Found</title>\n");
		strcat(html, "</head>\n");
		strcat(html, "<body>\n");
		strcat(html, "<h1>Not Found</h1>\n");
		strcat(html, "<p>The requested URL was not found on this server.<p>\n");
		strcat(html, "</body>\n");
		strcat(html, "</html>\n");
	}

		send(s, data, strlen(data), 0);
	
		sprintf(data, "HTTP/1.1 200 OK\r\n");
		send(s, data, strlen(data), 0);
		
		sprintf(data, "Content-Length: %d\r\n", (int)strlen(html));
		send(s, data, strlen(data), 0);
		
		sprintf(data, "Connection: close\r\n");
		send(s, data, strlen(data), 0);
		
		sprintf(data, "Content-Type: text/html\r\n");
		send(s, data, strlen(data), 0);
		
		sprintf(data, "\r\n");
		send(s, data, strlen(data), 0);

		//HTML本文を送信
		send(s, html, strlen(html), 0);

		close(s);
}