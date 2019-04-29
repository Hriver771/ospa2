//
//  filnalWorker.c
//  
//
//  Created by 하민지 on 29/04/2019.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

void
child_proc(int conn)
{
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
}

int
main(int argc, char const *argv[])
{
    pid_t child_pid[10] ;
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
    address.sin_port = htons(8080);
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed : ");
        exit(EXIT_FAILURE);
    }
    
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
        
        int conn = new_socket ;
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
        
        
        char * command = (char *)malloc(sizeof(char)*(strlen(code_file_name)+4)) ;
        strcpy("gcc ", code_file_name) ;
        system("%s &> error", commad);
        if( fopen("error", "r") != NULL ) {
            send_file(conn, "error") ;
            close(new_socket) ;
            return 1;
        }
        else {
            for(int i=0; i<10; i++) {
                switch(pid[i]=fork()) {
                    case -1:
                        perror("Error message ");
                        exit(EXIT_FAILURE);
                        break;
                    case 0: //chid process
//                        run_file(input, file)
                        return ; // 이렇게 해도 괜찮나? 알 수 없음. 
                    default: //parent procecc
                        break;
                }
                pid[i] = fork()
            }
            
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
        
        if (fork() > 0) {
            child_proc(new_socket) ;
        }
        else {
            close(new_socket) ;
        }
        //여기서 pid다 초기화하고 파일 다 지우고 돌아가야함. 그냥 fork 하지말고 원래대로 쓸까? 그냥 보고서에만 쓰고.. 모르겠다. 뭘 어떻게 해야해.
    }
}
