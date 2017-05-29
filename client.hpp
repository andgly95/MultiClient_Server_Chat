//
//  client.hpp
//  
//
//  Created by Andrew on 5/23/17.
//
//

#ifndef client_h
#define client_h

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

#define PORTNO 12004;

using namespace std;

//Compile with g++ -std=c++11 client.cpp -o client

/**
 *
 *   Client Class
 *
 *  Creates client to connect to server, allows for chat with multiple client through server
 *
 **/
class Client{
private:
    int serverSocket;
    int portNo = PORTNO;
    char userNo;
public:
    void error(const char *msg);
    
    //Creates socket configuration for server Socket and connects to Server
    int configure();
    
    //Logs in User to Server
    int login();
    
    //Multicasts Message to All Users
    int sendAll(string message);
    
    //Unicasts Message to Specified User
    int sendTo(string message, string user);
    
    //Receives Messages from Server
    int receive();
    
    //Spawns thread to listen to Server Messages
    int listen();
    
    //Sends who request to server
    int who();
    
    //Logs out client and closes socket
    int logout();
};

#endif /* client_h */
