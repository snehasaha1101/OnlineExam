#include "client.h"
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <iomanip>
#include <fstream>

// Function to display a fancy header
void Client::displayHeader(const string& title) {
    cout << "╔";
    for (int i = 0; i < 30; i++) cout << "═";  // Repeat '=' character instead of using '═'
    cout << "╗\n";
    cout << "║ " << left << setw(28) << title << " ║\n";

    cout << "╚";
    for (int i = 0; i < 30; i++) cout << "═";
    cout << "╝\n";
}

// Function to display success message
void Client::displaySuccess(const string& message) {
    cout << "[✔] " << message << endl;
}

// Function to display error message
void Client::displayError(const string& message) {
    cout << "[✖] " << message << endl;
}

// Function to display menu options in a structured format
void Client::displayMenu() {
    cout << "\n=======";
    cout << "  Main Menu  ";
    cout << "=======\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "----------------------\n";
    cout << "Choose an option: ";
}

// Function to display Student Menu
void Client::displayStudentMenu() {
    cout << "\n========";
    cout << "  Student Menu  ";
    cout << "=========\n";
    cout << "1. Take Exam\n";
    cout << "2. Show Dashboard\n";
    cout << "3. Logout\n";
    cout << "-----------------------\n";
    cout << "Choose an option: ";
}

// Function to display Instructor Menu
void Client::displayInstructorMenu() {
    cout << "\n=====";
    cout << "  Instructor Menu  ";
    cout << "=====\n";
    cout << "1. Upload Exam\n";
    cout << "2. Upload Seating Pattern\n";
    cout << "3. Show Student Performance\n";
    cout << "4. View Uploaded Exams\n";
    cout << "5. Logout\n";
    cout << "----------------------\n";
    cout << "Choose an option: ";
}

// Constructor to set up the client connection
Client::Client(const string& server_ip, int server_port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Error: Could not create socket\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Error: Connection to server failed\n";
        exit(EXIT_FAILURE);
    }
}

// Handles student-specific actions
void* Client::studentHandler(void* arg) {
    Client* client = static_cast<Client*>(arg);
    char buffer[1024] = {0};
    int choice;

    while (true) {
        displayStudentMenu();
        cin >> choice;


        sprintf(buffer, "%d", choice);
        send(client->sock, buffer, strlen(buffer), 0);

        if (choice == 3) {
            cout << "Logging out...\n";
            close(client->sock);
            return nullptr;  // Signal thread to exit
        } else if (choice == 1) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recv(client->sock, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) {
                cerr << "Error: Failed to read data from server\n";
                close(client->sock);
                return nullptr;
            }
            cout << buffer << endl;
            cout << "press 0 to go back to main menu\n";
            cout << "select exam number to start the exam: ";
            
            cin >> choice;
            sprintf(buffer, "%d", choice);
            if(choice>0){
                receiveAndStoreExamQuestions(client->sock,choice);
            }

        } else if(choice == 2){
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recv(client->sock, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) {
                cerr << "Error: Failed to read data from server\n";
                close(client->sock);
                return nullptr;
            }
            cout << buffer << endl;
        }
        else {
            cout << "Invalid choice! Please select a valid option.\n";
        }
    }
    return nullptr;
}

void Client::receiveAndStoreExamQuestions(int sock, int examNumber) {
    char buffer[4096] = {0};  // Buffer for receiving data

    // Send selected exam number to the server
    string examSelection = to_string(examNumber);
    send(sock, examSelection.c_str(), examSelection.size(), 0);

    // Receive the questions file content
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        cerr << "Error: Failed to receive exam questions from server.\n";
        return;
    }

    string server_reply(buffer);
        if (server_reply == "Error: Invalid exam selection") {
            cout << "[+] " << server_reply << endl;
            return;
        }
    buffer[bytesReceived] = '\0';  // Null-terminate the received data
    // Create a file to store the questions
    string fileName = "./exams/exam_" + to_string(examNumber) + ".txt";
    ofstream outFile(fileName);
    if (!outFile) {
        cerr << "Error: Unable to create file " << fileName << "\n";
        return;
    }

    outFile << buffer;  // Write received data to file
    outFile.close();
    cout << "[+] Question received successfully\n";
    cout << "Exam questions stored in file: " << fileName << endl;
}


// Handles instructor-specific actions
void* Client::instructorHandler(void* arg) {
    Client* client = static_cast<Client*>(arg);
    char buffer[1024] = {0};
    int choice;

    while (true) {
        displayInstructorMenu();
        cin >> choice;

        sprintf(buffer, "%d", choice);
        send(client->sock, buffer, strlen(buffer), 0);

        if (choice == 5) {
            cout << "Logging out...\n";
            close(client->sock);
            return nullptr; // Signal thread to exit
        } else if (choice == 1) {
            while ((getchar()) != '\n'); // Clear buffer
            string examName, duration, fileName;

            cout << "Enter Exam Name: ";
            getline(cin, examName);
            cout << "Enter Exam Duration (minutes): ";
            cin >> duration;
            cout << "Enter Exam File Name: ";
            cin >> fileName;

            examName += "|" + duration + "|" + fileName;
            send(client->sock, examName.c_str(), examName.size(), 0);

            memset(buffer, 0, sizeof(buffer));
            recv(client->sock, buffer, sizeof(buffer), 0);
            cout << buffer << endl;
        } else if (choice >= 2 && choice <= 4) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recv(client->sock, buffer, sizeof(buffer), 0);
            cout << buffer << endl;
        } else {
            cout << "Invalid choice! Please select a valid option.\n";
        }
    }
    return nullptr;
}

// Handles user authentication (Login/Register)
void Client::authenticate() {
    int choice;
    displayHeader("Welcome to the Exam System");

    while (true) {
        displayMenu();
        cin >> choice;

        if (choice == 3) {
            cout << "Exiting...\n";
            close(sock);
            exit(0);
        }

        cout << "Enter role (S for Student, I for Instructor): ";
        cin >> role;
        transform(role.begin(), role.end(), role.begin(), ::tolower);

        if (role != "s" && role != "i") {
            displayError("Invalid role! Please enter S or I.");
            continue;
        }

        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        string user_type = (role == "s") ? "student" : "instructor";
        string request = (choice == 1) ? "LOGIN " : "REGISTER ";
        request += user_type + " " + username + " " + password;

        send(sock, request.c_str(), request.length(), 0);

        char response[1024] = {0};
        int bytes_read = recv(sock, response, sizeof(response), 0);
        if (bytes_read <= 0) {
            displayError("Error: Failed to read data from server.");
            close(sock);
            return;
        }

        string server_reply(response);
        if (server_reply == "AUTHENTICATION_SUCCESS" || server_reply == "REGISTER_SUCCESS") {
            cout << "[+] " << server_reply << endl;
            break;
        } else {
            displayError(choice == 1 ? "Login failed! Try again." : "Registration failed! Username may already exist.");
        }
    }
}

// Starts the client and assigns the appropriate handler
void Client::start() {
    authenticate();

    pthread_t thread;
    if (role == "s") {
        pthread_create(&thread, nullptr, studentHandler, this);
    } else {
        pthread_create(&thread, nullptr, instructorHandler, this);
    }
    pthread_join(thread, nullptr);
}