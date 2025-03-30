#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

using namespace std;

class Client {
private:
    int sock;
    string role;
    string username;
    string password;

    void authenticate();
    static void* studentHandler(void* arg);
    static void* instructorHandler(void* arg);

    // UI Helper Functions
    void displayHeader(const string& title);
    void displaySuccess(const string& message);
    void displayError(const string& message);
    void displayMenu();
    static void displayStudentMenu();       // Shows the student menu
    static void displayInstructorMenu(); 
    static void receiveAndStoreExamQuestions(int sock, int examNumber);   // Shows the instructor menu

public:
    Client(const string& ip, int port);
    void start();
};

#endif
