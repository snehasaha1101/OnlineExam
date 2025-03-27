#ifndef AUTH_H
#define AUTH_H

#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;

class AuthManager {
private:
    static unordered_map<string, string> student_db;   // Stores students (username -> hashed password)
    static unordered_map<string, string> instructor_db; // Stores instructors (username -> hashed password)

    static string hash_password(const string& password);
    static void load_users(const string& filename, unordered_map<string, string>& user_db);
    static bool save_user(const string& filename, const string& username, const string& password);

public:
    AuthManager(); // Constructor
    static bool register_user(const string& username, const string& password, const string& user_type);
    static bool authenticate_user(const string& username, const string& password, const string& user_type);
};
#endif
