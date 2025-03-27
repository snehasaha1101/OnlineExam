#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>


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
    ExamManager();
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

void Server::handle_authentication(int sock, const string& command, const string& user_type, const string& username, const string& password) {
    char message[128] = {0};

    if (user_type.empty() || username.empty()) {
        strcpy(message, "INVALID_REQUEST");
        send(sock, message, strlen(message), 0);
        cerr << "Invalid role or username received from client.\n";
        close(sock);
        return;
    }

    if (command == "LOGIN") {
        if (AuthManager::authenticate_user(username, password, user_type)) {
            send(sock, "AUTH_SUCCESS", strlen(message), 0);
            cout << username << " logged in successfully as " << user_type << endl;
        } else {
            strcpy(message, "AUTH_FAILED");
            send(sock, message, strlen(message), 0);
            cerr << "Authentication failed for " << username << endl;
        }
    } else if (command == "REGISTER") {
        if (AuthManager::register_user(username, password, user_type)) {
            cout << "[+] Success in register!" << endl;
            send(sock, "REGISTER_SUCCESS", strlen(message), 0);
            cout << username << " registered successfully as " << user_type << endl;
        } else {
            strcpy(message, "REGISTER_FAILED");
            send(sock, message, strlen(message), 0);
            cerr << "Registration failed for " << username << endl;
        }
    } else {
        strcpy(message, "INVALID_COMMAND");
        send(sock, message, strlen(message), 0);
        cerr << "Invalid command received: " << command << endl;
    }
    // **Flush Input Buffer**
    memset(message, 0, sizeof(message));
}


void* Server::handle_client(void* client_socket) {
    int sock = *(int*)client_socket;
    char buffer[1024] = {0};

    ssize_t bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        cerr << "Error: Failed to read data from client\n";
        close(sock);
        return nullptr;
    }

    string request(buffer);
    istringstream iss(request);
    string command, user_type, username, password;
    iss >> command >> user_type >> username >> password;

    if (command == "LOGIN" || command == "REGISTER") {
        handle_authentication(sock, command, user_type, username, password);
    }

    if (user_type == "student") {
        process_student_request(sock, command, username);
    } else if (user_type == "instructor") {
        process_instructor_request(sock, command, username);
    } else {
        send(sock, "INVALID_ROLE", strlen("INVALID_ROLE"), 0);
        cerr << "Invalid role received from client.\n";
    }
    
    close(sock);
    return nullptr;
}

void Server::process_student_request(int sock, const string& command, const string& username) {
    if (command == "TAKE_EXAM") {
        ExamManager exam;
        string exam_data = "Exam started...";//exam.get_exam_questions();
        send(sock, exam_data.c_str(), exam_data.length(), 0);
    } else if (command == "SHOW_DASHBOARD") {
        //Grading grading;
        //string dashboard = grading.get_student_performance(username);
        //send(sock, dashboard.c_str(), dashboard.length(), 0);
    } else {
        send(sock, "INVALID_REQUEST", strlen("INVALID_REQUEST"), 0);
    }
}

void Server::process_instructor_request(int sock, const string& command, const string& username) {
    ExamManager exam;
    if (command == "UPLOAD_EXAM") {
        char buffer[1024] = {0};
        recv(sock, buffer, sizeof(buffer), 0);

        istringstream iss(buffer);
        string exam_name, duration_str;
        iss >> exam_name >> duration_str;
        int duration = stoi(duration_str);

        string response = exam.parse_exam("../data/exams/exam.txt", exam_name, username, duration) ? 
                          "EXAM_UPLOAD_SUCCESS" : "EXAM_UPLOAD_FAILED";
        send(sock, response.c_str(), response.length(), 0);

    }else if (command == "VIEW_UPLOADED_EXAMS") {
        vector<string> instructorExams = exam.get_instructor_exams(username);

        string response;
        int examIndex = 1;
        for (const string& examInfo : instructorExams) {
            response += to_string(examIndex++) + ". " + examInfo + "\n";
        }

        if (response.empty()) {
            response = "No exams uploaded.";
        }

        send(sock, response.c_str(), response.length(), 0);

    }else if (command.rfind("VIEW_EXAM", 0) == 0) {
        int examNumber = stoi(command.substr(10)) - 1;
        vector<string> instructorExams = exam.get_instructor_exams(username);

        if (examNumber < 0 || examNumber >= instructorExams.size()) {
            send(sock, "INVALID_SELECTION", strlen("INVALID_SELECTION"), 0);
            return;
        }

        stringstream ss(instructorExams[examNumber]);
        string exam_file;
        while (getline(ss, exam_file)) {
            if (exam_file.find("Questions File:") != string::npos) {
                exam_file = exam_file.substr(15);  // Remove label
                break;
            }
        }

        string exam_questions = exam.get_exam_questions("../data/exams/" + exam_file);
        send(sock, exam_questions.c_str(), exam_questions.length(), 0);
    }

}

