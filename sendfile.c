#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void
send_file(int conn, char filename[])
{
    int s, len ;
    char buffer[1024] = {0};
    char * data ;
    FILE *fp ;
    
    fp = fopen(filename, "r") ;
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

void
recv_message(int conn)
{
    char buf[1024] ;
    char * data = 0x0 ;
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
    printf(">%s\n", data);
}


int
main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    int sock_fd ;
    char ip[50];
    strcpy(ip, argv[1]);
    int port = atoi(argv[2]) ;
    char filename[50];
    strcpy(filename, argv[3]) ;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if (sock_fd <= 0) {
        perror("socket failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    send_file(sock_fd, filename) ;
    shutdown(sock_fd, SHUT_WR) ;
    recv_message(sock_fd) ;
    
}
