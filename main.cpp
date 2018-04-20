/* 
 * File:   main.cpp
 * Author: anders
 *
 * Created on April 12, 2018, 12:14 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include "TCPServer.h"
#include "json.hpp"

TCPServer tcp;
using namespace std;

/* SQLite variables */
sqlite3 *db; // The database
char *zErrMsg = 0; // Holds the SQL database error message
int rc; // Holds return values
char *sql; // Holds the SQL commands to be executed
const char* data = "Callback function called"; // Arg to callback function

template <typename T> string tostr(const T& t) {
    ostringstream os;
    os << t;
    return os.str();
}

static int callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

void * loop(void * m) {


    pthread_detach(pthread_self()); // Thread should not be joined
    while (1) {

        string str = tcp.getMessage();

        if (str != "") {
            cout << ":::DEBUG:::Recieved this:" << str << endl;
            
            // deserialization (turn string back into json)
            auto j = nlohmann::json::parse(str);

            // Read out a value associated with a key in the json object
            int amount;
            if (j.find("amount") != j.end()) {
                cout << ":::DEBUG::: there is an entry with key \"amount\"" << endl;
                amount = j.at("amount");
            }

            if (j.find("command") != j.end()) {
                cout << ":::DEBUG::: there is an entry with key \"command\"" << endl;
                string command = j.at("command");

                if (command == "set") {

                    /* Create SQL statement */
                    string x = "INSERT INTO SENSOR (ID,AMOUNT) VALUES (1, " + tostr(amount) + ");";
                    char *y = new char[x.length() + 1];
                    strcpy(y, x.c_str());
                    sql = y;
                    delete[] y;

                    /* Execute SQL statement */
                    rc = sqlite3_exec(db, sql, callback, (void*) data, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    } else {
                        fprintf(stdout, "Records created successfully\n\n\n");
                    }
                } else if (command == "get") {

                    /* Create SQL statement */
                    sql = (char*)"SELECT * from SENSOR";

                    /* Execute SQL statement */
                    rc = sqlite3_exec(db, sql, callback, (void*) data, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    } else {
                        fprintf(stdout, "SELECT done successfully\n\n\n");
                    }
                }
            }
        }
        tcp.Send(":::DEBUG:::Server correctly recieved message");
        tcp.clean(); // zeroes the "Message" variable and the receive buffer
        sleep(1);
    }
    tcp.detach();
}

int main(int argc, char** argv) {

    /* Open database */
    rc = sqlite3_open("test.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    } else {
        fprintf(stdout, "Opened database successfully\n\n\n");
    }

    /* Create SQL statement */
    sql = (char*)"CREATE TABLE SENSOR("  \
         "ID INT PRIMARY KEY     NOT NULL," \
         "AMOUNT         INT   NOT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table created successfully\n\n\n");
    }

    pthread_t msg;
    tcp.setup(1955); // Socket, Bind, Listen

    while (1) {
        if (pthread_create(&msg, NULL, loop, (void *) 0) == 0) {
            tcp.receive(); // Starts the recieve task that updates "Message"
        } else {
            puts("Could not create thread: loop");
        }
    }
    /* We should never arrive here */
    puts("main exited with return code 1");
    return 1;
}