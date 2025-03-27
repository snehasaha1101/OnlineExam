#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

class Client {
private:
    int sock;
    string role;
    string username;
    string password;

    void authenticate();

    // Student-specific functionalities
    int studentHandler();
    void takeExam();
    void showDashboard();

    // Instructor-specific functionalities
    int instructorHandler();
    void uploadExam();
    void uploadSeatingPattern();
    void showStudentPerformance();
    void viewUploadedExams();
    
    // Static wrappers for pthread_create()
    static void* studentHandlerWrapper(void* arg);
    static void* instructorHandlerWrapper(void* arg);

public:
    Client(const string& ip, int port);
    void start();
};

#endif
