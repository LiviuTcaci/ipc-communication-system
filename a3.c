#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>

#define RESP_PIPE "RESP_PIPE_45862"
#define REQ_PIPE "REQ_PIPE_45862"
#define VARIANT_NUMBER 45862
#define SHM_NAME "/9P8eB2"

void *shm_addr = NULL;
size_t shm_size = 0;
int shm_fd = -1;
void *file_addr = NULL;
size_t file_size = 0;

void create_and_open_pipes(int *resp_fd, int *req_fd) {
    // Create the response pipe
    if (mkfifo(RESP_PIPE, 0644) == -1) {
        perror("cannot create the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Open the request pipe for reading
    *req_fd = open(REQ_PIPE, O_RDONLY);
    if (*req_fd == -1) {
        perror("cannot open the request pipe");
        unlink(RESP_PIPE);
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Open the response pipe for writing
    *resp_fd = open(RESP_PIPE, O_WRONLY);
    if (*resp_fd == -1) {
        perror("cannot open the response pipe");
        close(*req_fd);
        unlink(RESP_PIPE);
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }
}

void send_connect_message(int resp_fd) {
    const char *connect_message = "CONNECT";
    size_t message_length = strlen(connect_message);

    // Send the length of the message as a single byte
    unsigned char length_byte = (unsigned char)message_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the message length to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Send the actual message
    if (write(resp_fd, connect_message, message_length) == -1) {
        perror("cannot write the connect message to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }
}

void handle_echo_request(int resp_fd) {
    const char *echo_response = "ECHO";
    const char *variant_response = "VARIANT";
    size_t echo_length = strlen(echo_response);
    size_t variant_length = strlen(variant_response);

    // Send the length of the echo response message as a single byte
    unsigned char length_byte = (unsigned char)echo_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the echo message length to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Send the actual echo response message
    if (write(resp_fd, echo_response, echo_length) == -1) {
        perror("cannot write the echo response to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Send the length of the variant response message as a single byte
    length_byte = (unsigned char)variant_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the variant message length to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Send the actual variant response message
    if (write(resp_fd, variant_response, variant_length) == -1) {
        perror("cannot write the variant response to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }

    // Send the variant number
    unsigned int variant = VARIANT_NUMBER;
    if (write(resp_fd, &variant, sizeof(variant)) == -1) {
        perror("cannot write the variant number to the response pipe");
        printf("ERROR\n");
        exit(EXIT_FAILURE);
    }
}

void handle_create_shm_request(int resp_fd, int req_fd) {
    unsigned int size;
    if (read(req_fd, &size, sizeof(size)) != sizeof(size)) {
        perror("cannot read the size from the request pipe");
        printf("ERROR\n");
        return;
    }

    // Create shared memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0664);
    if (shm_fd == -1) {
        perror("cannot create the shared memory");
        printf("ERROR\n");
        return;
    }

    // Set the size of the shared memory
    if (ftruncate(shm_fd, size) == -1) {
        perror("cannot set the size of the shared memory");
        shm_unlink(SHM_NAME);
        printf("ERROR\n");
        return;
    }

    // Map the shared memory
    shm_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("cannot map the shared memory");
        shm_unlink(SHM_NAME);
        printf("ERROR\n");
        return;
    }

    shm_size = size;

    // Send success response
    const char *response = "CREATE_SHM";
    size_t response_length = strlen(response);

    unsigned char length_byte = (unsigned char)response_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the response length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, response, response_length) == -1) {
        perror("cannot write the response to the response pipe");
        printf("ERROR\n");
        return;
    }

    const char *status = "SUCCESS";
    size_t status_length = strlen(status);
    length_byte = (unsigned char)status_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the status length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, status, status_length) == -1) {
        perror("cannot write the status to the response pipe");
        printf("ERROR\n");
        return;
    }
}

void handle_write_to_shm_request(int resp_fd, int req_fd) {
    unsigned int offset, value;
    if (read(req_fd, &offset, sizeof(offset)) != sizeof(offset)) {
        perror("cannot read the offset from the request pipe");
        printf("ERROR\n");
        return;
    }
    if (read(req_fd, &value, sizeof(value)) != sizeof(value)) {
        perror("cannot read the value from the request pipe");
        printf("ERROR\n");
        return;
    }

    const char *response = "WRITE_TO_SHM";
    size_t response_length = strlen(response);

    unsigned char length_byte = (unsigned char)response_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the response length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, response, response_length) == -1) {
        perror("cannot write the response to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (offset + sizeof(value) > shm_size) {
        const char *status = "ERROR";
        size_t status_length = strlen(status);
        length_byte = (unsigned char)status_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the status length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, status, status_length) == -1) {
            perror("cannot write the status to the response pipe");
            printf("ERROR\n");
            return;
        }
        return;
    }

    memcpy((char *)shm_addr + offset, &value, sizeof(value));

    const char *status = "SUCCESS";
    size_t status_length = strlen(status);
    length_byte = (unsigned char)status_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the status length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, status, status_length) == -1) {
        perror("cannot write the status to the response pipe");
        printf("ERROR\n");
        return;
    }
}

