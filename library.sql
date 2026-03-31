
PRAGMA foreign_keys = ON;


CREATE TABLE "users" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "email" TEXT UNIQUE NOT NULL,
    "password" TEXT NOT NULL
);

CREATE TABLE "books" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "title" TEXT NOT NULL,
    "author" TEXT NOT NULL,
    "is_issued" INTEGER NOT NULL DEFAULT 0 CHECK ("is_issued" IN (0,1))
);

CREATE TABLE book_issues (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    book_id INTEGER NOT NULL,
    issue_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    return_date DATETIME DEFAULT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (book_id) REFERENCES books(id)
);


CREATE UNIQUE INDEX one_active_issue_per_book
ON book_issues(book_id)
WHERE return_date IS NULL;

CREATE TRIGGER "update_issues"
AFTER INSERT ON "book_issues"
FOR EACH ROW
BEGIN
UPDATE "books"
SET "is_issued" = 1
WHERE "id" = NEW."book_id";
END;



CREATE TRIGGER "update_return_date" 
AFTER UPDATE OF "is_issued" ON "books"
FOR EACH ROW
WHEN OLD."is_issued" = 1 AND NEW."is_issued" =  0
BEGIN 
UPDATE "book_issues" 
SET "return_date" = CURRENT_TIMESTAMP
WHERE "book_id" = NEW."id"
AND "return_date" IS NULL;
END;



