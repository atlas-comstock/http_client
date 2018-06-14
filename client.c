#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <regex.h>

char* getIPFromDNS(char *host) {
    struct hostent *server = gethostbyname(host);
    char *address = (char*)malloc(50);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
    }else {
        printf("server 's hostname: %s\n\n", server->h_name);

        struct in_addr **addr_list = (struct in_addr **) server->h_addr_list;
        for(int i = 0; addr_list[i] != NULL; i++) {
            strcpy(address, inet_ntoa(*addr_list[i]) );
            printf(" 解析到的 ip 地址为: IP ADDRESS->%s\n", address);
        }
    }
    return address;
}

void generate_request_header(char *hostname, char *url, char *request_header)
{
    // 注意这里有坑, http 协议规定header参数不能有空格, 所以下面必须没有空格
    //  另外, HOST 这个头部参数为了应对 server_name,也是必须的

    sprintf(request_header, "\
GET / HTTP/1.1\r\n\
HOST: %s\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36\r\n\
Cache-Control: no-cache\r\n\r\n", hostname);
    // 另一种办法, 但调用函数太多次了, 效率相对不高
    /* strcat(request_header, "GET "); */
	/* strcat(request_header, "/"); */
	/* strcat(request_header, " HTTP/1.1\r\n"); */
	/* strcat(request_header, "HOST: "); */
	/* strcat(request_header, hostname); */
	/* strcat(request_header, "\r\n"); */
	/* strcat(request_header,"Cache-Control: no-cache\r\n\r\n"); */
	printf("-> HTTP请求报文如下\n--------HTTP Request--------\n%s--------\n", request_header);
}

struct sockaddr_in init_serv_addr(char *address, int port) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(address);
    serv_addr.sin_port = htons(port);
    return serv_addr;
}

int http_or_https(char *url) {
    if(strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return 1;
    }else {
        return 0;
    }
}

char* extract_hostname(char *url) {
    int url_len = strlen(url);

    if(url_len >= 7 && http_or_https(url)) {
        int i = 0;
        while(url[i]!='/' && i!=url_len) {
            i += 1;
        }

        int www_len = strncmp(url, "www.", 4) == 0?4:0;
        int end = i+2;
        while(url[end] != '/') {
            ++end;
        }
        int len = end - (i+2) - www_len;
        char *ret = malloc(len);
        strncpy(ret, url + i+2 + www_len, len);
        printf("schema: http://abc.com/abc \n ret is %s\n",  ret);
        return ret;
    }else {
        // abc.com/abc
        // www.abc.com/abc
        char *ret;
        if(strncmp(url, "www.", 4) == 0) {
            ret = malloc(url_len - 4);
            strncpy(ret, url+4, url_len - 4);
        }else{
            ret = malloc(url_len - 1);
            strncpy(ret, url, url_len);
        }
        printf("schema: abc.com/abc \n ret is %s\n",  ret);
        return ret;
    }
}


int main(){
    /* char *hostname = "example.com"; */
    char *url = "http://www.man7.org/linux/man-pages/man2/recvmsg.2.html";
    url = "example.com";
    url = "http://example.com";
    /* url = "http://www.example.com"; */
    /* url = "https://www.example.com"; */
    /* url = "http://www.cr175.com/n/231.html"; */
    /* url = "zhihu.com"; */
    printf("\ncurl :  %s\n\n", url);
    char *hostname = extract_hostname(url);
    printf("hostname is %s\n", hostname);
    char *address = getIPFromDNS(hostname);

    struct sockaddr_in serv_addr = init_serv_addr(address, 80);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("connect err");
        return 0;
    }

    char request_header[200];
    generate_request_header(hostname, url, request_header);
    /* send the request */
    int total = strlen(request_header);
    int sent = 0;
    do {
        printf("total is %d \n", total);
        int bytes = write(sock, request_header + sent, total - sent);
        if (bytes < 0)
            printf("ERROR writing message to socket");
        if (bytes == 0)
            break;
        printf("wrote bytes %d \n", bytes);
        sent += bytes;
    } while (sent < total);

    char resp[10*1024];
    memset(resp, 0, sizeof(resp));
    total = sizeof(resp)-1;
    int received = 0;
    int content_len = 0;
    int body_size = 0;
    do {
        content_len = 0;
        printf("---------------\n");
        printf("start received: %d, total: %d, total - received= %d \n", received, total,
                total-received);
        int bytes = recv(sock,  resp+received, total-received, 0);
        printf("received bytes %d \n", bytes);
        if (bytes < 0) {
            printf("ERROR reading resp from socket");
        }
        if (bytes == 0) {
            printf("received bytes 0, break\n");
            break;
        }
        received += bytes;
        /* printf("\n------- resp ------- \n%s\n", resp); */
        printf("-------- sizeof(resp) %lu\n", sizeof(resp));
        int header_size = 0;
        for(; header_size<sizeof(resp) &&
                (strncmp(resp+header_size, "\r\n\r\n", strlen("\r\n\r\n")) != 0); ++header_size) {
            if(strncmp(resp+header_size, "Content-Length:", strlen("Content-Length:")) == 0) {
                int tmp = header_size + strlen("Content-Length:") + 1;
                while(resp[tmp] != '\r') {
                    content_len = content_len*10 + (resp[tmp] - '0');
                    tmp++;
                }
            }
            printf("%c", resp[header_size]);
        }
        printf("\ncontent_len %d\n", content_len);
        /* printf("\n\n&&&&&&&&&&&&&&&&&&\n"); */
        /* for(int i=header_size; i!=total; ++i) { */
        /*     printf("%c", resp[i]); */
        /* } */
        header_size += strlen("\r\n\r\n");
        body_size = content_len + header_size;
        printf("header size %d\n", header_size);
        printf("\nbody_size %d\n", body_size);
        printf("end received: %d, total: %d, total - received= %d \n", received, total,
                total-received);
    } while(received != body_size && received < total);


    if (received == total)
        printf("ERROR storing complete resp from socket");

    close(sock);

    free(address);
    return 0;
}
