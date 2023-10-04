#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int playerCount = 0;
pthread_mutex_t playerMutex;

void reportError(const char *message);
void sendClientMessage(int clientSock, char *message);
void sendClientInteger(int clientSock, int value);
void sendClientsMessage(int *clientSocks, char *message);
void sendClientsInteger(int *clientSocks, int value);
int setupListener(int portNumber);
int receiveInteger(int clientSock);
void getClientSockets(int listenerSock, int *clientSocks);
int getPlayerMove(int clientSock);
int isValidMove(char board[][3], int move, int playerId);
void updateGameBoard(char board[][3], int move, int playerId);
void drawGameBoard(char board[][3]);
void sendGameUpdate(int *clientSocks, int move, int playerId);
void sendPlayerCount(int clientSock);
int checkGameBoard(char board[][3], int lastMove);

int setupListener(int portNumber)
{
    int listenerSock;
    struct sockaddr_in serverAddress;

    listenerSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenerSock < 0)
        reportError("Error opening listener socket.");

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    if (bind(listenerSock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        reportError("Error binding listener socket.");

    return listenerSock;
}

void sendClientInteger(int clientSock, int value)
{
    int n = write(clientSock, &value, sizeof(int));
    if (n < 0)
        reportError("Error writing integer to client socket");
}

void sendClientsMessage(int *clientSocks, char *message)
{
    sendClientMessage(clientSocks[0], message);
    sendClientMessage(clientSocks[1], message);
}

void sendClientsInteger(int *clientSocks, int value)
{
    sendClientInteger(clientSocks[0], value);
    sendClientInteger(clientSocks[1], value);
}

int receiveInteger(int clientSock)
{
    int value = 0;
    int n = read(clientSock, &value, sizeof(int));

    if (n < 0 || n != sizeof(int))
        return -1;

    return value;
}

void sendClientMessage(int clientSock, char *message)
{
    int n = write(clientSock, message, strlen(message));
    if (n < 0)
        reportError("Error writing message to client socket");
}

void reportError(const char *message)
{
    perror(message);
    pthread_exit(NULL);
}

void getClientSockets(int listenerSock, int *clientSocks)
{
    socklen_t clientAddressLength;
    struct sockaddr_in serverAddress, clientAddress;
    int numConnections = 0;

    while (numConnections < 2)
    {
        listen(listenerSock, 253 - playerCount);

        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLength = sizeof(clientAddress);

        clientSocks[numConnections] = accept(listenerSock, (struct sockaddr *)&clientAddress, &clientAddressLength);

        if (clientSocks[numConnections] < 0)
            reportError("Error accepting a connection from a client.");

        write(clientSocks[numConnections], &numConnections, sizeof(int));

        pthread_mutex_lock(&playerMutex);
        playerCount++;
        printf("Number of players is now %d.\n", playerCount);
        pthread_mutex_unlock(&playerMutex);

        if (numConnections == 0)
        {
            sendClientMessage(clientSocks[0], "HLD");
        }

        numConnections++;
    }
}

int getPlayerMove(int clientSock)
{
    sendClientMessage(clientSock, "TRN");
    return receiveInteger(clientSock);
}

int isValidMove(char board[][3], int move, int playerId)
{
    if ((move == 9) || (board[move / 3][move % 3] == ' '))
        return 1;
    else
        return 0;
}

void updateGameBoard(char board[][3], int move, int playerId)
{
    board[move / 3][move % 3] = playerId ? 'X' : 'O';
}

void drawGameBoard(char board[][3])
{
    printf("\n");
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

void sendGameUpdate(int *clientSocks, int move, int playerId)
{
    sendClientsMessage(clientSocks, "UPD");
    sendClientsInteger(clientSocks, playerId);
    sendClientsInteger(clientSocks, move);
}

void sendPlayerCount(int clientSock)
{
    sendClientMessage(clientSock, "CNT");
    sendClientInteger(clientSock, playerCount);
}

int checkGameBoard(char board[][3], int lastMove)
{
    int row = lastMove / 3;
    int col = lastMove % 3;

    if (board[row][0] == board[row][1] && board[row][1] == board[row][2])
    {
        return 1;
    }
    else if (board[0][col] == board[1][col] && board[1][col] == board[2][col])
    {
        return 1;
    }
    else if (!(lastMove % 2))
    {
        if ((lastMove == 0 || lastMove == 4 || lastMove == 8) && (board[1][1] == board[0][0] && board[1][1] == board[2][2]))
        {
            return 1;
        }
        if ((lastMove == 2 || lastMove == 4 || lastMove == 6) && (board[1][1] == board[0][2] && board[1][1] == board[2][0]))
        {
            return 1;
        }
    }

    return 0;
}

void *runGame(void *threadData)
{
    int *clientSocks = (int *)threadData;
    char board[3][3] = {{' ', ' ', ' '},
                        {' ', ' ', ' '},
                        {' ', ' ', ' '}};

    printf("Game on!\n");

    sendClientsMessage(clientSocks, "SRT");

    drawGameBoard(board);

    int prevPlayerTurn = 1;
    int playerTurn = 0;
    int gameOver = 0;
    int turnCount = 0;

    while (!gameOver)
    {
        if (prevPlayerTurn != playerTurn)
            sendClientMessage(clientSocks[(playerTurn + 1) % 2], "WAT");

        int valid = 0;
        int move = 0;

        while (!valid)
        {
            move = getPlayerMove(clientSocks[playerTurn]);
            if (move == -1)
                break;

            printf("Player %d played position %d\n", playerTurn, move);

            valid = isValidMove(board, move, playerTurn);
            if (!valid)
            {
                printf("Move was invalid. Let's try this again...\n");
                sendClientMessage(clientSocks[playerTurn], "INV");
            }
        }

        if (move == -1)
        {
            printf("Player disconnected.\n");
            break;
        }
        else if (move == 9)
        {
            prevPlayerTurn = playerTurn;
            sendPlayerCount(clientSocks[playerTurn]);
        }
        else
        {
            updateGameBoard(board, move, playerTurn);
            sendGameUpdate(clientSocks, move, playerTurn);

            drawGameBoard(board);

            gameOver = checkGameBoard(board, move);

            if (gameOver == 1)
            {
                sendClientMessage(clientSocks[playerTurn], "WIN");
                sendClientMessage(clientSocks[(playerTurn + 1) % 2], "LSE");
                printf("Player %d won.\n", playerTurn);
            }
            else if (turnCount == 8)
            {
                printf("Draw.\n");
                sendClientsMessage(clientSocks, "DRW");
                gameOver = 1;
            }

            prevPlayerTurn = playerTurn;
            playerTurn = (playerTurn + 1) % 2;
            turnCount++;
        }
    }

    printf("Game over.\n");

    close(clientSocks[0]);
    close(clientSocks[1]);

    pthread_mutex_lock(&playerMutex);
    playerCount--;
    printf("Number of players is now %d.\n", playerCount);
    pthread_mutex_unlock(&playerMutex);

    free(clientSocks);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Error, no port provided\n");
        exit(1);
    }
    printf("Server is up and running, waiting for clients to connect...\n");
    fflush(stdout); // Flush the output buffer

    int listenerSock = setupListener(atoi(argv[1]));
    pthread_mutex_init(&playerMutex, NULL);

    while (1)
    {
        if (playerCount <= 252)
        {
            int *clientSocks = (int *)malloc(2 * sizeof(int));
            memset(clientSocks, 0, 2 * sizeof(int));

            getClientSockets(listenerSock, clientSocks);

            pthread_t thread;
            int result = pthread_create(&thread, NULL, runGame, (void *)clientSocks);
            if (result)
            {
                printf("Thread creation failed with return code %d\n", result);
                exit(-1);
            }

            printf("New game thread started.\n");
        }
    }

    close(listenerSock);
    pthread_mutex_destroy(&playerMutex);
    pthread_exit(NULL);
}
