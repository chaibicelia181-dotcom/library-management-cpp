#include<iostream>
#include"sqlite3.h"
#include<string>
#include<vector>
#include <stdexcept>
#include <limits>



class User {
    private:
    int user_id;
    std::string first_name;
    std::string last_name;
    std::string password;
    std::string email;

    public:
    User(){
        user_id = 0;
        first_name = "Enter your first name";
        last_name = "Enter your last name";
        email ="abcd@mail.com";
        password  = "****";
    }
    User(int I,std::string P,std::string E,std::string F,std::string L){
        email = E;
        password = P;
        first_name = F;
        last_name = L;
        user_id = I;
    }
    int getUserId() const{
        return user_id;

    }
    std::string getemail() const{
        return email;
    }
    bool checkpassword(const std::string& ent_password) {
        return password == ent_password;
    }
    std::string get_first_name() const{
        return first_name;
    }
    std::string get_last_name() const{
        return last_name;
    }
    void displayUsers(){
        std::cout <<"id: "<< user_id
        <<"| First name: " << first_name
        <<"| Last name: " << last_name
        <<"| Email: " <<email;
    
    }
};

class Book {
    private:
    int bookId;
    std::string title;
    std::string author;
    int isIssued;

    public:
    Book(){
        title = "Unknown";
        author = "Unknown";
        bookId = 0;
        isIssued = 0;

    }
    Book(std::string T,std::string A, int x){
        bookId = x;
        author = A;
        title  =T;
        isIssued = 0;
  
    }
    int getId() const{
        return bookId;  
    }
    
    int getstatus () const{
        return isIssued;
    }
    void displayBook(){
        std::cout << "ID: " << bookId
        << "| Title: " << title
        <<"| Author: " <<author
        <<"| Status: " << (isIssued ? "issued" : "Available")
        << "\n";
    }
    void issuebook(){
        isIssued = 1;
    }
    void markReturned(){
        isIssued = 0;
    }
};

class BookIssue {
private:
    int issue_id;
    int user_id;
    int book_id;
    int isReturned;   // 0 = not returned, 1 = returned

public:
    BookIssue(int iid, int uid, int bid)
        : issue_id(iid), user_id(uid), book_id(bid), isReturned(0) {}

    int getUserId() const { return user_id; }
    int getBookId() const { return book_id; }
    int returned() const { return isReturned; }
    void markReturned() { isReturned = 1; }
};

//we include the poointer to the database in the library class, because it is the class that manages all the library system, and it falls to the same abstarction level as the database
class Library {
    private:
    std::vector<Book> Books;
    std::vector<User> Users;
    std::vector<BookIssue> Issues;
    //add the database into a the library class
    sqlite3* db;//sqlite3* is a pointer to an open database connection
    public:
    Library() : db(nullptr) {// db starts as a null pointer
    int rc = sqlite3_open("library_management_system/library.db", &db);

    if (rc != SQLITE_OK) {// if sqlite3 opens (if db points to an open database)
        std::cerr << "Cannot open database: " // cerr is like cout but we use it for error messasge because it is unbuffered and appears immediatly even before the error
                  << sqlite3_errmsg(db) << "\n"; //Returns a human-readable error message
        sqlite3_close(db);//close the database 
        db = nullptr;
        throw std::runtime_error("Database open failed");// immediatly stop the executing the program
    }

    // Enable foreign keys (IMPORTANT in SQLite)
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    std::cout << "Database connected successfully.\n";
    }
    //deconstructor, close the database connection to avoid memory leaks 
    ~Library() {
    if (db) {
        sqlite3_close(db);
    }
    }  

    //users methods

