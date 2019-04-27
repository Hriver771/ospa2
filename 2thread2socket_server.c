//gcc 2thread2socket_server.c -o server
//./server -p <Port> -w <IP>:<WPort> <Dir>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int submitter_port ;
char * worker_ip ;
int worker_port ;
char * file_dir ;

void
*socket_thread(void *arg)
{
    int conn = *((int *)arg);
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ;
    

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
	printf(">%s\n", data) ;
	
	orig = data ;
	while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}
	shutdown(conn, SHUT_WR) ;
	if (orig != 0x0) 
		free(orig) ;
    
    struct sockaddr_in thread_serv_addr;
    int thread_sock_fd ;
    int thread_s, thread_len ;
    char thread_buffer[1024] = {0};
    char * thread_data ;
    
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
    
    //strcpy(thread_buffer, "hello" ) ;
    printf(">") ;
    scanf("%s", thread_buffer) ;
    
    thread_data = thread_buffer ;
    thread_len = strlen(thread_buffer) ;
    thread_s = 0 ;
    while (thread_len > 0 && (thread_s = send(thread_sock_fd, thread_data, thread_len, 0)) > 0) {
        thread_data += thread_s ;
        thread_len -= thread_s ;
    }
    
    shutdown(thread_sock_fd, SHUT_WR) ;
    
    char thread_buf[1024] ;
    thread_data = 0x0 ;
    thread_len = 0 ;
    while ( (thread_s = recv(thread_sock_fd, thread_buf, 1023, 0)) > 0 ) {
        thread_buf[thread_s] = 0x0 ;
        if (thread_data == 0x0) {
            thread_data = strdup(thread_buf) ;
            thread_len = thread_s ;
        }
        else {
            thread_data = realloc(thread_data, thread_len + thread_s + 1) ;
            strncpy(thread_data + thread_len, thread_buf, thread_s) ;
            thread_data[thread_len + thread_s] = 0x0 ;
            thread_len += thread_s ;
        }
        
    }
    printf(">%s\n", thread_data);
    
}

int 
main(int argc, char const *argv[]) 
{
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
    
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 

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
