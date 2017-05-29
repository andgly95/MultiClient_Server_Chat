//
//  server.hpp
//  
//
//  Created by Andrew on 5/23/17.
//
//
#ifndef server_h
#define server_h
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
#include "server.hpp"

#define MAXCLIENTS 3;
#define PORTNO 12004;

int NoOfThreads = 1;

using namespace std;

//Compile with g++ -std=c++11 server.cpp -o server

/**
 *
 *   User Accounts Class
 *
 *  Used to store valid user accounts for the server
 *
 **/

class UserAccounts{
private:
    string userName;
    string passWord;
    bool isLoggedIn;
    int clientSocket;
public:
    
    //Constructors
    UserAccounts(string userName, string passWord);
    UserAccounts();
    //Set account
    int setAccount(string username, string password);
    //Verify usernames and passwords
    bool verifyPassword(string passWord);
    bool verifyUser(string userName);
    //Log on/Log off
    bool logIn(string userName, string passWord);
    bool logOff();
    //Getters
    string getName();
    int getSocket();
    void setSocket(int cliSocket);
    bool is_Logged_In();
};

/**
 *
 *   Server Class
 *
 *  Creates and runs server, allows for multiple client connections
 *
 **/

class Server{
private:
    int activeClients = 0;
    int serverSocket;
    int portNo = PORTNO;
    int* clientSockets;
    UserAccounts* validUsers;
public:
    void error(const char *msg)
    {
        perror(msg);
        exit(1);
    }
    
    //Configures server socket for connections
    int configure();
    
    //Logs in User Client to Server, Creates New Thread
    int newConnection(int socket);
    
    //Sends messages from Clients to other Clients
    int clientToSocket(string message, UserAccounts sendToUser, UserAccounts sendFrom);
    
    //Sends messages from Server to Clients
    int serverToSocket(string message, UserAccounts sendToUser);
    
    //Runs commands from clients, Creates New Thread
    int receive(int socket);
};

#endif /* server_h */
