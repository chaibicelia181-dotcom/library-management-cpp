#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>
#include "sqlite3.h"


class Library {
private:
    sqlite3* db;


    // Returns true if a row exists for the given query 
    bool rowExists(const char* sql, int id) {
        sqlite3_stmt* stmt = nullptr;
        bool exists = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            exists = (sqlite3_step(stmt) == SQLITE_ROW);
        }
        if (stmt) sqlite3_finalize(stmt);
        return exists;
    }

    bool userExists(int id) {
        return rowExists("SELECT id FROM users WHERE id=?;", id);
    }

    bool bookExists(int id) {
        return rowExists("SELECT id FROM books WHERE id=?;", id);
    }

    // BUG FIX: bookIsIssued was called in issuebook() but never defined
    bool bookIsIssued(int book_id) {
        const char* sql =
            "SELECT id FROM book_issues WHERE book_id=? AND return_date IS NULL;";
        return rowExists(sql, book_id);
    }

public:
   

    Library() : db(nullptr) {
        int rc = sqlite3_open("library.db", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
            sqlite3_close(db);
            db = nullptr;
            throw std::runtime_error("Database open failed");
        }

        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

        // Create tables if they don't exist yet
        const char* schema =
            "CREATE TABLE IF NOT EXISTS users ("
            "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  first_name TEXT NOT NULL,"
            "  last_name  TEXT NOT NULL,"
            "  email      TEXT NOT NULL UNIQUE,"
            "  password   TEXT NOT NULL"
            ");"
            "CREATE TABLE IF NOT EXISTS books ("
            "  id     INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  title  TEXT NOT NULL,"
            "  author TEXT NOT NULL"
            ");"
            "CREATE TABLE IF NOT EXISTS book_issues ("
            "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  user_id     INTEGER NOT NULL REFERENCES users(id),"
            "  book_id     INTEGER NOT NULL REFERENCES books(id),"
            "  issue_date  TEXT NOT NULL DEFAULT (date('now')),"
            "  return_date TEXT"          // NULL  = still issued
            ");";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, schema, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Schema error: " << errMsg << "\n";
            sqlite3_free(errMsg);
        }

        std::cout << "Database connected successfully.\n";
    }

    ~Library() {
        if (db) sqlite3_close(db);
    }

    //user operations

    void addUser(const std::string& first_name, const std::string& last_name,
                 const std::string& email,      const std::string& password) {
        const char* sql =
            "INSERT INTO users (first_name, last_name, email, password)"
            " VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, first_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, last_name.c_str(),  -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, email.c_str(),      -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, password.c_str(),   -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE)
                std::cout << "\nUser added successfully.\n";
            else
                std::cerr << "\nError inserting user: " << sqlite3_errmsg(db) << "\n";
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void listAllUsers() {
        const char* sql = "SELECT id, first_name, last_name, email FROM users;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                std::cout << "Id: "          << sqlite3_column_int (stmt, 0)
                          << " | First name: "<< sqlite3_column_text(stmt, 1)
                          << " | Last name: " << sqlite3_column_text(stmt, 2)
                          << " | Email: "     << sqlite3_column_text(stmt, 3) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nNo users found.\n";
    }

    void getUserById(int id) {
        const char* sql =
            "SELECT id, first_name, last_name, email FROM users WHERE id=?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW)
                std::cout << "Id: "          << sqlite3_column_int (stmt, 0)
                          << " | First name: "<< sqlite3_column_text(stmt, 1)
                          << " | Last name: " << sqlite3_column_text(stmt, 2)
                          << " | Email: "     << sqlite3_column_text(stmt, 3) << "\n";
            else
                std::cout << "\nUser not found.\n";
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void getUserByEmail(const std::string& email) {
        const char* sql =
            "SELECT id, first_name, last_name, email"
            " FROM users WHERE email LIKE ? COLLATE NOCASE;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string pattern = "%" + email + "%";
            sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                std::cout << "Id: "          << sqlite3_column_int (stmt, 0)
                          << " | First name: "<< sqlite3_column_text(stmt, 1)
                          << " | Last name: " << sqlite3_column_text(stmt, 2)
                          << " | Email: "     << sqlite3_column_text(stmt, 3) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nUser not found.\n";
    }

    //book operations

    void addBook(const std::string& title, const std::string& author) {
        const char* sql = "INSERT INTO books (title, author) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, title.c_str(),  -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE)
                std::cout << "\nBook added successfully.\n";
            else
                std::cerr << "\nError inserting book: " << sqlite3_errmsg(db) << "\n";
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void displayBooks() {
        const char* sql =
            "SELECT b.id, b.title, b.author,"
            "  CASE WHEN bi.id IS NOT NULL THEN 'Issued' ELSE 'Available' END AS status"
            " FROM books b"
            " LEFT JOIN book_issues bi"
            "   ON b.id = bi.book_id AND bi.return_date IS NULL;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                std::cout << "Id: "      << sqlite3_column_int (stmt, 0)
                          << " | Title: " << sqlite3_column_text(stmt, 1)
                          << " | Author: "<< sqlite3_column_text(stmt, 2)
                          << " | Status: "<< sqlite3_column_text(stmt, 3) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nNo books in the library.\n";
    }

    
    void searchBook(const std::string& title) {
        const char* sql =
            "SELECT id, title, author FROM books WHERE title LIKE ? COLLATE NOCASE;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;                  
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string pattern = "%" + title + "%";
            sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                std::cout << "Id: "      << sqlite3_column_int (stmt, 0)
                          << " | Title: " << sqlite3_column_text(stmt, 1)
                          << " | Author: "<< sqlite3_column_text(stmt, 2) << "\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nBook not found.\n";
    }

    //issuing/returning

    void issueBook(int book_id, int user_id) {
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

        const char* sql =
            "INSERT INTO book_issues (user_id, book_id) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_int(stmt, 2, book_id);
            if (sqlite3_step(stmt) == SQLITE_DONE)
                std::cout << "\nBook issued successfully.\n";
            else
                std::cerr << "\nError issuing book: " << sqlite3_errmsg(db) << "\n";
        }
        if (stmt) sqlite3_finalize(stmt);
    }

  
    void returnBook(int book_id, int user_id) {
        if (!userExists(user_id)) {
            std::cout << "\nUser ID " << user_id << " does not exist.\n";
            return;
        }
        if (!bookExists(book_id)) {
            std::cout << "\nBook ID " << book_id << " does not exist.\n";
            return;
        }

        // Verify this user actually borrowed the book and hasn't returned it yet
        const char* check_sql =
            "SELECT id FROM book_issues"
            " WHERE book_id=? AND user_id=? AND return_date IS NULL;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << sqlite3_errmsg(db) << "\n";
            return;
        }
        sqlite3_bind_int(stmt, 1, book_id);
        sqlite3_bind_int(stmt, 2, user_id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        stmt = nullptr;

        if (rc != SQLITE_ROW) {
            std::cout << "\nThis user does not have this book issued.\n";
            return;
        }

        // Mark the issue record as returned 
        const char* return_sql =
            "UPDATE book_issues SET return_date = date('now')"
            " WHERE book_id=? AND user_id=? AND return_date IS NULL;";
        if (sqlite3_prepare_v2(db, return_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, book_id);
            sqlite3_bind_int(stmt, 2, user_id);
            if (sqlite3_step(stmt) == SQLITE_DONE)
                std::cout << "\nBook returned successfully.\n";
            else
                std::cerr << "\nError returning book: " << sqlite3_errmsg(db) << "\n";
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    void getActiveIssuedBooks() {
        const char* sql =
            "SELECT u.first_name, u.last_name, u.email,"
            "       b.id, b.title, bi.issue_date"
            " FROM book_issues bi"
            " JOIN users u ON bi.user_id = u.id"
            " JOIN books b ON bi.book_id = b.id"
            " WHERE bi.return_date IS NULL;";
        sqlite3_stmt* stmt = nullptr;
        bool found = false;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                std::cout << "User: "       << sqlite3_column_text(stmt, 0)
                          << " "            << sqlite3_column_text(stmt, 1)
                          << " | Email: "   << sqlite3_column_text(stmt, 2)
                          << " | Book ID: " << sqlite3_column_int (stmt, 3)
                          << " | Title: "   << sqlite3_column_text(stmt, 4)
                          << " | Issued on: "<< sqlite3_column_text(stmt, 5)
                          << " | Status: Issued\n";
            }
        }
        if (stmt) sqlite3_finalize(stmt);
        if (!found) std::cout << "\nNo active issued books.\n";
    }
};

int main() {
    Library lib;
    int choice = 0;
    int book_id, user_id;
    std::string title, author, first_name, last_name, email, password;

    std::cout << "\n\nWelcome to our online library!\n";

    while (true) {
        std::cout << "\nBook related operations:\n"
                  << "  1. Add a book\n"
                  << "  2. Search a book\n"
                  << "  3. Issue a book\n"
                  << "  4. Return a book\n"
                  << "  5. Get active issued books\n"
                  << "  6. Display all books\n"
                  << "\nUser related operations:\n"
                  << "  7.  Add user\n"
                  << "  8.  List all users\n"
                  << "  9.  Search for user by ID\n"
                  << "  10. Search for user by email\n"
                  << "  11. Exit\n"
                  << "\nPlease enter a choice (1-11): ";

        std::cin >> choice;
        std::cout << "\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 11) return 0;

        switch (choice) {
            case 1:
                std::cout << "Enter the title of the book: ";
                std::getline(std::cin, title);
                std::cout << "Enter the author: ";
                std::getline(std::cin, author);
                lib.addBook(title, author);
                break;

            case 2:
                std::cout << "Enter the title of the book: ";
                std::getline(std::cin, title);
                lib.searchBook(title);
                break;

            case 3:
                std::cout << "Enter the book ID to issue: ";
                std::cin >> book_id;
                std::cout << "Enter the borrower's user ID: ";
                std::cin >> user_id;
                lib.issueBook(book_id, user_id);
                break;

            case 4:
                std::cout << "Enter the book ID to return: ";
                std::cin >> book_id;
                std::cout << "Enter the returner's user ID: ";
                std::cin >> user_id;
                lib.returnBook(book_id, user_id);
                break;

            case 5:
                lib.getActiveIssuedBooks();
                break;

            case 6:
                lib.displayBooks();
                break;

            case 7:
                std::cout << "Enter the user's first name: ";
                std::getline(std::cin, first_name);
                std::cout << "Enter the user's last name: ";
                std::getline(std::cin, last_name);
                std::cout << "Enter the user's email: ";
                std::getline(std::cin, email);
                std::cout << "Enter the user's password: ";
                std::getline(std::cin, password);
                lib.addUser(first_name, last_name, email, password);
                break;

            case 8:
                lib.listAllUsers();
                break;

            case 9:
                std::cout << "Enter the user's ID: ";
                std::cin >> user_id;
                lib.getUserById(user_id);
                break;

            case 10:
                std::cout << "Enter the user's email: ";
                std::getline(std::cin, email);
                lib.getUserByEmail(email);
                break;

            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 11.\n";
                break;
        }
        std::cout << "\n";
    }
}
