#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#define SIZE 1024
#define STR_ADDR "127.0.0.69"
#define PORT 1069

int main(void) {
    int fd_client[100];
    int n_client = 0;
    int pipefd[2];
    int sfd, pfd;
    pid_t pid;
    struct sockaddr_in s_addr, p_addr;     
    socklen_t p_addr_len;

    s_addr.sin_port = PORT; 
    s_addr.sin_family = AF_INET; 
    inet_aton(STR_ADDR, &s_addr.sin_addr);
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(sfd, (struct sockaddr*)&s_addr, sizeof(s_addr)) == -1) {
        perror("bind");
        goto cleanup;
    }

    if (listen(sfd, 100) == -1) {
        perror("listen");
        goto cleanup;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        goto cleanup;
    }

    for (int i = 0; i < 2; i++) {
        socklen_t s_len;
        p_addr_len = sizeof(p_addr);
        pfd = accept(sfd, (struct sockaddr*)&p_addr, &p_addr_len);

        if (pfd == -1) {
            perror("accept");
        } else {
            fd_client[n_client] = pfd; 
            n_client++;
            pid  = fork();

            if (pid == 0) {
                printf("Accepted %d\n", pfd);
                break;
            } else if (pid > 0) {
                continue;
            }
        }
    }

    while (1) {
        if (pid == 0) {
            close(pipefd[0]);
            close(sfd);
            char buffer[SIZE];    
            int n_byte_read;
            n_byte_read = read(pfd, buffer, SIZE);

            if (n_byte_read > 0) {
                int n;
                n = write(pipefd[1], buffer, n_byte_read);

                if (n == -1) {
                    perror("pipe writing");
                }
            } else if (n_byte_read == -1) {
                perror("reading from peer");
                close(pfd);
                break;
            }
        } else if (pid > 0) {
            close(pipefd[1]);
            int n_byte_read;
            char buffer[SIZE];    
            n_byte_read = read(pipefd[0], buffer, SIZE);

            if (n_byte_read >= 0) {
                printf("writing to peers...\n");

                if (n_client < 0) {
                    wait(NULL);
                    break;
                }

                for (int i = n_client; 0 < i; i--) {
                    int n = 0; 
                    n = write(fd_client[i - 1], buffer, n_byte_read);

                    if (n == -1) {
                        fd_client[i - 1] = fd_client[i - 2];
                        n_client--;
                    } 
                }
            }
        } else if (pid == -1) {
            perror("fork");
        }
    }

    // yep, goto >>> all
cleanup: 
    if (sfd == -1)
        close(sfd);
    return EXIT_FAILURE;
}
