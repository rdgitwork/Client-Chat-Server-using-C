/*
* Programmer NAME - Vansh Prajapati, Rudrakumar Patel, Deep Patel
* Student Number - 8888319 ,  8872703 , 8847589
* Assignment Name - Can We Talk
* File Name - Chat_client.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <time.h>

#define MAX_MESSAGE_LEN 90
#define MAX_USERNAME_LEN 10
#define SERVER_PORT 12345
#define MAX_MESSAGES 10
//#define SERVER_IP "169.254.245.28"

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *win);
void input_win_fun(WINDOW *win, char *word);
void display_win(WINDOW *win, char *word, int whichRow, int shouldBlank);
void blankWin(WINDOW *win);

void *receive_messages(void *arg);
void send_message(int socket_fd, const char *message);
void refresh_display_messages(void);
void getCurrentTime(char* buffer, size_t bufferSize);
void addMessageToBuffer(const char* formattedMessage);
WINDOW *input_win, *output_win; 

typedef struct {
    char text[MAX_MESSAGE_LEN + MAX_USERNAME_LEN + 3]; // Message text
} Message;

typedef struct {
    int socket_fd;
    char clientIP[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN to accommodate the IP string
    char username[MAX_USERNAME_LEN + 1];
} ThreadArgs;

Message messages[MAX_MESSAGES];
int message_count = 0; // Total number of messages received


int main(int argc, char *argv[]) {
    int socket_fd;
    struct sockaddr_in server_address;
    pthread_t tid;
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
    // Assuming socket_fd is the socket descriptor for the connected client

   char* server_ip = NULL;
    
   for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "-user", 5) == 0) {
            strncpy(username, argv[i] + 5, MAX_USERNAME_LEN);
        } else if (strncmp(argv[i], "-server", 7) == 0) {
            server_ip = argv[i] + 7;
        }
    }

    if (strlen(username) == 0 || server_ip == NULL) {
        fprintf(stderr, "Usage: %s -user<Username> -server<ServerIP>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialize ncurses
    initscr();
    refresh();
    
       // Create windows for input and output
    //WINDOW *input_win, *output_win;
    int input_height = 3;
    int output_height = LINES - input_height - 1;
    int width = COLS;

    input_win = create_newwin(input_height, width, LINES - input_height, 0);
    output_win = create_newwin(output_height, width, 0, 0);

    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

 ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (args == NULL) {
        perror("Failed to allocate memory for thread arguments");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in localAddress;
    socklen_t addressLength = sizeof(localAddress);
    getsockname(socket_fd, (struct sockaddr*)&localAddress, &addressLength);
    strncpy(args->clientIP, inet_ntoa(localAddress.sin_addr), INET_ADDRSTRLEN);
     
    args->socket_fd = socket_fd;

    // Create thread to receive messages
    pthread_create(&tid, NULL, receive_messages, (void *)args);


    // Send username to server
    send_message(socket_fd, username);
    strncpy(args->clientIP, inet_ntoa(localAddress.sin_addr), INET_ADDRSTRLEN);

 

args->socket_fd = socket_fd;

strncpy(args->username, username, MAX_USERNAME_LEN + 1);

pthread_create(&tid, NULL, receive_messages, (void *)args);
 
    // Main loop for sending messages
while (1) {
 //char formattedMessage[200]; // Adjust size as per your specification
    //char timestamp[20];
    
    mvwprintw(input_win, 1, 1, "Enter message: ");
    wclrtoeol(input_win); // Clear the line
    wrefresh(input_win);
    wgetstr(input_win, message); // Use wgetstr instead of fgets for ncurses
    
    if (strcmp(message, ">>bye<<") == 0) {
        
        break; // Exit the loop to clean up and close the program
    }
    
 
    //addMessageToBuffer(formattedMessage); // Add formatted message directly
    send_message(socket_fd, message); // Send formatted message
}

    // Close socket
    close(socket_fd);

    // End ncurses
    endwin();

    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);
    return local_win;
}

void destroy_win(WINDOW *win) {
    delwin(win);
}

void input_win_fun(WINDOW *win, char *word) {
    int ch;
    int maxrow, maxcol, row = 1, col = 0;

    blankWin(win); // Clear window
    getmaxyx(win, maxrow, maxcol);
    bzero(word, MAX_MESSAGE_LEN);
    mvwprintw(win, 1, 1, "> ");
    wrefresh(win);
    for (int i = 0; (ch = wgetch(win)) != '\n'; i++) {
        word[i] = ch;
        if (col++ < maxcol - 4) {
            wprintw(win, "%c", word[i]);
        } else {
            col = 1;
            if (row == maxrow - 2) {
                scroll(win);
                row = maxrow - 2;
                wmove(win, row, col);
                wclrtoeol(win);
                box(win, 0, 0);
            } else {
                row++;
                wmove(win, row, col);
                wrefresh(win);
                wprintw(win, "%c", word[i]);
            }
        }
    }
}

void display_win(WINDOW *win, char *word, int whichRow, int shouldBlank) {
    if (shouldBlank == 1) blankWin(win);
    mvwprintw(win, whichRow + 1, 1, word);
    wrefresh(win);
}

void blankWin(WINDOW *win) {
    int maxrow, maxcol;

    getmaxyx(win, maxrow, maxcol);
    for (int i = 1; i < maxcol - 2; i++) {
        mvwprintw(win, i, 1, " ");
        wrefresh(win);
    }
    box(win, 0, 0);
    wrefresh(win);
}

void *receive_messages(void *arg) {
    // ThreadArgs *args = (ThreadArgs *)arg; // Not used here, since formatting is handled by the server.
    int socket_fd = *((int *)arg); // Dereference to get the socket descriptor.
    char receivedMessage[MAX_MESSAGE_LEN + 100]; // Adjust size to handle maximum formatted message length.

    ssize_t bytes_received;
    while ((bytes_received = recv(socket_fd, receivedMessage, sizeof(receivedMessage), 0)) > 0) {
        receivedMessage[bytes_received] = '\0'; // Ensure null termination.

        // Directly add the received, pre-formatted message to the message buffer.
        addMessageToBuffer(receivedMessage);
        refresh_display_messages(); // Refresh the display to show the new message.
    }

    return NULL;
}

void send_message(int socket_fd, const char *message) {
    send(socket_fd, message, strlen(message), 0);
}

void refresh_display_messages() {
    werase(output_win); // Clear the output window first

    int start = message_count > MAX_MESSAGES ? message_count - MAX_MESSAGES : 0;
    for (int i = start; i < message_count; ++i) {
        // Calculate the index in the circular buffer
        int index = i % MAX_MESSAGES;
        
        // Display the message on the window; adjust the 2nd parameter as needed for positioning
        mvwprintw(output_win, (i - start) + 1, 1, "%s", messages[index].text);
    }

    box(output_win, 0, 0); // Optionally redraw box border if used
    wrefresh(output_win); // Refresh the window to show the updated content
}


void getCurrentTime(char* buffer, size_t bufferSize) {
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, bufferSize, "(%H:%M:%S)", timeinfo);
}

void addMessageToBuffer(const char* formattedMessage) {
    // Ensure the index is within bounds of the messages array
    int index = message_count % MAX_MESSAGES;

    // Copy the formatted message into the next slot in the circular buffer
    strncpy(messages[index].text, formattedMessage, sizeof(messages[index].text) - 1);
    messages[index].text[sizeof(messages[index].text) - 1] = '\0'; // Ensure null termination

    // Increment the total message count
    message_count++;

    // Refresh the display to show the new message
    refresh_display_messages();
}



