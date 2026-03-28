#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define SIZE 1024
#define STR_ADDR "127.0.0.69"
#define PORT 1069

int main(void) {
    int cfd;
    socklen_t p_addr_len;
    struct sockaddr_in p_addr;
    
    p_addr.sin_port = PORT;
    p_addr.sin_family = AF_INET;
    inet_aton(STR_ADDR, &p_addr.sin_addr);
    cfd = socket(AF_INET, SOCK_STREAM, 0);

    if (cfd == -1) {
        perror("socket");
        goto cleanup;
    }

    if (connect(cfd, (struct sockaddr*)&p_addr, sizeof(p_addr)) == -1) {
        perror("connect");
    } else {
        printf("Connected...\n");
        while (1) {
            int n_byte_read = 0;
            int n = 0;
            char buffer[SIZE];
            n_byte_read = read(STDIN_FILENO, buffer, SIZE);

            if (n_byte_read > 0) {
                int n = 0;
                n = write(cfd, buffer, n_byte_read);

                if (n == -1) {
                    perror("writing to the peer");
                    goto cleanup;
                }

            } else if (n_byte_read == 0) {
                continue;
            }

            n = read(cfd, buffer, SIZE);

            if (n >= 0) {
                write(STDOUT_FILENO, buffer, n);    
            }
        }

    }

    goto cleanup;

cleanup: 
    if (cfd == 0) 
        close(cfd);
    return EXIT_FAILURE;
}
