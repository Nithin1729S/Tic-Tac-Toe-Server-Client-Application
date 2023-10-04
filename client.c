#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void receiveMessage(int sockfd, char *msg);
int receiveInteger(int sockfd);
void sendServerInteger(int sockfd, int msg);
void sendServerInteger(int sockfd, int msg);
void reportError(const char *msg);
int connectToServer(char *hostname, int portno);
void displayGameBoard(char board[][3]);
void takeTurn(int sockfd);
void getGameUpdate(int sockfd, char board[][3]);

int connectToServer(char *hostname, int portno)
{
    struct sockaddr_in serverAddress;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        perror("Error opening socket for server!!");

    server = gethostbyname(hostname);

    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host!!\n");
        exit(0);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    memmove(server->h_addr, &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        perror("Error Connecting to server!!");

    printf("Connected to server.\n");
    return sockfd;
}

void receiveMessage(int sockfd, char *msg)
{
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);

    if (n < 0 || n != 3)
        perror("Error reading message from server socket.");
}

int receiveInteger(int sockfd)
{
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));

    if (n < 0 || n != sizeof(int))
        perror("Error reading integer from server socket");
    return msg;
}

void sendServerInteger(int sockfd, int msg)
{
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        perror("Error writing integer to server socket");
}

void reportError(const char *msg)
{
    perror(msg);
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    exit(0);
}


void displayGameBoard(char board[][3])
{
    printf("\n");
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            char symbol = board[row][col];
            if (symbol == ' ')
                printf(" %d ", (row * 3) + col); // Display the position number
            else
                printf(" %c ", symbol);

            if (col < 2)
                printf("|");
        }
        printf("\n");
        if (row < 2)
            printf("-----------\n");
    }
}

void takeTurn(int sockfd)
{
    char buffer[10];

    while (1)
    {
        printf("Enter 0-8 to make a move: ");
        fgets(buffer, 10, stdin);
        int move = buffer[0] - '0';
        if (move < 9 && move >= 0)
        {
            printf("\n");
            sendServerInteger(sockfd, move);
            break;
        }
        else
            printf("\nInvalid input. Try again.\n");
    }
}

void getGameUpdate(int sockfd, char board[][3])
{
    int playerId = receiveInteger(sockfd);
    int move = receiveInteger(sockfd);
    board[move / 3][move % 3] = playerId ? 'X' : 'O';
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    int sockfd = connectToServer(argv[1], atoi(argv[2]));

    int id = receiveInteger(sockfd);

    char msg[4];
    char board[3][3] = {{' ', ' ', ' '},
                        {' ', ' ', ' '},
                        {' ', ' ', ' '}};

    printf("\n"
           " _______ _        _______           _______ \n"
           "|__   __(_)      |__   __|         |__   __| \n"
           "   | |   _  ___     | | __ _  ___     | | ___   ___ \n"
           "   | |  | |/ __|    | |/ _` |/ __|    | |/ _ \\ / _ \\ \n"
           "   | |  | | (__     | | (_| | (__     | | (_) |  __/ \n"
           "   |_|  |_|\\___|    |_|\\__,_|\\___|    |_|\\___/ \\___| \n\n\n");

    do
    {
        receiveMessage(sockfd, msg);
        if (!strcmp(msg, "HLD"))
            printf("Waiting for a second player...\n");
    } while (strcmp(msg, "SRT"));

    /* The game has begun. */
    printf("Game on!\n");
    printf("You are %c's\n", id ? 'X' : 'O');

    displayGameBoard(board);

    while (1)
    {
        receiveMessage(sockfd, msg);

        if (!strcmp(msg, "TRN"))
        {
            printf("Your move...\n");
            takeTurn(sockfd);
        }
        else if (!strcmp(msg, "INV"))
        {
            printf("That position has already been played. Try again.\n");
        }
        else if (!strcmp(msg, "CNT"))
        {
            int numPlayers = receiveInteger(sockfd);
            printf("There are currently %d active players.\n", numPlayers);
        }
        else if (!strcmp(msg, "UPD"))
        {
            getGameUpdate(sockfd, board);
            displayGameBoard(board);
        }
        else if (!strcmp(msg, "WAT"))
        {
            printf("Waiting for the other player's move...\n");
        }
        else if (!strcmp(msg, "WIN"))
        {
            printf("__   __           __        ___       \n");
            printf("\\ \\ / /__  _   _  \\ \\      / (_)_ __  \n");
            printf(" \\ V / _ \\| | | |  \\ \\ /\\ / /| | '_ \\ \n");
            printf("  | | (_) | |_| |   \\ V  V / | | | | |\n");
            printf("  |_|\\___/ \\__,_|    \\_/\\_/  |_|_| |_|\n");

            break;
        }
        else if (!strcmp(msg, "LSE"))
        {
            printf("__   __            _              _   \n");
            printf("\\ \\ / /__  _   _  | |    ___  ___| |_ \n");
            printf(" \\ V / _ \\| | | | | |   / _ \\/ __| __|\n");
            printf("  | | (_) | |_| | | |__| (_) \\__ \\ |_ \n");
            printf("  |_|\\___/ \\__,_| |_____\\___/|___/\\__|\n");
            break;
        }
        else if (!strcmp(msg, "DRW"))
        {
            printf(" ___ _   _               _____ _      \n");
            printf("|_ _| |_( )___    __ _  |_   _(_) ___ \n");
            printf(" | || __|// __|  / _` |   | | | |/ _ \\\n");
            printf(" | || |_  \\__ \\ | (_| |   | | | |  __/\n");
            printf("|___|\\__| |___/  \\__,_|   |_| |_|\\___|\n");

            break;
        }
        else
        {
            reportError("Unknown message.");
        }
    }

    printf("Game over.\n");
    close(sockfd);
    return 0;
}
