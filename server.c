#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>
#include <sys/types.h>

#include <pthread.h>

#include <time.h>

#include <malloc.h>

#include <assert.h>

#include "header.h"

#define MAX_CLIENTS_COUNT 100


static unsigned int clients_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
} clients_t;

clients_t *clients[MAX_CLIENTS_COUNT];
void add_q(clients_t *client){
	pthread_mutex_lock(&mutex);

	for(int i=0; i < MAX_CLIENTS_COUNT; ++i){
		if(!clients[i]){
			clients[i] = client;
			break;
		}
	}

	pthread_mutex_unlock(&mutex);
}

int Read(int fd, char *buffer, int size);

int Write(int fd, char *buffer, int size);

void remove_q(int sockfd){
	pthread_mutex_lock(&mutex);

	for(int i=0; i < MAX_CLIENTS_COUNT; ++i){
		if(clients[i]){
			if(clients[i]->sockfd == sockfd){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&mutex);
}

void notify_all(char *message){

    pthread_mutex_lock(&mutex);
    for(int i=0; i<MAX_CLIENTS_COUNT; ++i){
		if(clients[i]){
            uint32_t message_size = strlen(message);

            uint32_t size_mes_network=htonl(message_size);
            /* Send message to the server */
            int32_t result = Write(clients[i]->sockfd,(void*)&size_mes_network,4);
            if(result==0){
                result = Write(clients[i]->sockfd, message, message_size);   
            }
            else{
                printf("ERROR 1");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    
}


void *client_handler(void *arg){
    int32_t result = 0;
    uint32_t nick_size;
    //char *buffer;
    int leave_flag = 0;
    char *buffer;


	clients_count++;
    clients_t *cli = (clients_t *)arg;
    while(1){
        if (leave_flag) {
			break;
		}
        result = Read(cli->sockfd, (void*)&nick_size, 4);
        if(result==EXIT_SUCCESS){
            uint32_t nick_size_host = ntohl(nick_size);
            char* nick = malloc(sizeof(char)*(nick_size_host+1));
            bzero(nick,nick_size_host+1);
            result = Read(cli->sockfd, nick, nick_size_host);
            if(result==EXIT_SUCCESS){
                uint32_t message_size;
                result = Read(cli->sockfd, (void*)&message_size, 4);
                if(result==EXIT_SUCCESS){
                    uint32_t message_size_host = ntohl(message_size);
                    buffer = (char *) calloc((message_size_host+1) , sizeof(char));
                    //char * buffer = malloc(message_size_host+1);
                    result = Read(cli->sockfd, buffer, message_size_host);
                    if(result==EXIT_SUCCESS){
                        char *date = malloc(sizeof(int)*4);
                        bzero(date,4);
                        time_t t = time(NULL);
                        struct tm* lt = localtime(&t);
                        int hh = lt->tm_hour;
                        int mm = lt->tm_min;
                        sprintf(date,"%02d:%02d",hh, mm);
                        notify_all(nick);
                        notify_all(buffer);
                        notify_all(date);
                        free(buffer);
                        free(date);
                        free(nick);
                    }
                }
            }
        }

        else if (result != 0){
            printf("EOF\n");
            leave_flag = 1;
        } 
    }

    remove_q(cli->sockfd);
	close(cli->sockfd);
    printf("Socket %d closed\n",cli->sockfd);
  
    free(cli);
    clients_count--;
    
    return NULL;
}


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    int sockfd, newsockfd;
    uint16_t portno;
    pthread_t tid;

    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        //exit(1);
    }
    else{

        /* Initialize socket structure */
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = atoi(argv[1]);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        
        int value = 1;
        int res = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)); 
        if (res == -1) {
            perror("setsockopt1");
            exit(1);
        }

        /* Now bind the host address using bind() call.*/
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("ERROR on binding");
            //exit(1);
        }
        else{

        /* Now start listening for the clients, here process will
        * go in sleep mode and will wait for the incoming connection
        */

        listen(sockfd, 5);

        /* Accept actual connection from the client */
        while(1){
            socklen_t clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if((clients_count + 1) == MAX_CLIENTS_COUNT){
                printf("Max clients reached.");
                close(newsockfd);
                continue;
                }

            if (newsockfd < 0) {
                perror("ERROR on accept");
                //exit(1);
            }
            else{
            clients_t *cli = (clients_t *)malloc(sizeof(clients_t));
                cli->address = cli_addr;
                cli->sockfd = newsockfd;
                add_q(cli);
                printf("Socket %d openned\n", cli->sockfd);
                pthread_create(&tid,NULL,&client_handler,(void*)cli);
                pthread_detach(tid);
                }
        
            }
        }
    }

    return 0;
}
