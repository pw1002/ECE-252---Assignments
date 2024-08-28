#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 2520

volatile int quit = 0;
char* plansfile = "deathstarplans.dat";
char* plansbuf;
ssize_t filesize;

char* msg_ok = "Success - Death Star Plans Received!\n";
char* err_inc = "Error - Incomplete Data Received.\n";
char* err_long = "Error - More Data Than Expected Received.\n";
char* err_corr = "Error - Data corrupted.\n";


int main( int argc, char** argv ) {
    struct stat st;
    stat( plansfile, &st );
    filesize = st.st_size;
    plansbuf = malloc( filesize );
    memset( plansbuf, 0, filesize );
    int fd = open( plansfile , O_RDONLY );
    read( fd, plansbuf, filesize );

    /* Create Socket, IPv4 / Stream */
    int sockfd = socket( AF_INET, SOCK_STREAM, 0 ); 

    /* Create the sockaddr in on port */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( PORT ); 
    addr.sin_addr.s_addr = htonl( INADDR_ANY );

   
   
       
    free( plansbuf );
    close( fd );
    return 0;
}

