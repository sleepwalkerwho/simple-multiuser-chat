# simple-multiuser-chat  
Make is used as an build system.  
## Server  
Server receive messages from clients and send messages to all connected clients.
Threads are used to work with multiple clients. 
The server have a console interface in the parameters to which is transferred: port

```./server <port>```  

## Client  
Each client must have own nickname, set by the user. When you receive a message from another client, the screen displays the time of receiving the
message, the user-sender’s nickname, and the text of the message.   
An example:  

```{05:20} [John] Hi!```

The client have a console interface. The client accepts the work options through the command line arguments in the following order: server address, port, nickname.  
```./client localhost <port> <nick>```  
To prevent the received (incoming) messages from interfering with the user’s typing, it is suggested that a separate mode of sending a message, for example, when the m key is pressed, the user enters his message, new messages from other users are temporarily not displayed, after sending the message (by Enter) the mode is automatically turned off.  