    void addUser(std::string first_name, std::string last_name,
                 std::string email, std::string password) {
        const char* sql = "INSERT INTO users (first_name, last_name, email, password) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, first_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, last_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, password.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                std::cout << "\nUser added successfully.\n";
            } else {
                std::cerr << "\nError inserting user: " << sqlite3_errmsg(db) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void listAllUsers() {
        const char* sql = "SELECT id, first_name, last_name, email FROM users;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {//SQLITE ROW = a row is available    //SQLITE_DONE = No more available row
                int id = sqlite3_column_int(stmt, 0);//read column 0
                const unsigned char* fn = sqlite3_column_text(stmt, 1);//read column 1 , Type is const unsigned char* (SQLite uses unsigned bytes)
                const unsigned char* ln = sqlite3_column_text(stmt, 2);//read column 2
                const unsigned char* em = sqlite3_column_text(stmt, 3);//read column 3
                std::cout << "Id: " << id
                          << " | First name: " << fn
                          << " | Last name: " << ln
                          << " | Email: " << em << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    }


     void getUserById(int id) {  
        const char* sql = "SELECT id, first_name, last_name, email FROM users WHERE id=?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) { 
                std::cout << "Id: " << sqlite3_column_int(stmt, 0)
                          << " | First name: " << sqlite3_column_text(stmt, 1)
                          << " | Last name: " << sqlite3_column_text(stmt, 2)
                          << " | Email: " << sqlite3_column_text(stmt, 3) << "\n";
            } else {
                std::cout << "\nUser not found.\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void getUserByEmail(std::string email){
        const char* sql = "SELECT id, first_name, last_name, email FROM users WHERE email LIKE ? COLLATE NOCASE;";
        sqlite3_stmt* stmt  = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string pattern = "%" + email + "%";
            sqlite3_bind_text(stmt, 1,pattern.c_str(),-1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) { 
                std::cout << "Id: " << sqlite3_column_int(stmt, 0)
                          << " | First name: " << sqlite3_column_text(stmt, 1)
                          << " | Last name: " << sqlite3_column_text(stmt, 2)
                          << " | Email: " << sqlite3_column_text(stmt, 3) << "\n";
            } else {
                std::cout << "\nUser not found.\n";
            }
        }
        if(stmt) sqlite3_finalize(stmt);
    }

    //book methods
    bool userExists(int id) {
        const char* sql = "SELECT id FROM users WHERE id=?;";
        sqlite3_stmt* stmt = nullptr;
        bool exists = false;
        // if the sql query is correctly compiled
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            // we raplace the place holder with the actual id
            sqlite3_bind_int(stmt, 1, id);
            //execute the query with the id of the user in the placeholder
            exists = (sqlite3_step(stmt) == SQLITE_ROW);
            //returns true if the row exists or false otherwise
        }
        if (stmt) sqlite3_finalize(stmt);
        return exists;
    }

    bool userExists(int id) {
        const char* sql = "SELECT id FROM users WHERE id=?;";
        sqlite3_stmt* stmt = nullptr;
        bool exists = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            exists = (sqlite3_step(stmt) == SQLITE_ROW);
        }
        if (stmt) sqlite3_finalize(stmt);
        return exists;
    }



    void getActiveIssuedBooks() {
        const char* sql =
            "SELECT u.first_name, u.last_name, u.email, b.id "
            "FROM book_issues bi "
            "JOIN users u ON bi.user_id = u.id "
            "JOIN books b ON bi.book_id = b.id "
            "WHERE bi.return_date IS NULL;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const unsigned char* fn = sqlite3_column_text(stmt, 0);
                const unsigned char* ln = sqlite3_column_text(stmt, 1);
                const unsigned char* em = sqlite3_column_text(stmt, 2);
                int bookId = sqlite3_column_int(stmt, 3);
                std::cout << "User: " << fn << " " << ln
                          << " | Email: " << em
                          << " | Book ID: " << bookId
                          << " | Status: Issued\n";
                found = true;
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) {
            std::cout << "\nNo active issued books.\n";
        }
    
    }



 

    void issuebook(int book_id, int user_id) {
        if (!userExists(user_id)) {
            std::cout << "\nUser ID " << user_id << " does not exist.\n";
            return;
        }
        if (!bookExists(book_id)) {
            std::cout << "\nBook ID " << book_id << " does not exist.\n";
            return;
        }
        if (bookIsIssued(book_id)) {
            std::cout << "\nThis book is already issued.\n";
            return;
        }
        
        const char* sql = "INSERT INTO book_issues (user_id, book_id) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) { // -1 means that sql reads the string until \0 meaning that it is calculating its length
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_int(stmt, 2, book_id);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                std::cout << "\nBook issued successfully.\n";
            } else {
                std::cerr << "\nError issuing book: " << sqlite3_errmsg(db) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    }


        
    

    void addbook(std::string title,std::string author) {

        const char* sql = "INSERT INTO books (title, author) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                std::cout << "\nBook added successfully.\n";
            } else {
                std::cerr << "\nError inserting book: " << sqlite3_errmsg(db) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    
        
    }

    void returnbook(int Book_id, int User_id) {
        sqlite3_stmt* stmt = nullptr;
        if (!userExists(user_id)) {
            std::cout << "\nUser ID " << user_id << " does not exist.\n";
            return;
        }
        if (!bookExists(book_id)) {
            std::cout << "\nBook ID " << book_id << " does not exist.\n";
            return;
        }
    

        // 1️Verify this user actually borrowed the book
        const char* check_sql =
            "SELECT id FROM book_issues "
            "WHERE book_id=? AND user_id=? AND return_date IS NULL;";
    
        if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << sqlite3_errmsg(db) << "\n";
            return;
        }
    
        sqlite3_bind_int(stmt, 1, book_id);
        sqlite3_bind_int(stmt, 2, user_id);
    
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    
        if (rc != SQLITE_ROW) {
            std::cout << "\nThis user does not have this book issued.\n";
            return;
        }
    
        // 2️Mark book as returned 
        const char* return_sql =
            "UPDATE books SET is_issued=0 WHERE id=?;";
    
        if (sqlite3_prepare_v2(db, return_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << sqlite3_errmsg(db) << "\n";
            return;
        }
    
        sqlite3_bind_int(stmt, 1, book_id);
    
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "\nBook returned successfully.\n";
        } else {
            std::cerr << "\nError returning book: " << sqlite3_errmsg(db) << "\n";
        }
    
        if (stmt) sqlite3_finalize(stmt);
    }


 


    void displaybooks(){
        const char* sql = "SELECT id, title, author FROM books;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {//SQLITE ROW = a row is available    //SQLITE_DONE = No more available row
                int id = sqlite3_column_int(stmt, 0);//read column 0
                const unsigned char* ti = sqlite3_column_text(stmt, 1);//read column 1 , Type is const unsigned char* (SQLite uses unsigned bytes)
                const unsigned char* au = sqlite3_column_text(stmt, 2);//read column 2
                std::cout << "Id: " << id
                          << " | Title: " << ti
                          << " | Author: " << au << "\n"; 
                        
            }
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void searchbook(std::string title){
        const char* sql = "SELECT id, title,author FROM books WHERE title LIKE ? COLLATE NOCASE;"; //COLLATE NOCASE makes the comparison case-insensitive and it works for ASCII letters
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string pattern = "%" + title + "%";
            sqlite3_bind_text(stmt, 1, pattern.c_str(),-1, SQLITE_TRANSIENT);
            while (sqlite3_step(stmt) == SQLITE_ROW) { 
                std::cout << "Id: " << sqlite3_column_int(stmt, 0)
                          << " | Title: " << sqlite3_column_text(stmt, 1)
                          << " | Author: " << sqlite3_column_text(stmt, 2) << "\n";
                         
            } 
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nBook not found.\n";
    
    }
     
};

