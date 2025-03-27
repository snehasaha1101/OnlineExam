#ifndef EXAM_MANAGER_H
#define EXAM_MANAGER_H

#include <string>
#include <vector>
#include <sstream>

using namespace std;

class ExamManager {
private:
    vector<string> examDataList;  // Stores all exam metadata

public:
    ExamManager();  // Constructor to load exam data at startup
    bool parse_exam(const string& input_file, const string& exam_name, const string& instructor, int duration);
    void load_exam_data();  // Loads exam data into `examDataList`
    vector<string> get_exam_data();  // Returns preloaded exam metadata

    vector<string> get_instructor_exams(const string& instructor);  // View uploaded exams
    string get_exam_questions(const string& exam_file);  // View questions of a selected exam
};

#endif // EXAM_MANAGER_H
