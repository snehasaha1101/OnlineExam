#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <pthread.h>
#include <unordered_map>
#include "auth.h"
#include "exam_manager.h"
#include "grading.h"
#include "seating.h"

using namespace std;

class Server {
public:
    Server(int port);
    void start();
    
private:
    int server_socket;
    static bool handle_authentication(int sock, const string& command, const string& user_type, const string& username, const string& password);    
    static void* handle_client(void* client_socket);
    static void handleStudentExamRequest(int sock, ExamManager exam);
    //static void process_student_request(int sock, const string& username);
    //static void process_instructor_request(int sock, const string& username);
    
};

#endif
