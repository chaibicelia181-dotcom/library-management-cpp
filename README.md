\# Library Management System



A console-based Library Management System written in C++, backed by a SQLite database.



* Features

\- Add and search books

\- Register users

\- Issue and return books with date tracking

\- Prevents issuing an already-borrowed book (enforced at the database level)

\- Automatically updates book status via SQL triggers



* &#x20;Project Structure

\- `library.cpp` — C++ source code (User, Book, BookIssue, Library classes)

\- `library.sql` — Full database schema including tables, triggers, and indexes

\- `sqlite3.h` — SQLite3 header file

\- `sqlite3.c` — SQLite3 source file

\- `libsqlite3.a` — SQLite3 static library

\- `sqlite3.dll` — SQLite3 dynamic library



* How to Run



&#x20; Requirements:



\- A C++ compiler (g++ recommended — via MSYS2/MinGW on Windows)



\### Compile



```

&#x20;  bash

g++ library.cpp libsqlite3.a -o a

```



\### Run





```bash

./a.exe

```

The program will automatically create a `library.db` file on first run.



* How to Use

Once running, you'll see a menu with 11 options:



Book operations:

1\. Add a book

2\. Search a book by title

3\. Issue a book to a user

4\. Return a book

5\. View all currently issued books

6\. Display all books



User operations:



7\. Add a user

8\. List all users

9\. Search for a user by ID

10.Search for a user by email

11.Exit



\## Database Design

\- `users`: stores registered library members

\- `books`: stores the book catalogue with availability status

\- `book\_issues`: tracks every borrow and return with timestamps



Two SQL triggers handle status updates automatically:

\- When a book is issued, its status flips to "issued"

\- When a book is returned, the return date is recorded automatically



A partial unique index ensures no book can be issued to two people at the same time.

