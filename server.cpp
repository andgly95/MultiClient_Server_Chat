//
//  server.cpp
//  
//
//  Created by Andrew on 5/20/17.
//
//

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <thread>
#include <chrono>
#include <fstream>
#include "server.hpp"

#define MAXCLIENTS 3;
#define PORTNO 12004;

using namespace std;


int main(){
    
    Server* newServer = new Server;
    
    newServer->configure();
    
}


//Configures server socket
int Server::configure(){
    
    //Creates user accounts from text file
    validUsers = new UserAccounts[4];
    ifstream userfile;
    userfile.open("users.txt");
    for (int i=0;i<4;i++){
        string uname, pword;
        userfile >> uname >> pword;
        validUsers[i].setAccount(uname, pword);
    }
    
    //Creates socket for Server
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int portNo = PORTNO;
    if (serverSocket < 0) {
        error("ERROR opening socket");
    }
    
    //Creates socked address for server
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    
    //Configure server socket address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNo);
    
    //Bind socket to address
    if (::bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        error("ERROR on binding");
    
    //Create array of sockets for clients
    clientSockets = new int[4];
    
    //Opens up Server socket to connections
    listen(serverSocket,5);
    
    int newSocket;
    
    //Runs continuously to allow for connections
    while (activeClients < 4)
    {
        //Creates new socket for new connection
        newSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientLength);
        if (newSocket < 0)
            error("ERROR on accept");
        activeClients++;
        
        //Creates new thread to log in client and then returns
        NoOfThreads++;
        thread loginThread(mem_fun(&Server::newConnection),this, newSocket);
        loginThread.join();
        NoOfThreads--;
        
        //Creates new thread for Client to run on Server
        thread runningClient(mem_fun(&Server::receive),this, newSocket);
        NoOfThreads++;
        runningClient.detach();
    } /* end of while */
    return 0;
}


//Runs as seperate thread for client login
int Server::newConnection(int socket){
    
    
    cout << "Client has connected\n";
    bool validLogin = 0;
    //Create Buffer for data transfer
    char buffer[256];
    int n;
    int validUser = -1;
    
    // LOG IN LOOP
    
    while (!validLogin){
        validUser = -1;
        cout << "Client login\n";
        bzero(buffer,256);
        n =recv(socket,buffer,255, 0);
        if (n < 0) error("ERRORrecving from socket");
        
        string login = buffer;
        //Checks user login against names of users, returns user index if found and -1 if not
        for(int i=0;i<4;i++){
            if (validUsers[i].verifyUser(login)){
                validUser = i;
            }
        }
        
        if (validUser == -1){
            //Send 1 if user not found
            bzero(buffer,256);
            n = send(socket,"Account Not Found\n",18, 0);
            cout << "Acount " << login << " Not Found\n";
            bzero(buffer,256);
            n = send(socket,"1",1, 0);
        }
        
        else {
            bzero(buffer,256);
            n = send(socket,"Account Found\n",14, 0);
            bzero(buffer,256);
            //Send 0 if user found
            bzero(buffer,256);
            usleep(5);
            n = send(socket,"0",1, 0);
            bzero(buffer,256);
            n =recv(socket,buffer,255, 0);
            //Checks password against user password;
            string password = buffer;
            if (validUsers[validUser].verifyPassword(password)){
                cout << "User " << login << " has connected\n";
                n = send(socket,"User logged in\n",256, 0);
                validUsers[validUser].logIn(login, password);
                cout << "Account " << login << " Logged In\n";
                bzero(buffer,256);
                n = send(socket,"0",1, 0);
                validLogin = 1;
            }
            else {
                cout << "Incorrect Password\n";
                n = send(socket,"Incorrect Password\n",19, 0);
                bzero(buffer,256);
                n = send(socket,"1",1, 0);
            }
        }
        
    }
    
    //Adds user's socket to userAccount
    clientSockets[validUser] = socket;
    validUsers[validUser].setSocket(socket);
    bzero(buffer,256);
    
    //Sents User's ID number back to client;
    string userBit = to_string(validUser);
    userBit.copy(buffer, 1);
    n = send(socket,buffer,1, 0);
    n = recv(socket,buffer,1, 0);
    
    return 0;
}

