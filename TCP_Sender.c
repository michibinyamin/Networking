#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>


#define BUFFER_SIZE 2048 // Adjust this as needed
#define FILE_SIZE (3 * 1024 * 1024) // 3MB file

/*
* @brief A random data generator function based on srand() and rand().
* @param size The size of the data to generate (up to 2^32 bytes).
* @return A pointer to the buffer.
*/
char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
    {
        return NULL;
    }

    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
    {
        return NULL;
    }

    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    {
        *(buffer + i) = ((unsigned int)rand() % 256);
    }
    return buffer;
}

void send_file(int sockfd) {
    // Send data
    printf("Genarating random file...\n");
    char *buffer = util_generate_random_data(FILE_SIZE);    // Function which genarates a random file

    int remaining_data = FILE_SIZE;
    int bytes_sent;


    // In case the whole file was not sent
    while (remaining_data > 0) {
        bytes_sent = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Send failed");
            return;
        }
        remaining_data -= bytes_sent;
    }
}

int main(int argc, char *argv[]) {
    printf("Starting Sender...\n");
    // Make sure we recieved all the nesasery arguments for the program
    if (argc != 7) {
        printf("Not entered enogh arguments for %s, needed :./sender -ip <IP_Address> -p <Port> -algo <congestion_algorithm>\n", argv[0]);
        return -1;
    }

    char *ip_addr = NULL;
    int port = -1;
    char *congestion_algo = NULL;

    // Pass the arguments to variables in our program
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0) {
            // For local host ip - 127.0.0.1
            ip_addr = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            congestion_algo = argv[++i];
        }
    }

    // Invalid arguments
    if (ip_addr == NULL || port == -1 || congestion_algo == NULL) {
        printf("Invalid arguments. Please provide IP address, port, and congestion algorithm.\n");
        return -1;
    }

    printf("Creating socket...\n");
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    printf("Setting congestion control algorithm...\n");

    // Set congestion control algorithm
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, congestion_algo, strlen(congestion_algo)) < 0) {
        perror("Failed to set congestion control algorithm");
        return -1;
    }

    // Server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    printf("Connecting to receiver...\n");
    // Connect to the receiver
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }


    char choice;
    do {
        // Send file
        printf("Sending file...\n");
        send_file(sockfd);

        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');   // User choses if to send again

    // Send exit message
    const char *exit_message = "exit";
    if (send(sockfd, exit_message, strlen(exit_message), 0) < 0) {
        perror("Send exit message failed");
        return -1;
    }

    // Close socket
    printf("Socket closing\n");
    close(sockfd);
    return 0;
}