#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

static vector<string> exams;

Server::Server(int port) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Error: Could not create server socket\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Error: Could not bind to port\n";
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        cerr << "Error: Could not listen for connections\n";
        exit(EXIT_FAILURE);
    }
    cout << "[+] Server started on port " << port << endl;
}

void Server::start() {
    AuthManager();
    ExamManager em;
    exams = em.load_exam_metadata("../data/exams/exam_list.txt");
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == -1) {
            cerr << "Error: Could not accept connection\n";
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, nullptr, handle_client, &client_socket);
        pthread_detach(thread);
    }
}

bool Server::handle_authentication(int sock, const string& command, const string& user_type, const string& username, const string& password) {
    if (command == "LOGIN") {
        if (AuthManager::authenticate_user(username, password, user_type)) {
            send(sock, "AUTHENTICATION_SUCCESS", strlen("AUTHENTICATION_SUCCESS"), 0);
            cout << username << " logged in successfully as " << user_type << endl;
            return true;
        } else {
            send(sock, "AUTHENTICATION_FAILED", strlen("AUTHENTICATION_FAILED"), 0);
            cerr << "Authentication failed for " << username << endl;
            return false;
        }
    } else if (command == "REGISTER") {
        if (AuthManager::register_user(username, password, user_type)) {
            send(sock, "REGISTER_SUCCESS", strlen("REGISTER_SUCCESS"), 0);
            cout << username << " registered successfully as " << user_type << endl;
            return true;
        } else {
            send(sock, "REGISTER_FAILED", strlen("REGISTER_FAILED"), 0);
            cerr << "Registration failed for " << username << endl;
            return false;
        }
        
    }
    return false;
}

void* Server::handle_client(void* client_socket) {

    int sock = *(int*)client_socket;
    char buffer[1024] = {0};
    string command, user_type, username, password;
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(sock, buffer, sizeof(buffer), 0);
        string request(buffer);
        istringstream iss(request);
        iss >> command >> user_type >> username >> password;
        if(handle_authentication(sock, command, user_type, username, password)) break;
    }

    if (user_type == "student") {
        while (true){
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) break;
            buffer[bytes_received] = '\0';
            string request(buffer);

            if (request == "1") {
                string all_exams;
                int qno = 1;
                for (auto& exam : exams) {
                    string formattedExam;
                    istringstream iss(exam);
                    string line;
                    
                    while (getline(iss, line)) 
                        formattedExam += line + " | ";
        
                    if (!formattedExam.empty() && formattedExam.back() == ' ')
                        formattedExam.pop_back();

                    all_exams += to_string(qno) + ". " + formattedExam + "\n";
                    qno++;
                }
            
                if (all_exams.empty())
                    all_exams = "No exams available.\n";
                
                all_exams += "\0";
                send(sock, all_exams.c_str(), all_exams.size(), 0);
            }
            
            
            else if (request == "2") {
                send(sock, "Exam starting..", strlen("Exam starting.."), 0);
            }
            else if(request == "3") break;
        }
    }
    else if (user_type == "instructor") {
        while (true){
            ExamManager exam_manager;
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }
            buffer[bytes_received] = '\0';
            string request(buffer);
            string response = "";

            if (request == "1") {
                memset(buffer, 0, sizeof(buffer));
                recv(sock, buffer, sizeof(buffer), 0);
                string examData(buffer);
            
                size_t pos1 = examData.find("|");
                size_t pos2 = examData.find("|", pos1 + 1);
            
                if (pos1 == string::npos || pos2 == string::npos) {
                    response = "Error: Invalid format. Use 'Exam Name | Duration | FileName'";
                } else {
                    string examName = examData.substr(0, pos1);  
                    int examDuration = atoi(examData.substr(pos1 + 1, pos2 - pos1 - 1).c_str());
                    string examFileName = "../data/exams/" + examData.substr(pos2 + 1);
                    
                    if (exam_manager.parse_exam(examFileName, examName, username, examDuration)) {
                        exams = exam_manager.load_exam_metadata("../data/exams/exam_list.txt");
                        response = "Exam successfully uploaded!"; 
                    } else {
                        response = "Error: Invalid exam format!";
                    }
                }
                send(sock,response.c_str(),response.size(),0);
            }
            else if(request == "2"){
                send(sock, "upload exam sheet...", strlen("upload exam sheet..."), 0);
            }
            else if (request == "3"){
                send(sock, "student performance...", strlen("student performance..."), 0);
            }
            else if (request == "4") {
                string all_exams;
                int qno = 1;  // Start numbering from 1
            
                for (const auto& exam : exams) {
                    istringstream iss(exam);
                    ostringstream filteredExam;
                    string line;
                    string instructor_name;
                    bool include_exam = false;
            
                    while (getline(iss, line)) {
                        if (line.find("Instructor:") != string::npos) {
                            instructor_name = line.substr(line.find(":") + 2);  // Extract instructor name
                            if (instructor_name == username) {
                                include_exam = true;  // Match found, include this exam
                            }
                        }
                        if (line.find("Instructor:") != std::string::npos)
                            continue;
                        filteredExam << line << " | ";
                    }
            
                    if (include_exam) {
                        all_exams += to_string(qno++) + ". " + filteredExam.str() + "\n";
                    }
                }
            
                if (all_exams.empty()) {
                    all_exams = "No exams available for this instructor.\n";
                }
            
                all_exams += "\0";
                send(sock, all_exams.c_str(), all_exams.size(), 0);

            }
            
            else if (response=="5") break;
        }
    }
    close(sock);
    cout << "[-] client disconnected!"<<endl;
    return nullptr;
}




