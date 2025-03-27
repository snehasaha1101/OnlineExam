#include "client.h"
#include <cstring>
#include <algorithm>
#include <unistd.h>

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

void* Client::studentHandlerWrapper(void* arg) {
    Client* client = static_cast<Client*>(arg);
    while (client->studentHandler() != -1); // Keep handling until logout
    return nullptr;
}

void* Client::instructorHandlerWrapper(void* arg) {
    Client* client = static_cast<Client*>(arg);
    while (client->instructorHandler() != -1); // Keep handling until logout
    return nullptr;
}

void Client::authenticate() {
    int choice;
    while (true) {
        cout << "\n1. Login\n2. Register\n3. Exit\nChoose an option: ";
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
            cerr << "Invalid role! Please enter S or I.\n";
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
        recv(sock, response, sizeof(response), 0);
        string server_reply(response);
        cout << "[+] "<<server_reply<<endl;
        if (server_reply == "AUTH_SUCCESS" || server_reply == "REGISTER_SUCCESS") {
            cout << (choice == 1 ? "Login successful!\n" : "Registration successful!\n");
            break;
        } else {
            cout << server_reply << endl;
            cerr << (choice == 1 ? "Login failed! Try again.\n" : "Registration failed! Username may already exist.\n");
        }
    }
}

// ----------------- Student Handler -----------------
int Client::studentHandler() {
    int choice;
    cout << "\nStudent Menu:\n";
    cout << "1. Take Exam\n";
    cout << "2. Show Dashboard\n";
    cout << "3. Logout\n";
    cout << "Choose an option: ";
    cin >> choice;

    if (choice == 3) {
        cout << "Logging out...\n";
        close(sock);
        return -1; // Signal thread to exit
    }

    string request;
    switch (choice) {
        case 1:
            request = "TAKE_EXAM " + username;
            send(sock, request.c_str(), request.length(), 0);
            takeExam();
            break;
        case 2:
            request = "SHOW_DASHBOARD " + username;
            send(sock, request.c_str(), request.length(), 0);
            showDashboard();
            break;
        default:
            cerr << "Invalid choice! Try again.\n";
            return 0; // Keep thread running
    }

    return 0; // Keep thread running
}

// ----------------- Instructor Handler -----------------
int Client::instructorHandler() {
    int choice;
    cout << "\nInstructor Menu:\n";
    cout << "1. Upload Exam\n";
    cout << "2. Upload Seating Pattern\n";
    cout << "3. Show Student Performance\n";
    cout << "4. View Uploaded Exams\n";
    cout << "5. Logout\n";
    cout << "Choose an option: ";
    cin >> choice;

    if (choice == 5) {
        cout << "Logging out...\n";
        close(sock);
        return -1; // Signal thread to exit
    }

    string request;
    switch (choice) {
        case 1:
            request = "UPLOAD_EXAM " + username;
            send(sock, request.c_str(), request.length(), 0);
            uploadExam();
            break;
        case 2:
            request = "UPLOAD_SEATING_PATTERN " + username;
            send(sock, request.c_str(), request.length(), 0);
            uploadSeatingPattern();
            break;
        case 3:
            request = "SHOW_STUDENT_PERFORMANCE " + username;
            send(sock, request.c_str(), request.length(), 0);
            showStudentPerformance();
            break;
        case 4:
            request = "VIEW_UPLOADED_EXAMS " + username;
            send(sock, request.c_str(), request.length(), 0);
            viewUploadedExams();
            break;
        default:
            cerr << "Invalid choice! Try again.\n";
            return 0; // Keep thread running
    }

    return 0; // Keep thread running
}

// ----------------- Student Functionalities -----------------
void Client::takeExam() {
    char response[1024] = {0};
    recv(sock, response, sizeof(response), 0);
    cout << "Exam Started: " << response << endl;
}

void Client::showDashboard() {
    char response[1024] = {0};
    recv(sock, response, sizeof(response), 0);
    cout << "Dashboard: " << response << endl;
}

// ----------------- Instructor Functionalities -----------------
void Client::uploadExam() {
    cout << "Enter exam filename: ";
    string filename;
    cin >> filename;
    string request = "UPLOAD_EXAM_FILE " + filename;
    send(sock, request.c_str(), request.length(), 0);
}

void Client::uploadSeatingPattern() {
    cout << "Enter seating pattern filename: ";
    string filename;
    cin >> filename;
    string request = "UPLOAD_SEATING_PATTERN_FILE " + filename;
    send(sock, request.c_str(), request.length(), 0);
}

void Client::showStudentPerformance() {
    cout << "Enter student username: ";
    string student_username;
    cin >> student_username;
    string request = "SHOW_STUDENT_PERFORMANCE " + student_username;
    send(sock, request.c_str(), request.length(), 0);
}

void Client::viewUploadedExams() {
    string request = "VIEW_UPLOADED_EXAMS " + username;
    send(sock, request.c_str(), request.length(), 0);
    char response[1024] = {0};
    recv(sock, response, sizeof(response), 0);
    cout << "Uploaded Exams: " << response << endl;
}

void Client::start() {
    authenticate();  // First, authenticate the user

    pthread_t thread;
    if (role == "s") {
        pthread_create(&thread, nullptr, studentHandlerWrapper, this);
    } else {
        pthread_create(&thread, nullptr, instructorHandlerWrapper, this);
    }

    pthread_join(thread, nullptr);
}