int main(){
    Library lib;
    int choice = 0;
    int book_id;
    int user_id;
    std::string title, author,first_name,last_name,email,password;


    std::cout << "\n\nWelcome to our online library!\n";

    while(1){
        std::cout <<"Choose an option among these:\n";
        std::cout<< "\nBook related operations:\n";
        std::cout << "1.Add a book\n2.Search a book\n3.Issue a book\n4.Return a book\n5.Get active Issued books\n6.Display books\n";
        std::cout << "\nUser related operations:\n";
        std::cout << "7.Add user\n8.List all users\n9.Search for user by ID\n10.Search for user by email\n11.Exit\n";
        std::cout << "Please enter a choice (1-11): ";
        std::cin >> choice;
        std::cout << "\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if(choice==11 ){
            return 0;
        }

        switch(choice){
            case 1:
                std::cout << "Enter the title of the book: ";
                std::getline(std::cin,title);
                std::cout << "\nEnter the author: ";
                std::getline(std::cin,author);
                lib.addbook(title,author);
                break;

            case 2:
                std::cout <<"Enter the title of the book: ";
                std::getline(std::cin,title);
                lib.searchbook(title);
                break;

            case 3:
                std::cout << "Enter the id of the book you want to issue: ";
                std::cin >> book_id;
                std::cout << "\nEnter the borrower's ID: ";
                std::cin>> user_id;
                lib.issuebook(book_id,user_id);
                break;

            case 4:
                std::cout << "Enter the id of the book you want to return: ";
                std::cin >> book_id;;
                std::cout << "\nEnter the returner's ID: ";
                std::cin>> user_id;
                lib.returnbook(book_id,user_id);
                break;

            case 5:
                lib.getActiveIssuedBooks();
                break;
            
            case 6:
                lib.displaybooks();
                break;
            
            case 7:
                std::cout << "\nEnter the user's first name: ";
                std::getline(std::cin,first_name);
                std::cout << "\nEnter the user's last name: ";
                std::getline(std::cin,last_name);
                std::cout << "\nEnter the user's email: ";
                std::getline(std::cin,email);
                std::cout << "\nEnter the user's password: ";
                std::getline(std::cin,password);
                lib.addUser(first_name, last_name, email, password);
                break;

            case 8: 
                lib.listAllUsers();
                break;

            case 9:
                std::cout << "\nEnter the user's ID: ";
                std::cin >>user_id;
                std::cout << "\n";
                lib.getUserById(user_id);
                break;

            case 10:
                std::cout << "\nEnter the user's email: ";
                std::getline(std::cin,email);
                std::cout << "\n";
                lib.getUserByEmail(email);
                break;
        }
        std::cout <<"\n";  
    }
}
