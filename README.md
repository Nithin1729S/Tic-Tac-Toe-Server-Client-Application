# Tic-Tac-Toe Client-Server Application

This project is a client-server application designed for playing the game of Tic-Tac-Toe between two players. It utilizes C for the implementation and socket programming for communication between the server and clients. The server is designed to handle two clients concurrently using multithreading.

## Getting Started

### Prerequisites

- GCC compiler
- Linux environment (for pthread library)

### Server Setup

1. Compile the server code:

    ```bash
    gcc server.c -lpthread -o server
    ```

2. Run the server:

    ```bash
    ./server 8080
    ```

   Replace `8080` with the desired port number.

### Client Setup

1. Compile the client code:

    ```bash
    gcc client.c -o client
    ```

2. Run the client on each player's machine:

    ```bash
    ./client <server_ip_address> <port_number>
    ```

   Replace `<server_ip_address>` with the server's IP address and `<port_number>` with the chosen port number.

## Gameplay

1. The server waits for two clients to connect.

2. Each client connects to the server using the provided client code.

3. Once both clients are connected, the game begins.

4. Players take turns making moves by entering positions (0-8) on the Tic-Tac-Toe board.

5. The server validates moves, updates the game board, and checks for a win, loss, or draw.

6. The game continues until a win, loss, or draw condition is met.

7. Clients receive updates from the server and display the current state of the game.

8. Players can disconnect, and the server handles the termination of the game.

## Directory Structure

- **server.c**: Contains the server-side code for handling game logic and client connections.
- **client.c**: Implements the client-side code for connecting to the server and interacting with the game.

## Notes

- Ensure that the server is running before clients attempt to connect.
- Players can connect from different machines using the server's IP address.
- Look at the project report for more details regarging the code and execution.
  
Feel free to customize the code or enhance the game features based on your preferences. Enjoy playing Tic-Tac-Toe with your friends!
