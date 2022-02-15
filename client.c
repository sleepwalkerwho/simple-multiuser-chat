#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>

#include <pthread.h>

#include <assert.h>
#include "header.h"


#define h_addr  h_addr_list[0]

int sockfd = 0;
char *name;
uint32_t flag = 0;
pthread_mutex_t mutex;



int Read(int fd, char *buffer, int size);
int Write(int fd, char *buffer, int size);


void send_message(char* message){
    uint32_t message_size = strlen(message);

    uint32_t size_mes_network=htonl(message_size);
    int32_t n = Write(sockfd,(void*)&size_mes_network,4);
    if(n==0){
        n = Write(sockfd, message, message_size);
        if (n!=0) {
            perror("ERROR writing to socket");

        }
    }
}
void* handler(){
    size_t n = 0, res;
    char *buffer;
    printf("Press m to send a message\n");
    while(1){
        char Button_m [10];

        int h = scanf("%[^\n]s", Button_m);
        h = scanf("%*c");
        if (h < 0) {
            perror("ERROR on scanf");
            exit(1);
    }
    
        
        if(strcmp(Button_m, "m\0") == 0) {
            pthread_mutex_lock(&mutex);
            printf("Please enter the message: \n");

            res = getline(&buffer, &n, stdin);

            pthread_mutex_unlock(&mutex);
             if (res == 0) {
                    perror("Press any message!");
                }
            else{
                send_message(name);
                send_message(buffer);
            }
            
        }
        else{
            printf("Press m to send a message\n");
        }
}
    return NULL;
}

void* receive_message(){
    uint32_t result = 0;
    uint32_t nick_size;
    char *buffer;
    while(1){
    
    result = Read(sockfd, (void*)&nick_size, 4);
    if(result==EXIT_SUCCESS){
        uint32_t nick_size_host = ntohl(nick_size);
        char* nick = malloc(sizeof(char)*(nick_size_host+1));
        bzero(nick,nick_size_host+1);
        result = Read(sockfd, nick, nick_size_host);
        if(result==EXIT_SUCCESS){
            uint32_t message_size;
            result = Read(sockfd, (void*)&message_size, 4);
            if(result==EXIT_SUCCESS){
                uint32_t message_size_host = ntohl(message_size);
                buffer = (char *) calloc((message_size_host+1) , sizeof(char));
                result = Read(sockfd, buffer, message_size_host);
                if(result==EXIT_SUCCESS){
                    uint32_t date_size;
                    result = Read(sockfd, (void*)&date_size, 4);
                    if(result==EXIT_SUCCESS){
                        uint32_t date_size_host = ntohl(date_size);
                        char* date = malloc(sizeof(char)*(date_size_host));
                        bzero(date,date_size_host);
                        result = Read(sockfd, date, date_size_host);
                        pthread_mutex_lock(&mutex);
                        printf("{%s} [%s] %s",date,nick, buffer);
                        pthread_mutex_unlock(&mutex);
                        //free(buffer);
                        free(nick);
                        free(date);
                    }
                }
            }
        }
    }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    

    
    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);
    name = argv[3];

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))< 0) {
        perror("ERROR connecting");
        exit(1);
    }

    pthread_t send_msg_thread;
    pthread_create(&send_msg_thread, NULL, handler, NULL);

    pthread_t recv_msg_thread;
    pthread_create(&recv_msg_thread, NULL, receive_message, NULL);

    pthread_join(send_msg_thread, NULL);
    pthread_join(recv_msg_thread, NULL);

    close(sockfd);
    return EXIT_SUCCESS;
}
