//
//  sampleworker.c
//  
//
//  Created by 하민지 on 27/04/2019.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

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

//char
//*file_to_data(char * file_name)
//{
//
//}

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
*new_thread(void *arg)
{
    int conn = *((int *)arg);
    data = recv_message(conn) ;
    
    char * ptr;
    ptr = strtok(data, "-");
    char *sid = (char *)malloc(sizeof(char)*strlen(ptr)) ;
    strcpy(sid, ptr);
    ptr = strtok(NULL, "::-+_");
    
    FILE *fp;
    char *code_file_name = (char *)malloc(sizeof(char)*11) ;
    strcpy(code_file_name, sid);
    strcat(code_file_name, ".c") ;
    fp = fopen(code_file_name, "w");
    fprintf(fp, "%s", ptr);
    fclose(fp);
    
    char * error_file_name = (char *)malloc(sizeof(char)*13) ;
    strcpy(error_file_name, sid);
    strcat(error_file_name, ".err") ;
    
    char * command = (char *)malloc(sizeof(char)*(strlen(code_file_name)+strlen(error_file_name) +9)) ;
    strcpy( command, "gcc" ) ;
    strcat( command, code_file_name ) ;
    strcat( command, " &> " ) ;
    strcat( command, error_file_name ) ;
    
    strcpy("gcc ", code_file_name) ;
    system("%s &> error", commad); -> 이렇게 하면 안될꺼 같은데 그냥 파일 열어서 확인하는게 좋을 듯.
    if( fopen("error") != NULL ) {
        send_file(conn, "error");
        free(data) ;
        free(command) ;
        return ;
    }
    else {
        for( int i=1; i<=10; i++ ) {
            system( "cat %d.in | ./a.out > %d.out", i, i ) ;
        }
        
        for( int i=1; i<=10; i++ ) {
            send_file(conn, "%d.out", i) ;
        }
        shutdown(conn, SHUT_WR) ;
        free(data) ;
        free(command) ;
    }
}

int
main(int argc, char const *argv[])
{
    int listen_fd, new_socket ;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int port_number ;
    int c ;
    
    while( (c = getopt(argc, argv, "p: ")) != -1) {
        switch (c) {
            case 'p':
                port_number = atoi(optarg) ;
                break;
            default:
                perror("wrong format : ");
                exit(EXIT_FAILURE);
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
    address.sin_port = htons(port_number);
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed : ");
        exit(EXIT_FAILURE);
    }
    
    pthread_t tid[2];
    int i = 0 ;
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
        
        if( pthread_create(&tid[i], NULL, new_thread, &new_socket) != 0 )
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
