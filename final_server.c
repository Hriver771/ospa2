#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <arpa/inet.h>

int submitter_port ;
char * worker_ip ;
int worker_port ;
char * file_dir ;

void
recv_file(int conn, char * file_name)
{
    char buf[1024] ;
    char * data = 0x0 ;
    int len = 0 ;
    int s ;
    
    FILE *fp;
    fp = fopen(file_name, "w");
    
    while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
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
    
    fprintf(fp, "%s", data);
    printf(">%s\n", data) ;
    
    fclose(fp);
}

void
send_file( int conn, char * file_name )
{
    int s, len ;
    char buffer[1024] = {0};
    char * data ;
    FILE *fp ;
    
    fp = fopen(file_name, "r") ;
    if (fp == NULL){
        printf("Error! opening file");
        exit(1);
    }
    
    while( !feof( fp ) )
    {
        if(fgets(buffer, sizeof(buffer), fp) == NULL) break;
        
        data = buffer ;
        len = strlen(buffer) ;
        s = 0 ;
        while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
            data += s ;
            len -= s ;
        }
        
        printf( "%s", buffer );
    }
    fclose( fp );
}

char
*recv_message(int conn)
{
    int s, len ;
    char buf[1024] ;
    char * data = 0x0 ;
    len = 0 ;
    
    while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
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
    return data ;
}

void
send_message(int conn, char * buffer)
{
    char * data;
    data = (char *)malloc(sizeof(char)*(strlen(buffer)+1)) ;
    strcpy(data, buffer);
    int len ;
    len = strlen(data) ;
    int s = 0 ;
    while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
        data += s ;
        len -= s ;
    }
}

void
*socket_thread(void *arg)
{
    int conn = *((int *)arg);
//    char * data ;
//    data = recv_message(conn) ;
//    send_message(conn, data) ;
//    shutdown(conn, SHUT_WR) ;
//    free(data) ;
    
    char * data = recv_message(conn) ;
    char * ptr;
    ptr = strtok(data, "-");
    char *sid = (char *)malloc(sizeof(char)*strlen(ptr)) ;
    strcpy(sid, ptr);
    ptr = strtok(NULL, "-");
    char * password = (char *)malloc(sizeof(char)*strlen(ptr)) ;
    strcpy(password, ptr) ;
    ptr = strtok(NULL, "::-+_");
    
    FILE *fp;
    char *code_file_name = (char *)malloc(sizeof(char)*11) ;
    strcpy(code_file_name, sid);
    strcat(code_file_name, ".c") ;
    fp = fopen(code_file_name, "w");
    fprintf(fp, "%s", ptr);
    fclose(fp);
    
//    send_message(conn, sid) ;
//    char * password = recv_message(conn) ;
//    send_message(conn, password) ;
//    recv_file(conn, strcat(sid, ".c")) ;
    char str[] = "recived!\n" ;
    send_message(conn, str) ;
//    shutdown(conn, SHUT_WR) ;
//
    printf("%s %s\n", sid, password) ;
    
    
    struct sockaddr_in thread_serv_addr;
    int thread_sock_fd ;
    char thread_buffer[1024] = {0};
    
    thread_sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if (thread_sock_fd <= 0) {
        perror("thread socket failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    memset(&thread_serv_addr, '0', sizeof(thread_serv_addr));
    thread_serv_addr.sin_family = AF_INET;
    thread_serv_addr.sin_port = htons(worker_port);
    if (inet_pton(AF_INET, worker_ip, &thread_serv_addr.sin_addr) <= 0) {
        perror("thread inet_pton failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    if (connect(thread_sock_fd, (struct sockaddr *) &thread_serv_addr, sizeof(thread_serv_addr)) < 0) {
        perror("thread connect failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    char *out_file_name = (char *)malloc(sizeof(char)*14) ;
    strcpy(out_file_name, "out");
    strcat(out_file_name, sid) ;
    strcat(out_file_name, ".c") ;
    printf("%s", out_file_name) ;
    
    send_message(thread_sock_fd, sid) ;
    send_message(thread_sock_fd, "-") ;
    send_file(thread_sock_fd, code_file_name);
    shutdown(thread_sock_fd, SHUT_WR) ;
    recv_file(thread_sock_fd, out_file_name) ;
    send_file(conn, code_file_name) ;
    shutdown(conn, SHUT_WR) ;
    
}

int 
main(int argc, char const *argv[]) 
{
    int listen_fd, new_socket ;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int c ;
    char *p ;
    
    while( (c = getopt(argc, argv, "p:w: ")) != -1) {
        switch (c) {
            case 'p':
                submitter_port = atoi(optarg) ;
                break;
            case 'w':
                p = strtok(optarg, ":");
                worker_ip = (char *)malloc(sizeof(char)*strlen(p)) ;
                strcpy(worker_ip, p) ;
                p = strtok(NULL, ":");
                worker_port = atoi(p) ;
                break ;
                
            default:
                file_dir = (char *)malloc(sizeof(char)*strlen(optarg)) ;
                strcpy(file_dir, optarg) ;
                
        }
    }
    
	char buffer[1024] = {0}; 

	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(submitter_port);
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	}
    
    pthread_t tid[2];
    int i = 0;
	while (1) {
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
        if( pthread_create(&tid[i], NULL, socket_thread, &new_socket) != 0 )
            printf("Failed to create thread\n");
        if( i >= 2)
        {
            i = 0;
            while(i < 2)
            {
                pthread_join(tid[i++],NULL);
            }
            i = 0;
        }
	}
}
