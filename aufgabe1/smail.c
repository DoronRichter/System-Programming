#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#define KiB 1024
#define SERVER "faui03.cs.fau.de"
#define PORT "25"

void die(char* death_message){
	perror(death_message);
	exit(EXIT_FAILURE);
}
char *get_username(){
	//apparently can never fail lol
	uid_t userid = getuid(); 
	
	//to ensure no errnos overlap
	errno = 0; 
	//get user details
	struct passwd *upw = getpwuid(userid); 
	//guess who failed after all
	if (errno != 0){
		die("getpwuid");
	
	//get user name
	return upw->pw_name;
}

char *get_sender_address(char *username){
	char*  address;
	size_t maxsize;
	
	maxsize = strlen("verylongusername") + strlen("@cip.cs.fau.de") + 1;
	address = malloc(sizeof(*address) * maxsize);
	if(!buff)
		die("malloc");
	
	//username@cip.cs.fau.de
	strcat(address, username);
	strcat(address, "@cip.cs.fau.de");
	
	return address;
}

int establish_connection(struct addrinfo** server_info){
    //initialize sock in case search fails
    //edgecase: info is NULL on first iteration
    int sock = -1;
    //iterate through all subdomains of server_info until valid connection is found
    for (struct addrinfo *info = *server_info; info != NULL; info = info->ai_next) {
        //use server info to establish a socket matching server criterias
        //can think about it like like using the right to connection
        //for your outlet
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if(sock == -1){
            perror("socket");
            continue;
        }
        //try to connect with server via socket
        if (connect(sock, info->ai_addr, info->ai_addrlen) != -1)
            break;
        close(sock);
    }
    
    return sock;
}

void handle_dialog(FILE** rx. FILE** tx, char* uaddr, char** argv[]){
	//recepient address
	char *raddr = *argv[2];
	for(;;){
		
	}
}

int main(int argc, char *argv[]){
	//snail [-s <subject>] <address>
    if(argc != 3)
    	die("argc");
    
    //get user name, kinda self explanatory
    char *usrn = get_username();
    //use usrn to create a sender address
    char *uaddr = get_sender_address(usrn);
    if(!uaddr)
    	die("failed to create email address");
    
	//set hints to match with server protocol
    struct addrinfo hints = {
        .ai_flags = AI_CANONNAME,
        .ai_family = AF_INET,  // ipv4 only
        .ai_socktype = SOCK_STREAM,
    };
	
    struct addrinfo *server_info;
    //to check for error for addrinfo
    int s;
    //get address infromation for the FAU server
    //save to server_info
    if ( (s = getaddrinfo(SERVER, PORT, &hints, &server_info)) != 0 ) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
    }
	//establish a socket of communication between client and server
	int sock = establish_connection(&server_info);
    if(sock == -1)
    	die("Failed to connect");
    freeaddrinfo(server_info);
    
    // FILE * fürs Empfangen erstellen
	FILE *rx = fdopen(sock, "r");
	if(!rx)
		die("fdopen: rx");
	// Duplikat des Socket-Deskriptors anlegen
	int sock_copy = dup(sock);
	if (sock_copy < 0)
		die("dup");
	// FILE * fuers Senden erstellen
	FILE *tx = fdopen(sock_copy, "w");
	if(!tx)
		die("fdopen: rx");
    
    handle_dialog(&rx, &tx, uaddr, &argv);
    
    free(uaddr);
    return EXIT_SUCCESS;
}
