#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
int main(){
    /* char *address = "127.0.0.1"; */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* serv_addr.sin_addr.s_addr = inet_addr(address); */
    serv_addr.sin_port = htons(12345);

    if ((bind(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr))) == -1) {
        fprintf(stderr, "Error on bind --> %s", strerror(errno));

        exit(EXIT_FAILURE);
    }
    printf("bind succeed\n");

    /* Listening to incoming connections */
    if ((listen(sock, 5)) == -1) {
        fprintf(stderr, "Error on listen --> %s", strerror(errno));

    }
    printf("listen succeed\n");

    struct sockaddr* client_address;
    int client_sock;
    char buf[100];
    char* request_header[10];
    while(1) {
        printf("inside\n");
        socklen_t client_address_size = sizeof(client_address);
        client_sock = accept(sock, (struct sockaddr*) &client_address, &client_address_size);

        if (client_sock == -1) {
            printf("accept error");
        }

        printf("read\n");
        read(client_sock, buf, 80);
        for(int i=0; i<10; ++i) {
            request_header[i] = (char*)malloc(100);
        }
        /* memcpy(request_header[0], "abc", 3); */
        /* printf("request_header[0] %s\n",request_header[0]); */
        int header_count =  0;
        int last_position =  0;
        printf("start loop\n");
        for(int i=0; i<strlen(buf); ++i) {
            if(i+1<strlen(buf) && buf[i]=='\r' && buf[i+1]=='\n') {
                i += 1;
                printf("%c", buf[i]);
                printf("copy: %s ---",buf+last_position);
                memcpy(request_header[header_count++], buf+last_position, i-last_position);
                last_position = i;
            }else {
                printf("%c", buf[i]);
            }
        }

        printf("\n=========================\n");
        for(int i=0; i<10; ++i) {
            printf("request are %s\n", request_header[i]);
        }
    }

    /* close(sock); */
}
