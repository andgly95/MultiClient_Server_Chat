//
//  client.cpp
//  
//
//  Created by Andrew on 5/20/17.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "client.hpp"

using namespace std;

int main(){
    
    //Creates New Client
    Client newClient;
    
    //Creates socket configuration for server Socket
    newClient.configure();
    
    //Logs in Client
    int user = newClient.login();
    
    string command;
    int logout = 0;
    
    //Creates New Thread for Client to Listen for Server Messages
    newClient.listen();
    
    //Command loop
    while (!logout){
        
        //cout << "Awaiting Input: ";
        cin >> command;
        
        if (command == "send"){
            string sendTo;
            cin >> sendTo;
            string message;
            getline(cin, message);
            if (sendTo == "all"){
                newClient.sendAll(message);
            }
            else {
                newClient.sendTo(message, sendTo);
            }
            //cout << message << endl;
            
        }
        if (command == "logout"){
            newClient.logout();
            logout = 1;
        }
        if (command == "who"){
            newClient.who();
        }
        //logout = 1;
    }
    return 0;
}


int Client::configure(){
    
    //Configures socket for Server to connect to
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    portNo = PORTNO;
    if (serverSocket < 0) {
        error("ERROR opening socket");
        
    }
    
    //Creates address for Server
    struct sockaddr_in serverAddress;
    struct hostent *server;
    
    //Finds server on localhost, can be changed to host name
    server = gethostbyname("localhost");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    //Sets Server Address properties
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serverAddress.sin_addr.s_addr,
          server->h_length);
    serverAddress.sin_port = htons(portNo);
    
    //Connects client to Server
    if (connect(serverSocket,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0){
        error("ERROR connecting");
    }
    
    cout << "Connected to server: ";
    return 0;
}

int Client::login(){
    int userLogged = 0;
    string username;
    string password;
    string command;
    string entry;
    
    int n;
    //Creates buffer for text transfer
    char buffer[256];
    
    //Loop until user successfully logs in
    while (!userLogged) {
        //Input command, username and password
        cin >> command >> username >> password;
        if (command == "login"){
            //Login Command
            
            // Send Username to Server
            bzero(buffer,256);
            username.copy(buffer, 10);
            n = send(serverSocket,buffer,strlen(buffer), 0);
            
            // Receives Message for Account Found
            n = recv(serverSocket,buffer,255, 0);
            
            cout << buffer << endl;
            
            // Receives status bit for User Found
            bzero(buffer,256);
            n = recv(serverSocket,buffer,1, 0);
            if (n < 0)
                error("ERROR reading from socket");
            
            //If User Found, sends over Password
            if(buffer[0] == '0'){
                bzero(buffer,256);
                password.copy(buffer, 256);
                n = send(serverSocket,buffer,256, 0);
                
                usleep(5);
                bzero(buffer,256);
                
                //Receives Message for Correct password
                n = recv(serverSocket,buffer,256, 0);
                if (n < 0)
                    error("ERROR reading from socket");
                
                
                //Receives Status Bit for successful password
                
                bzero(buffer,256);
                n = recv(serverSocket,buffer,1, 0);
                if (n < 0)
                    error("ERROR reading from socket");
                
                
                //If Password Correct
                if(buffer[0] == '0'){
                    
                    userLogged = 1;
                    bzero(buffer,256);
                    cout << "Login Succeeded\n";

                }
                
                //If Incorrect
                else {
                    bzero(buffer,256);
                    cout << "Please Try Again: ";
                }
                
            }
            //If User Not Found, Try Again
            else {
                bzero(buffer,256);
                cout << "Please Try Again: ";
            }
        }
        //Any command other than Login fails
        else {
            bzero(buffer,256);
            cout << "You must log in first\n";
        }
    }
    //Sends UserID to Client
    bzero(buffer,256);
    n = recv(serverSocket,buffer,1, 0);
    if (n < 0)
        error("ERROR reading from socket");
    userNo = buffer[0];
    
    //Send as first digit SendTo Address
    userNo = buffer[0];
    
    //cout << "USER ID IS " << userNo << endl;

    return userNo;

}

//Multicasts message to all clients
int Client::sendAll(string message){
    char buffer[256];
    
    //Send bit for multicast
    int n = send(serverSocket,"a",1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    
    //Send Sender ID
    char userID = userNo;
    buffer[0] = userID;
    n = send(serverSocket,buffer,1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    
    //Send Message
    bzero(buffer,256);
    message.copy(buffer, 256);
    n = send(serverSocket,buffer,256, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    return 0;
    
}

//Unicasts message to user specified
int Client::sendTo(string message, string user){
    char buffer[256];
    
    //Send bit for unicast
    int n = send(serverSocket,"u",1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    
    //Send Sender ID
    char userID = userNo;
    cout << userID << endl;
    buffer[0] = userID;
    n = send(serverSocket,buffer,1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    
    //Send Message
    bzero(buffer,256);
    message.copy(buffer, 256);
    cout << buffer << endl;
    n = send(serverSocket,buffer,256, 0);
    if (n < 0)
        error("ERROR writing to socket");
    
    //Sends UserName to Client
    bzero(buffer,256);
    user.copy(buffer, 256);
    cout << buffer << endl;
    n = send(serverSocket,buffer,256, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    return 0;
}

void Client::error(const char *msg){
    perror(msg);
    exit(1);
}


int Client::listen(){
    
    //Creates seperate thread to listen to Server commands
    thread listen(mem_fun(&Client::receive), this);
    listen.detach();
    return 0;
}

int Client::receive(){
    int logout = 0;
    char buffer[256];
    
    //Receives messages from either client 'b' or server 'l'
    while(!logout){
        recv(serverSocket, buffer, 1, 0);
        if(buffer[0] == 'b'){
            bzero(buffer,256);
            recv(serverSocket, buffer, 256, 0);
            cout << buffer << ":";
            bzero(buffer,256);
            recv(serverSocket, buffer, 256, 0);
            cout << buffer << endl;
            bzero(buffer,256);

        }
        if(buffer[0] == 'l'){
            bzero(buffer,256);
            recv(serverSocket, buffer, 256, 0);
            cout << buffer << endl;
            bzero(buffer,256);
            
        }
    }
    return 0;
}

//Request server for list of Logged In Users
int Client::who(){
    char buffer[256];
    
    //Send bit for who command
    int n = send(serverSocket,"w",1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    
    //Send Sender ID
    bzero(buffer,256);
    char userID = userNo;
    cout << userID << endl;
    buffer[0] = userID;
    n = send(serverSocket,buffer,1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    
    return 0;
}

int Client::logout(){
    char buffer[256];
    
    //Send bit for logout
    int n = send(serverSocket,"l",1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    
    //Send Sender ID
    bzero(buffer,256);
    char userID = userNo;
    cout << userID << endl;
    buffer[0] = userID;
    n = send(serverSocket,buffer,1, 0);
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    
    //Close Server Socket on Client Side
    close(serverSocket);
    return 0;
    

}



