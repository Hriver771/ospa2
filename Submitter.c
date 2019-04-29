#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

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

void
recv_message(int conn)
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

int
main(int argc, char const *argv[])
{
    char * ip ;
    int port ;
    char *sid ;
    char *password ;
    int c ;
    char * ptr ;
    char * file_name ;
    
    while( (c = getopt(argc, argv, "n:u:k: ")) != -1) {
        switch (c) {
            case 'n':
                ptr = strtok(optarg, ":");
                ip = (char *)malloc(sizeof(char)*strlen(ptr)) ;
                strcpy(ip, ptr) ;
                ptr = strtok(NULL, ":");
                port = atoi(ptr) ;
                break ;
            case 'u':
                sid = optarg ;
                break ;
            case 'k':
                password = optarg ;
        }
    }
    file_name = (char *)malloc(sizeof(char)*strlen(optarg)) ;
    strcpy(file_name, argv[optind]) ;
    
    printf("%s %s %s\n", sid, password, file_name) ;
    
    struct sockaddr_in serv_addr;
    int sock_fd ;
    int s, len ;
    char buffer[1024] = {0};
    //char * data ;
    
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
    
    //send_file(file_name) ;
    printf(">") ;
    //scanf("%s", buffer) ;
    //send_message(sock_fd, buffer) ;
    
    send_message(sock_fd, sid) ;
    send_message(sock_fd, "-") ;
    send_message(sock_fd, password) ;
    send_message(sock_fd, "-") ;
    send_file(sock_fd, file_name) ;
    shutdown(sock_fd, SHUT_WR) ;

    recv_message(sock_fd) ;
    
}
