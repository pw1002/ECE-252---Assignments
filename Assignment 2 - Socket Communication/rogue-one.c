#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT "2520"
#define PLANS_FILE "deathstarplans.dat"

typedef struct {
    char * data;
    int length;
} buffer;

extern int errno;

/* This function loads the file of the Death Star
   plans so that they can be transmitted to the
   awaiting Rebel Fleet. It takes no arguments, but
   returns a buffer structure with the data. It is the
   responsibility of the caller to deallocate the 
   data element inside that structure.
   */ 
buffer load_plans( );
int sendall(int socket, char* buf, int* len);

int main( int argc, char** argv ) {

    if ( argc != 2 ) {
        printf( "Usage: %s IP-Address\n", argv[0] );
        return -1;
    }
    printf("Planning to connect to %s.\n", argv[1]);
	struct stat st;
	stat( PLANS_FILE, &st );
	ssize_t filesize = st.st_size;
	char* plansdata = malloc(filesize);
    buffer buf = load_plans(plansdata);

    struct addrinfo hints; 
    struct addrinfo *res; 
	int sockfd;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	int result = getaddrinfo(argv[1], PORT, &hints, &res);
	if(result  != 0){
			printf("getaddrinfo fail");
			return -1;
	}
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	int status = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if(status  != 0){
			printf("connect failed");
			return -1;
	}
	int check = sendall(sockfd, buf.data, &buf.length);
	if(check !=0){
			printf("sendall error");
			return -1;
	}
	char *reply = malloc(64);
	int rec = recv(sockfd, reply, 64, 0);
	if(rec <= 0){
			printf("Error or socket was closed");
			return -1;
	}
	printf("%s", reply);

	free(plansdata);
	free(reply);
	freeaddrinfo(res);
	close(sockfd);

    return 0;
}

int sendall(int socket, char* buf, int* len){
		int total = 0;
		int bytesleft = *len;
		int n;

		while(total < *len){
				n = send(socket, buf + total, bytesleft, 0);
				if(n == -1){
						break;
				}
				total += n;
				bytesleft -= n;
		}
		*len = total;
		return n  == -1 ? -1 : 0;
}

buffer load_plans(char* plansdata) {
    struct stat st;
    stat( PLANS_FILE, &st );
    ssize_t filesize = st.st_size;
    //char* plansdata = malloc( filesize );
    int fd = open( PLANS_FILE, O_RDONLY );
    memset( plansdata, 0, filesize );
    read( fd, plansdata, filesize );
    close( fd );
    buffer buf;
    buf.data = plansdata;
    buf.length = filesize;

    return buf;
}