void handle_map_file_request(int resp_fd, int req_fd) {
    // Read the length of the file name
    unsigned char size_byte;
    if (read(req_fd, &size_byte, sizeof(size_byte)) != sizeof(size_byte)) {
        perror("cannot read the size byte from the request pipe");
        printf("ERROR\n");
        return;
    }

    // Read the file name
    char file_name[size_byte + 1];
    if (read(req_fd, file_name, size_byte) != size_byte) {
        perror("cannot read the file name from the request pipe");
        printf("ERROR\n");
        return;
    }
    file_name[size_byte] = '\0';

    // Open the file
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("cannot open the file");
        const char *response = "MAP_FILE";
        size_t response_length = strlen(response);

        unsigned char length_byte = (unsigned char)response_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the response length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, response, response_length) == -1) {
            perror("cannot write the response to the response pipe");
            printf("ERROR\n");
            return;
        }

        const char *status = "ERROR";
        size_t status_length = strlen(status);
        length_byte = (unsigned char)status_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the status length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, status, status_length) == -1) {
            perror("cannot write the status to the response pipe");
            printf("ERROR\n");
            return;
        }
        return;
    }

    // Get the file size
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1) {
        perror("cannot get the file size");
        close(file_fd);
        const char *response = "MAP_FILE";
        size_t response_length = strlen(response);

        unsigned char length_byte = (unsigned char)response_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the response length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, response, response_length) == -1) {
            perror("cannot write the response to the response pipe");
            printf("ERROR\n");
            return;
        }

        const char *status = "ERROR";
        size_t status_length = strlen(status);
        length_byte = (unsigned char)status_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the status length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, status, status_length) == -1) {
            perror("cannot write the status to the response pipe");
            printf("ERROR\n");
            return;
        }
        return;
    }
    file_size = file_stat.st_size;

    // Map the file into memory
    file_addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    if (file_addr == MAP_FAILED) {
        perror("cannot map the file");
        close(file_fd);
        const char *response = "MAP_FILE";
        size_t response_length = strlen(response);

        unsigned char length_byte = (unsigned char)response_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the response length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, response, response_length) == -1) {
            perror("cannot write the response to the response pipe");
            printf("ERROR\n");
            return;
        }

        const char *status = "ERROR";
        size_t status_length = strlen(status);
        length_byte = (unsigned char)status_length;
        if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
            perror("cannot write the status length to the response pipe");
            printf("ERROR\n");
            return;
        }

        if (write(resp_fd, status, status_length) == -1) {
            perror("cannot write the status to the response pipe");
            printf("ERROR\n");
            return;
        }
        return;
    }

    // Close the file descriptor
    close(file_fd);

    // Send success response
    const char *response = "MAP_FILE";
    size_t response_length = strlen(response);

    unsigned char length_byte = (unsigned char)response_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the response length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, response, response_length) == -1) {
        perror("cannot write the response to the response pipe");
        printf("ERROR\n");
        return;
    }

    const char *status = "SUCCESS";
    size_t status_length = strlen(status);
    length_byte = (unsigned char)status_length;
    if (write(resp_fd, &length_byte, sizeof(length_byte)) == -1) {
        perror("cannot write the status length to the response pipe");
        printf("ERROR\n");
        return;
    }

    if (write(resp_fd, status, status_length) == -1) {
        perror("cannot write the status to the response pipe");
        printf("ERROR\n");
        return;
    }
}

void handle_requests(int resp_fd, int req_fd) {
    while (1) {
        unsigned char size_byte;

        // Read the length of the request message
        if (read(req_fd, &size_byte, sizeof(size_byte)) != sizeof(size_byte)) {
            perror("cannot read the size byte from the request pipe");
            printf("ERROR\n");
            exit(EXIT_FAILURE);
        }

        // Read the actual request message
        char request[size_byte + 1];
        if (read(req_fd, request, size_byte) != size_byte) {
            perror("cannot read the request from the request pipe");
            printf("ERROR\n");
            exit(EXIT_FAILURE);
        }
        request[size_byte] = '\0';

        // Handle the request
        if (strcmp(request, "ECHO") == 0) {
            handle_echo_request(resp_fd);
        } else if (strcmp(request, "EXIT") == 0) {
            break;
        } else if (strcmp(request, "CREATE_SHM") == 0) {
            handle_create_shm_request(resp_fd, req_fd);
        } else if (strcmp(request, "WRITE_TO_SHM") == 0) {
            handle_write_to_shm_request(resp_fd, req_fd);
        } else if (strcmp(request, "MAP_FILE") == 0) {
            handle_map_file_request(resp_fd, req_fd);
        }
    }
}

int main() {
    int req_fd, resp_fd;

    // Create and open pipes
    create_and_open_pipes(&resp_fd, &req_fd);

    // Send connect message
    send_connect_message(resp_fd);

    printf("SUCCESS\n");

    // Handle incoming requests
    handle_requests(resp_fd, req_fd);

    // Clean up
    if (shm_addr != NULL && shm_addr != MAP_FAILED) {
        munmap(shm_addr, shm_size);
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
    if (file_addr != NULL && file_addr != MAP_FAILED) {
        munmap(file_addr, file_size);
    }
    close(req_fd);
    close(resp_fd);
    unlink(RESP_PIPE);

    return 0;
}
