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

void * loop(void * m) {
    pthread_detach(pthread_self()); // Thread should not be joined
    while (1) {

        string str = tcp.getMessage();

        tcp.clean(); // zeroes the "Message" variable and the receive buffer
        sleep(1);
    }
    tcp.detach();
}

int main(int argc, char** argv) {

    /* SQLite variables */
    sqlite3 *db;        // The database
    char *zErrMsg = 0;  // Holds the SQL database error message
    int rc;             // Holds return values
    char *sql;          // Holds the SQL commands to be executed
    const char* data = "Callback function called"; // Arg to callback function

    /* Open database */
    rc = sqlite3_open("test.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    } else {
        fprintf(stdout, "Opened database successfully\n\n\n");
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