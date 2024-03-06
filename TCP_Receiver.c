#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define FILE_SIZE (3 * 1024 * 1024) // 3MB file
#define INITIAL_CAPACITY 5 // Initial capacity of the times array

long unsigned int counter = 0;

double current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // Get current time
    double seconds = (double)te.tv_sec + (double)te.tv_usec / 1000000.0; // Calculate seconds
    return seconds;     //return milliseconds;

}

double* receive_file(int client_sock)
{
    // Allocate memory for the initial capacity of the times array
    double* list = malloc(INITIAL_CAPACITY * sizeof(double));
    if (list == NULL) {
        // if memory allocation failed
        return NULL;
    }


    size_t capacity = INITIAL_CAPACITY; // Capacity of the list
    char response_buffer[BUFFER_SIZE];   // Stores data here
    int bytes_received = 0;
    double total_bytes_received = 0;


    while(1){
        // This if statment is for excluding the waiting time which the user is choosing to resend a file or not
        if (bytes_received == 0){
            bytes_received = recv(client_sock, response_buffer, BUFFER_SIZE, 0);
            total_bytes_received += bytes_received;
        }
        else{

            double start_time = current_timestamp();    // Calculate time for start

            // This while loop loops throgh all the data of a file sent, 
            // this is to insure that data from the next file will be sent seperatlly(if sent extra file)
            while (total_bytes_received < FILE_SIZE)
            {
                bytes_received = recv(client_sock, response_buffer, BUFFER_SIZE, 0);    // Receive data
                total_bytes_received += bytes_received; // Add up the bytes
            }
            counter++;
            printf("File transfer completed.\n");
            double end_time = current_timestamp();  // Calculate time for end
            double time = end_time - start_time;    // Calculate the time segment

            // Check if the list needs to be reallocated
            if (counter > capacity)
            {
                capacity += INITIAL_CAPACITY;
                double *temp = realloc(list, capacity * sizeof(double));
                if (temp == NULL) {
                    // Reallocation failed
                    free(list);
                    return NULL;
                }
                list = temp;
            }

            list[counter-1] = time;   // list of all the file's times
            bytes_received = 0;
            total_bytes_received = 0;   // Reset the bytes for next loop
            printf("Waiting for Sender response...\n");
        }
        char first_four[5];
        strncpy(first_four, response_buffer, 4);    // Copy only the "exit" (if exists)
        first_four[4] = '\0'; // Null terminate the string
        if (strcmp(first_four, "exit") == 0) {
            printf("Sender sent exit message.\n");
            return list;     // Exit loop when sender sends exit message
        }

    }

    free(list);
}

int main(int argc, char *argv[]) {
    printf("Starting Receiver...\n");
    // Make sure we recieved all the nesasery arguments for the program
    if (argc != 5) {
        printf("Not entered enogh arguments, needed :./receiver -p <Port> -algo <ALGO>\n");
        return -1;
    }
    int port = -1;

    // Pulling the port
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[++i]);
        }
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);   // Create TCP socket
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Set Addresses
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Listen for incoming connections
    int backlog = 5;
    if (listen(sockfd, backlog) < 0) {
        perror("Listen failed");
        return -1;
    }

    // While loop which waits for incoming connections
    while (1) {
        // Accept incoming connection
        socklen_t client_len = sizeof(client_addr);
        printf("Waiting for TCP connection...\n");
        int client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);     // Wait for connections
        if (client_sock < 0) {
            perror("Accept failed");
            return -1;
        }
        printf("Sender connected, beginning to receive file...\n");

        // Call function which will receive the files
        // The function returns a list with the time segments for each file
        double* times = receive_file(client_sock);

        if (times == NULL) {
            // Handle error
            printf("Error: Unable to allocate memory.\n");
            return 1;
        }

        // Wait for Sender response
        char response_buffer[10];
        if (recv(client_sock, response_buffer, sizeof(response_buffer), 0) < 0) {
            perror("Receive failed\n");
            return -1;
        }
        double totaltime;
        double totalbandwidth;
        int counter_times = 0;
        long unsigned int counter_temp = counter;
        counter = 0;
        printf("------------------------------------------------------------------------\n");
        printf("-         * statistics *        -\n");
        for (long unsigned int i = 0; i < counter_temp; i++) {      // Print for each file it's stats after calculation
            if (times[i] != 0)
            {
                counter_times++;
                double speed = 3/times[i];
                printf("-Run #%d data: Time=%fms; Speed=%fMB/s\n",counter_times, times[i], speed);
                totaltime += times[i];
                totalbandwidth += speed;
            }
        }
        // Print all of the avrage stats
        printf("-\n");
        printf("-Average time: %fms\n", totaltime/counter_times);
        printf("-Average bandwidth: %fMB/s\n", totalbandwidth/counter_times);
        printf("------------------------------------------------------------------------\n");
        printf("Receiver end.\n");
        free(times);    // Free memory
    }
    // Close socket

    close(sockfd);
    return 0;
}