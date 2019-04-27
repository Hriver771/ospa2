#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    char * ip ;
    int port ;
    int sid ;
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
                sid = atoi(optarg) ;
                break ;
            case 'k':
                password = optarg ;
        }
    }
    file_name = (char *)malloc(sizeof(char)*strlen(optarg)) ;
    strcpy(file_name, argv[optind]) ;
    
    printf("%d %s %s\n", sid, password, file_name) ;
    
    struct sockaddr_in serv_addr;
    int sock_fd ;
    int s, len ;
    char buffer[1024] = {0};
    char * data ;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if (sock_fd <= 0) {
        perror("socket failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed : ") ;
        exit(EXIT_FAILURE) ;
    }
    
    scanf("%s", buffer) ;
    
    data = buffer ;
    len = strlen(buffer) ;
    s = 0 ;
    while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
        data += s ;
        len -= s ;
    }
    
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
