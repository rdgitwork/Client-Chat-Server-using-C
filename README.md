# Chat Client-Server

This project implements a simple chat application where multiple clients can connect to a server and interact with each other in a group chat. Clients can join the chat by entering their name and the server IP address and can send messages to the entire group.

## Features
- **Client-Server Communication**: Utilizes TCP/IP sockets for communication between the client and server.
- **Group Chat**: Multiple clients can join a group chat session and exchange messages.
- **User Identification**: Clients can enter their name to identify themselves in the chat.
- **Graphical User Interface**: Uses the ncurses library for the server interface and the Windows library for the client interface.

## Usage

### Server

1. Clone the repository: `git clone <repository_url>`
2. Navigate to the `server` directory: `cd server`
3. Compile the server: `gcc server.c -o server -lncurses`
4. Run the server: `./server`

### Client

1. Clone the repository: `git clone <repository_url>`
2. Navigate to the `client` directory: `cd client`
3. Compile the client: `gcc client.c -o client -lncurses`
4. Run the client: `./client`

When prompted, enter your desired username and the server IP address to join the chat.

## Screenshots

[Include screenshots of the running application here]

### Server Interface

![image](https://github.com/rdgitwork/Client-Chat-Server-using-C/assets/110233441/73be6846-6a7b-44e8-b3da-ebe92a90ce0e)


### Client Interface

![image](https://github.com/rdgitwork/Client-Chat-Server-using-C/assets/110233441/6693c620-d9b0-4abb-bed1-87c8cd072b7a)


## Contributing

Contributions are welcome! If you'd like to contribute to this project, please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

Feel free to contribute to the project or provide feedback by opening issues or pull requests. Happy chatting!
