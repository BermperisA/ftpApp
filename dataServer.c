#include <stdio.h>
#include <sys/wait.h>	     
#include <sys/types.h>	     
#include <sys/socket.h>	    
#include <netinet/in.h>	    
#include <netdb.h>	         
#include <unistd.h>	         
#include <stdlib.h>	         
#include <ctype.h>	         
#include <signal.h>  
#include "producer-consumer.h"




void sig_hanlder (int signal) {
	
	
	flag = 1;

}

int main(int argc, char *argv[]) {
    
	int thread_num, i, stats, x;
	int port, sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;
    struct sigaction sa;
    
    stats = 0;
    flag = 0;
    if (argc != 7) {
    	printf("Please give all the parameters\n");
       	return 1;
    }
    for (i = 1 ; i<=5 ; i+=2)
    {
		if (strcmp(argv[i], "-s") == 0)
			thread_num = atoi(argv[i+1]);   // worker threads num
		else if (strcmp(argv[i], "-p") == 0)
			port = atoi(argv[i+1]); /*Convert port number to integer*/
		else if (strcmp(argv[i], "-q") == 0)
			x = atoi(argv[i+1]); //get queue size
		else 
		{
			printf("Please give the correct arguments\n");
			return 1;
		}
	}
	pthread_t cons[thread_num], prod; 
    
    sa.sa_handler = &sig_hanlder;
    sigfillset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    printf("(To stop the server type: \"KILL -10 %d )\n", getpid());
    
    
    initialize(&pool, x);
    printf("Server successfully initialized.\n");
    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);
	for (i=0 ; i < thread_num ; i++)
	{ 
		pthread_create(&cons[i], 0, consumer, 0);
	}

    /* Create socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        return 1;
    }
    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);      /* The given port */
    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0){
        perror("bind");
        return 1;
    }
    /* Listen for connections */
    if (listen(sock, 5) < 0) {
		perror("listen");
		return 1;
    }
    printf("Listening for connections to port %d\n", port);
    while (flag == 0) 
    {   
		clientlen = sizeof(client);
        /* accept connection */
    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0) 
		{	perror("accept");
			break;
		}
		pthread_create(&prod, 0, producer, &newsock);
    	printf("Accepted connection\n");
    	if (pthread_detach(prod) !=0)
			printf("Error detaching producer\n");
    	stats++;
    } 
    
	place(&pool, NULL);
	pthread_cond_broadcast(&cond_nonempty);
    for (i=0 ; i < thread_num ; i++)
	{ 		
		pthread_join(cons[i], 0);	
	}
	free(pool.queue);
	pool.queue = NULL;
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&mtx);
    
    return 0;
}

