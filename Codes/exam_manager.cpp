#include "exam_manager.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

using namespace std;

ExamManager::ExamManager() {
    load_exam_data();
}

bool ExamManager::parse_exam(const string& input_file, const string& exam_name, const string& instructor, int duration) {
    int fd = open(input_file.c_str(), O_RDONLY);
    if (fd == -1) {
        cerr << "[-] Error: Could not open file " << input_file << endl;
        return false;
    }

    vector<string> questions, answers;
    string currentQuestion, optionA, optionB, optionC, optionD, correctAnswer;
    char buffer[1024];
    ssize_t bytes_read;
    bool readingQuestion = false;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        string line(buffer);
        
        if (line.rfind("Q:", 0) == 0) {
            if (!currentQuestion.empty()) {
                questions.push_back(currentQuestion + "\nA) " + optionA + "\nB) " + optionB + "\nC) " + optionC + "\nD) " + optionD);
                answers.push_back(correctAnswer);
            }
            currentQuestion = line.substr(2);
            readingQuestion = true;
        } else if (line.rfind("A)", 0) == 0) {
            optionA = line.substr(2);
            readingQuestion = false;
        } else if (line.rfind("B)", 0) == 0) {
            optionB = line.substr(2);
        } else if (line.rfind("C)", 0) == 0) {
            optionC = line.substr(2);
        } else if (line.rfind("D)", 0) == 0) {
            optionD = line.substr(2);
        } else if (line.rfind("A:", 0) == 0) {
            correctAnswer = line.substr(2);
        } else if (readingQuestion) {
            currentQuestion += " " + line;
        }
    }

    close(fd);

    if (!currentQuestion.empty()) {
        questions.push_back(currentQuestion + "\nA) " + optionA + "\nB) " + optionB + "\nC) " + optionC + "\nD) " + optionD);
        answers.push_back(correctAnswer);
    }

    if (questions.empty()) {
        cerr << "[-] Error: No valid questions found in file." << endl;
        return false;
    }

    string metadata_file = "../data/exams/metadata_" + exam_name + ".txt";
    string questions_file = "../data/exams/questions_" + exam_name + ".txt";
    string answers_file = "../data/exams/answers_" + exam_name + ".txt";

    int meta_fd = open(metadata_file.c_str(), O_WRONLY | O_CREAT, 0644);
    string metadata = "Exam Name: " + exam_name + "\nDuration: " + to_string(duration) + "\nInstructor: " + instructor + "\nQuestions File: " + questions_file + "\nAnswers File: " + answers_file + "\n";
    write(meta_fd, metadata.c_str(), metadata.size());
    close(meta_fd);

    int ques_fd = open(questions_file.c_str(), O_WRONLY | O_CREAT, 0644);
    for (const auto& q : questions) {
        write(ques_fd, q.c_str(), q.size());
        write(ques_fd, "\n\n", 2);
    }
    close(ques_fd);

    int ans_fd = open(answers_file.c_str(), O_WRONLY | O_CREAT, 0644);
    for (const auto& a : answers) {
        write(ans_fd, a.c_str(), a.size());
        write(ans_fd, "\n", 1);
    }
    close(ans_fd);

    int exam_list_fd = open("../data/exams/exam_list.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    string entry = exam_name + "|" + metadata_file + "\n";
    write(exam_list_fd, entry.c_str(), entry.size());
    close(exam_list_fd);

    cout << "[+] Exam successfully parsed and stored!" << endl;
    return true;
}

void ExamManager::load_exam_data() {
    examDataList.clear();  // Clear old data before reloading

    int exam_list_fd = open("data/exams/exam_list.txt", O_RDONLY);
    if (exam_list_fd == -1) {
        cerr << "[-] Error: Could not open exam_list.txt\n";
        return;
    }

    char buffer[4096];
    ssize_t bytes_read = read(exam_list_fd, buffer, sizeof(buffer) - 1);
    close(exam_list_fd);

    if (bytes_read <= 0) {
        cerr << "[-] Error: Could not read exam list file.\n";
        return;
    }
    buffer[bytes_read] = '\0';  // Null terminate

    stringstream exam_list_ss(buffer);
    string line;
    
    while (getline(exam_list_ss, line)) {
        stringstream ss(line);
        string examName, metadataFile;

        // Extract exam name and metadata file
        getline(ss, examName, '|');
        getline(ss, metadataFile);

        // Trim spaces
        metadataFile.erase(0, metadataFile.find_first_not_of(" "));

        int meta_fd = open(metadataFile.c_str(), O_RDONLY);
        if (meta_fd == -1) {
            cerr << "[-] Warning: Could not open metadata file " << metadataFile << "\n";
            continue;
        }

        char meta_buffer[2048];
        ssize_t meta_bytes_read = read(meta_fd, meta_buffer, sizeof(meta_buffer) - 1);
        close(meta_fd);

        if (meta_bytes_read <= 0) {
            cerr << "[-] Warning: Could not read metadata file " << metadataFile << "\n";
            continue;
        }
        meta_buffer[meta_bytes_read] = '\0';

        stringstream meta_ss(meta_buffer);
        string metadata, metaLine;
        while (getline(meta_ss, metaLine)) {
            if (metaLine.find("Questions File") == string::npos && 
                metaLine.find("Answers File") == string::npos) {
                metadata += metaLine + " ";
            }
        }

        examDataList.push_back(metadata);
    }

    cout << "[+] Exam data loaded successfully!\n";
}

// Returns a copy of `examDataList` (preloaded exam data)
vector<string> ExamManager::get_exam_data() {
    return examDataList;
}

// Returns a list of exams uploaded by a specific instructor
vector<string> ExamManager::get_instructor_exams(const string& instructor) {
    vector<string> instructorExams;
    for (const string& examEntry : examDataList) {
        if (examEntry.find("Instructor: " + instructor) != string::npos) {
            instructorExams.push_back(examEntry);
        }
    }
    return instructorExams;
}

// Returns questions from the selected exam file
string ExamManager::get_exam_questions(const string& exam_file) {
    int fd = open(exam_file.c_str(), O_RDONLY);
    if (fd == -1) {
        cerr << "[-] Error: Could not open exam file: " << exam_file << endl;
        return "ERROR_OPENING_FILE";
    }

    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read <= 0) {
        cerr << "[-] Error: Could not read exam file.\n";
        return "ERROR_READING_FILE";
    }
    buffer[bytes_read] = '\0';

    return string(buffer);
}
