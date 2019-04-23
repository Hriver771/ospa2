#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <ctype.h>
#include <unistd.h>

int 
main(int argc, char const *argv[]) 
{ 
	struct sockaddr_in serv_addr; 
	int sock_fd ;
	int s, len ;
	char buffer[1024] = {0}; 
	char * data ;
	int portnumber = atoi(argv[2]);
	char IP[50];
	strcpy(IP, argv[1]);

	

	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(portnumber); 
	if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	bzero(buffer, 1023);

	printf("here11");

	FILE *f ;
	int words = 0;

	char c;
	f = fopen("example.c", "r") ;
	while((c=getc(f)) != EOF ) {
		fscanf(f, "%s", buffer );
		if(isspace(c)|| c=='\t')
		words++;
	}
	printf("here");

	send(sock_fd, &words, sizeof(int), 0);
	rewind(f);

	char ch;
	while(ch != EOF ) {
		fscanf(f, "%s", buffer);
		send(sock_fd, buffer, 1023, 0);
		ch = fgetc(f);
	}
	printf("send successfully.");
 
	shutdown(sock_fd, SHUT_WR) ;

	char buf[1024] ;
	data = 0x0 ;
	len = 0 ;
	while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}

	}
	printf(">%s\n", data); 

}