//Listens for commands from Client and runs them, runs as seperate thread
int Server::receive(int socket){
    //Create buffer for client communication
    char buffer[256];
    int n;
    char commandCode;
    bool logout = 0;
    //Run as loop
    while (!logout){
        cout << "Taking commands\n";
        bzero(buffer,256);
        n =recv(socket,buffer,1, 0);
        //Client sends character to Server to signal which command to run
        //If value is invalid, server continues listening
        while (buffer[0] != 'a' && buffer[0] != 'u' && buffer[0] != 'n' && buffer[0] != 'l' && buffer[0] != 'w' && n==-1){
            usleep(5);
            n =recv(socket,buffer,1, 0);
            cout << "Receive command\n";
        }
        commandCode = buffer[0];
        //cout << commandCode << endl;
        
        //Server receives User ID of Client to know where request is from
        bzero(buffer,256);
        n =recv(socket,buffer,1, 0);
        if (n < 0) error("ERRORrecving from socket");
        char sourceID = buffer[0];
        bzero(buffer,256);
        int userSourceNo = sourceID - 48;
        
        //Runs Command
        if (commandCode == 'a'){
            //SEND ALL Command
            cout << "SEND ALL" << endl;
            bzero(buffer,256);
            usleep(5);
            //Receives Message string from client
            n = recv(socket,buffer,256, 0);
            if (n < 0) error("ERRORrecving from socket");
            string multiMessage = buffer;
            cout << validUsers[userSourceNo].getName() << ":";
            cout << multiMessage << endl;
            //Searches users and sends message to all logged in users beside sender
            for (int i=0;i<4;i++){
                if (validUsers[i].is_Logged_In() && userSourceNo != i){
                    clientToSocket(multiMessage, validUsers[i], validUsers[userSourceNo]);
                    cout << "Send to user " << i << endl;
                }
            }
        }
        else if (commandCode == 'u'){
            //SEND TO Command
            cout << "SEND TO" << endl;
            
            bzero(buffer,256);
            usleep(5);
            //Receives Message string from client
            n = recv(socket,buffer,256, 0);
            cout << "RECEIVED Message()\n";
            if (n < 0) error("ERRORrecving from socket");
            string multiMessage = buffer;
            
            bzero(buffer,256);
            usleep(5);
            
            //Receives Sendee name from Client
            n = recv(socket,buffer,256, 0);
            cout << buffer;
            string sendTo = buffer;
            
            //Prints to server
            cout << validUsers[userSourceNo].getName() << ":";
            cout << multiMessage << endl;
            
            //Searches users to send Message to
            for (int i=0;i<4;i++){
                if (validUsers[i].is_Logged_In() && validUsers[i].verifyUser(sendTo)){
                    clientToSocket(multiMessage, validUsers[i], validUsers[userSourceNo]);
                    cout << "Send to user " << i << endl;
                }
                
            }
            
        }
        else if (commandCode == 'l'){
            //LOGOUT Command
            cout << "LOGOUT" << endl;
            
            //Runs logOff() method on UserAccount
            validUsers[userSourceNo].logOff();
            bzero(buffer,256);
            usleep(5);
            
            //Prints to server
            string logmessage = validUsers[userSourceNo].getName() + " has logged off\n";
            cout << validUsers[userSourceNo].getName() << " has logged off\n";
            
            //Sends all other users message that user has logged off
            for (int i=0;i<4;i++){
                if (validUsers[i].is_Logged_In() && userSourceNo != i){
                    serverToSocket(logmessage, validUsers[i]);
                    cout << "Send to user " << i << endl;
                }
                
            }
        }
        else if (commandCode == 'w'){
            //WHO Command
            cout << "WHO" << endl;
            bzero(buffer,256);
            usleep(5);
            string whomessage = "Logged in: ";
            
            //Sends user back string with all logged in users;
            for (int i=0;i<4;i++){
                if (validUsers[i].is_Logged_In()){
                    whomessage += validUsers[i].getName();
                    whomessage += " ";
                }
                
            }
            //Prints to server
            cout << whomessage << endl;
            
            
            serverToSocket(whomessage, validUsers[userSourceNo]);
            
        }
        
        //NoOfThreads--;
        //logout = 1;
        
    }
    //terminate();
    return 0;
}

//Send message to Client from Another Client
int Server::clientToSocket(string message, UserAccounts sendToUser, UserAccounts sendFrom){
    //Create buffer for string transfer
    char buffer[256];
    
    //Get Socket of SendTo Client
    int destSocket = sendToUser.getSocket();
    
    //Send Code character for Message Send
    send(destSocket, "b", 1, 0);
    bzero(buffer,256);
    
    //Send name of Sender
    sendFrom.getName().copy(buffer, 256);
    send(destSocket, buffer, 256, 0);
    bzero(buffer,256);
    
    //Send message to Client
    message.copy(buffer, 256);
    send(destSocket, buffer, 256, 0);
    return 0;
}

//Send message to Client from Server
int Server::serverToSocket(string message, UserAccounts sendToUser){
    //Create buffer for string transfer
    char buffer[256];
    
    //Get Socket of SendTo Client
    int destSocket = sendToUser.getSocket();
    
    //Send Code Character for Server Message
    send(destSocket, "l", 1, 0);
    bzero(buffer,256);
    
    //Send message to Client
    message.copy(buffer, 256);
    send(destSocket, buffer, 256, 0);
    bzero(buffer,256);
    return 0;
}

//Default Constructor
UserAccounts::UserAccounts(){
    this->userName = "";
    this->passWord = "";
    this->isLoggedIn = 0;
}

//Constructor
UserAccounts::UserAccounts(string userName, string passWord){
    this->userName = userName;
    this->passWord = passWord;
    this->isLoggedIn = 0;
}

//Sets account username and password
int UserAccounts::setAccount(string username, string password){
    this->userName = username;
    this->passWord = password;
    return 0;
}

//Verifies if password is valid for user
bool UserAccounts::verifyPassword(string passWord){
    if (passWord == this->passWord){
        return 1;
    }
    else return 0;
}

//Returns 1 if Account Username is equal to string
bool UserAccounts::verifyUser(string userName){
    if (userName == this->userName){
        return 1;
    }
    else return 0;
}

//Logs in User with Username and Password
bool UserAccounts::logIn(string userName, string passWord){
    if (userName == this->userName){
        if (passWord == this->passWord && !isLoggedIn){
            isLoggedIn = true;
            return 1;
        }
        else return 0;
    }
    else return 0;
}

//Logs of User and closes socket
bool UserAccounts::logOff(){
    isLoggedIn = 0;
    close(clientSocket);
    return 1;
}

//Getters
string UserAccounts::getName(){
    return userName;
}
int UserAccounts::getSocket(){
    return clientSocket;
}
void UserAccounts::setSocket(int cliSocket){
    clientSocket = cliSocket;
}
bool UserAccounts::is_Logged_In(){
    return isLoggedIn;
}






