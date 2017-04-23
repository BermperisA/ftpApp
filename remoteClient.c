#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>	    
#include <sys/socket.h>	     
#include <netinet/in.h>	     
#include <unistd.h>
#include <fcntl.h>          
#include <netdb.h>	         
#include <stdlib.h>	         
#include <string.h>	         
#define PATH_SIZE 1024

int	read_all ( int fd, void *buff , size_t size) {
	int sent , n;
	for (sent = 0; sent < size; sent+=n) {
		if ((n = read(fd, ((char *)buff) + sent , size -sent)) == -1)
			return -1; /* error */
		}
	return sent;
}

int	write_all ( int fd, void *buff , size_t size) {
	int sent , n;
	for (sent = 0; sent < size; sent+=n) {
		if ((n = write(fd, ((char *)buff) + sent , size -sent)) == -1)
			return -1; /* error */
		}
	return sent;
}

void nmkdir(const char *dir) {
        char tmp[PATH_SIZE], dirname[PATH_SIZE] ;
        char *p = NULL;
        size_t len;
        int j, num;
		memset(dirname, 0, sizeof(dirname));
		strcpy(dirname,dir);
		j=0;
		num = strlen(dirname) ;
		while (dirname[num - j] != '/')
		{	
			dirname[num - j] = '\0';
			j++;
		}
		dirname[num - j] = '\0';
        snprintf(tmp, sizeof(tmp),"%s",dirname);
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);        
}

int main(int argc, char *argv[]) {
    int    readnum,recnum,datanum ,fd,port,sum, sock, i,filesize, cnt, files_number;
    long addr;
    char   *hostaddr, buf[PATH_SIZE];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    i = 0;
    datanum = sysconf(_SC_PAGE_SIZE);
    char  buf2[datanum];    
    
    
    if (argc != 7) {
    	printf("Please give the correct parameters\n");
       	return 1;
    }
    for (i = 1 ; i<=5 ; i+=2)
    {
		if (strcmp(argv[i], "-i") == 0)
			hostaddr = argv[i+1];   // get hostname from arguments
		else if (strcmp(argv[i], "-p") == 0)
			port = atoi(argv[i+1]); /*Convert port number to integer*/
		else if (strcmp(argv[i], "-d") == 0)
			strcpy(buf, argv[i+1]); //get path dir
		else 
		{
			printf("Please give the correct arguments\n");
			return 1;
		}
	}
	i = 0;	
	/* Create socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    	perror("Error with socket");
    	return 1;
    }
	addr = inet_addr(hostaddr);
    if ((rem = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET)) == NULL) {	
	   perror("Error with gethostbyname"); 
	   return 1;
    }
    
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */
    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0){
	   perror("connect");
	   return 1;
    }
    
    printf("Connecting to %s port %d\n", hostaddr, port);
	if( buf[strlen(buf) -1] == '\n' )//vgazoume to \n apo to path
		buf[strlen(buf) -1] = '\0';

	if (write(sock, buf, strlen(buf)) < 0)// stelnoume to path tou dir
	   perror("write");
		
	if (read(sock, &files_number , sizeof(int))< 0)	//diavazoume posa areia perimenoume
		perror("write");
		
	while((i < files_number) && (read(sock, &cnt , sizeof(int)) > 0)) {
		memset(buf, 0, sizeof(buf)); //adeiasma tou buffer
		if (read(sock, buf , sizeof(char)*cnt) < 0)	
				perror("read");
		nmkdir(buf);//dhmiourgia katalogwn
		i++;
			
		if((fd=open (buf, O_CREAT | O_WRONLY,S_IRUSR | S_IWUSR 
			|S_IXUSR| S_IRGRP | S_IROTH|S_IXGRP|S_IXOTH))== -1){
			printf("error in opening anotherfile \n");
		}else {
			if (read(sock, &filesize , sizeof(int)) < 0)	
				perror("reading file size");
			sum = 0; // athroisma bytes gia kathe arxeio
			printf("Receiving file: %s#%d \n", buf, filesize);
			do 
			{	if (read(sock, &recnum, sizeof(int)) < 0) //diavazoume ton atihmo twn bytes pou perimenoume
					perror("reading bytes num");
		
				if ((readnum=read_all(sock, buf2, recnum)) != recnum)//koitame an ta lavame ola
					printf("Eftasan ligotera bytes\n");
				
				if (write(fd, buf2, readnum) < 0) //ta grafoume sto arxeio
					perror("writing to file");
				sum = sum + readnum;
				
			} while (sum < filesize);
			printf("Done\n");	
			if (close (fd)) {
				perror("Error ");
			} 
		}			
	}     
    close(sock);                 /* Close socket and exit */
}			     
