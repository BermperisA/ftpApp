#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include "producer-consumer.h"


int	write_all ( int fd, void *buff , size_t size) {
	int sent , n;
	for (sent = 0; sent < size; sent+=n) {
		if ((n = write(fd, ((char *)buff) + sent , size -sent)) == -1)
			return -1; /* error */
		}
	return sent;
}

int	read_all ( int fd, void *buff , size_t size) {
	int sent , n;
	for (sent = 0; sent < size; sent+=n) {
		if ((n = read(fd, ((char *)buff) + sent , size -sent)) == -1)
			return -1; /* error */
		}
	return sent;
}

void list_dir (const char * dir_name, int sock, pthread_mutex_t * mtx_s, int * number)
{
    DIR * d;
	struct stat s;
	data * d1;
	struct dirent * entry;
    const char * d_name;
	char buf[PATH_SIZE];

    memset(buf, 0, sizeof(buf)); //adeiasma tou buffer

    if ((d = opendir (dir_name))== NULL) {
        perror("Error ");
    }
    else {
		while ((entry = readdir (d)) != NULL) {       
			sprintf(buf,"%s/%s",dir_name,entry->d_name);
			if( stat(buf,&s) == 0 )
			{
				if( s.st_mode & S_IFDIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
				{	// einai directory
					list_dir(buf, sock, mtx_s, number);
				}
				else if( s.st_mode & S_IFREG ) //einai arxeio
				{	if (number == NULL)	 
					{	d1 = (data *) malloc(sizeof(data));
						if (d1==NULL)	
							printf("Error with malloc\n"); 
						strcpy(d1->str,buf); // vazoume to path tou arxeiou sthn domh 
						d1->sock = sock;	// to socket
						d1->mtx_s = mtx_s;  // kai ton mutex tou socket
						place(&pool, d1);  //kai to stelnoume sthn oura
						pthread_cond_broadcast(&cond_nonempty);
						d1=NULL;
					}
					else // metrame posa arxeia periexei o katalogos
						(*number)++; 					
				}
			}
		}
		if (closedir (d)) {
			perror("Error ");
		}    
   }

}

void initialize(pool_t * pool, int x) {
        int i;
        queue_size = x;
        pool->queue=(data **) malloc(queue_size*sizeof(data *));
		if (pool->queue==NULL)	
			printf("Error with malloc\n");
        for (i=0 ; i < queue_size ; i++)
			pool->queue[i] = NULL;
        pool->start = 0;
        pool->end = -1;
        pool->count = 0;
}

void place(pool_t * pool, data * d1) {
        pthread_mutex_lock(&mtx);
        while (pool->count >= queue_size) {
                printf(">> Found Buffer Full \n");
                pthread_cond_wait(&cond_nonfull, &mtx);
        }
        pool->end = (pool->end + 1) % queue_size;
        pool->queue[pool->end] = d1;
        pool->count++;
        pthread_mutex_unlock(&mtx); 
}

data * obtain(pool_t * pool) {
        data * d1;
        pthread_mutex_lock(&mtx);
        while (pool->count <= 0) {
                printf(">> Found Buffer Empty \n");
                pthread_cond_wait(&cond_nonempty, &mtx);
        }   
		d1 = pool->queue[pool->start];
		pool->start = (pool->start + 1) % queue_size;
		pool->count--;
		pthread_mutex_unlock(&mtx);
    
        return d1;
        
}

void * producer(void * ptr)
{		
	int newsock, number, flag;
	char buf[PATH_SIZE];
	pthread_mutex_t * mtx_s;
	newsock = * ((int*) ptr);
	number = 0;
	flag = 0;
    while(read(newsock, buf, PATH_SIZE) > 0) {  
    	mtx_s = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t)); // ena mutex gia kathe socket
    	if (mtx_s ==NULL)	
			printf("Error with malloc\n");
    	pthread_mutex_init(mtx_s, 0);
    	printf("[Thread: %ld] Scanning directory %s\n", pthread_self(), buf);
    	list_dir(buf, newsock, mtx_s, &number);
    	if (write(newsock, &number , sizeof(int)) < 0)// stelnoume ston client twn arithmo twn arxeiwn pou perimenei
			perror("write");
    	list_dir(buf, newsock, mtx_s, NULL); 		
		memset(buf, 0, sizeof(buf));
		flag = 1;
	}
    close(newsock);	  /* Close socket */
    printf("Socket closed!\n");
    if (flag == 1)
		pthread_mutex_destroy(mtx_s);
    pthread_exit(0);
}

void * consumer(void * ptr)
{	
	
	data * d1;
	int cnt, fd, readnum, size;
	struct stat st;
	long datanum;
	datanum = sysconf(_SC_PAGE_SIZE);
	char  buf[datanum];
    while ( flag == 0 ) {
        d1 = obtain(&pool);
        if (d1 == NULL)
			break;
		pthread_cond_broadcast(&cond_nonfull);
		cnt = strlen(d1->str);
        pthread_mutex_lock(d1->mtx_s);
        if (write(d1->sock, &cnt , sizeof(int)) < 0) //grafoume to mhkos tou path
			perror("write");
		if (write(d1->sock, d1->str, strlen(d1->str)) < 0)// stelnoume to path
			perror("write");	
		printf("[Thread: %ld] Received task: <%s, %d>\n",pthread_self(), d1->str, d1->sock);
		
		if((fd=open (d1->str, O_RDONLY))== -1){ //anoigma arxeiou gia diavasma
			printf("error in opening anotherfile \n");
		}
		
		stat(d1->str, &st);
		size = st.st_size;
		if (write(d1->sock, &size , sizeof(int)) < 0) //stelnoume to megethos tou arxeiou
			perror("write");
		printf("[Thread: %ld] Sending file: %s#%d \n",pthread_self(), d1->str, size);
		while ((readnum = read(fd, buf, datanum)) > 0 )// diavazoume apo to arxeio
		{	
			if (write(d1->sock, &readnum, sizeof(int)) < 0)//stelnoume posa dedomena 8a grapsoume
				perror("write");
							
			if (write_all(d1->sock, buf, readnum) != readnum)// koitame an stalthikan ontws osa upologizame
				perror("write");
		}
		if (close (fd)) {
			perror("Error ");
		}
		printf("[Thread: %ld] Done\n",pthread_self()); 
		pthread_mutex_unlock(d1->mtx_s);
		free(d1);	
		d1 = NULL;
        
	}
	place(&pool, NULL);
	pthread_cond_broadcast(&cond_nonempty);
	printf("[Thread: %ld] Terminating\n",pthread_self());
	pthread_exit(0);
    
}




