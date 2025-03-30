#ifndef EXAM_MANAGER_H
#define EXAM_MANAGER_H

#include <string>
#include <vector>
#include <sstream>
#include <netinet/in.h>

using namespace std;

class ExamManager {
public:
    bool parse_exam(const string& input_file, const string& exam_name, const string& instructor, int duration);
    //vector<string> get_exam_data();  // Returns preloaded exam metadata
    //vector<string> get_instructor_exams(string instructor);  // View uploaded exams
    //string get_exam_questions(const string& exam_file);  // View questions of a selected exam
    void handleStudentExamRequest(int sock);
    vector<string> load_exam_metadata(const string& exam_list_file);
    string getMetadataFilePath(const string& examName);
    string getQuestionsFilePath(const string& metadataPath) ;
    void sendExamQuestions(int sock, const string& examName);
};

#endif // EXAM_MANAGER_H
