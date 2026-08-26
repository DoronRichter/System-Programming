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

int EXIT_STATUS = EXIT_SUCCESS
char *fqdn, *uaddr;
FILE *rx, *tx;


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

char *strmrg(char *username, char *ending){
	char*  address;
	size_t maxsize;
	
	maxsize = strlen(username) + strlen(ending) + 1;
	address = malloc(sizeof(*address) * maxsize);
	if(!buff)
		die("malloc");
	
	//username@cip.cs.fau.de
	strcpy(address, username);
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

/*
get status code from line in long format
for example:
100 welcome user -> 100
*/
long statcode(char *buf){
	//parse string to long
	errno = 0;
	char* endptr;
	long x = strtol(buf, &endptr, 10);
	if(errno == EINVAL)
		die("no conversion could be	performed");
	if(errno == ERANGE)
		die("The resulting value was out of range");

	//check whether no digits were found
	if(endptr == buf)
		return 0;
	
	return x; 
}

void write_terminal(char *msg){

}

int handle_line(char* buf, long cd){
	// read server response
	if(!fgets(buf, sizeof(buf), rx))
		die("fgets");
	
	size_t len = strlen(buf);
	/* in case line exceeds KiB */
	if(len > sizeof(buf) && buf[len-1] != '\n'){
		int c;
		//look for end of end of file notation
		while( (c = fgetc(rx)) != EOF){
			if (c == '\n') break;
		}
		if(ferror(rx)) 
			die("fgetcs");
	}
	// repalce end of line with null terminator
	// to not read any further
	if(buf[len-1] == '\n')
		buf[len-1] = '\0';
	
	if(ferror(rx)) die("ferror");
	
	
	//verify status code
	long stcd = statcode(buf);
	if(stcd != cd){
		write_terminal(buf);
		die("invalid status code");
	}
}

void handle_dialog(char* uaddr, char* argv[]){
	// sender name
	char *sn;
	// recepient address
    char *raddr = argv[2];
    // [Sender Name  <Subect>] 
    char *str = argv[1];
    
    // full name of user
    sn = strtok(str, "<");
    if(!sn)
    	die("strtok"); 
    	
    // subject
    char *subj = strtok(NULL, >);
    int is_there_subject = 1 ? subj : 0;
     
    
	//content of reponse
	char buf[KiB + 2];
	
	/** bearbeiten des Dialogs **/
	if ( handle_line(buf, (long) 220) ){
		EXIT_STATUS = EXIT_FAILURE;
		return;
	}
	fprintf(tx, "HELO %s\r\n", fdqn);
	if ( handle_line(buf, (long) 250) ){
		EXIT_STATUS = EXIT_FAILURE;
		return;
	}
	fprintf(tx, "MAIL FROM:<%s>\r\n", uaddr);
	if ( handle_line(buf, (long) 250) ){
		EXIT_STATUS = EXIT_FAILURE;
		return;
	}
	fprintf(tx, "RCPT TO:%s\r\n", raddr);
	if ( handle_line(buf, (long) 250) ){
		EXIT_STATUS = EXIT_FAILURE;
		return;
	}
	fprintf(tx, "DATA\r\n", uaddr);
	if ( handle_line(buf, (long) 354) ){
		EXIT_STATUS = EXIT_FAILURE;
		return;
	}
}

int main(int argc, char *argv[]){
	//snail [-s <subject>] <address>
    if(argc != 3)
    	die("argc");
    
    //get user name, kinda self explanatory
    char *usrn = get_username();
    //use usrn to create a sender address
    uaddr = strmrg(usrn, "@cip.cs.fau.de");
    if(!uaddr)
    	die("failed to read user email address");
    
    struct addrinfo hints = {
        .ai_flags = AI_CANONNAME, // for host name
        .ai_family = AF_INET,  // ipv4 only
        .ai_socktype = SOCK_STREAM,
    };
	
    struct addrinfo *server_info;
    //check for errors in addrinfo
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
	
	//determine host name
	struct addrinfo *host_info;
	char name[KiB + 1];
	if(!gethostname(name, KiB)
		die("gethostname");
	
	if ( (s = getaddrinfo(name, NULL, &hints, &host_info)) != 0 ){
	   fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
    }
    
	fqdn = strmrg( host_info->ai_canonname, ".informatik.uni-erlangen.de" );
	freeaddrinfo(host_info);
	if(!fdqn)
		die("strcat");
	
    // FILE * fuers Empfangen erstellen
	rx = fdopen(sock, "r");
	if(!rx)
		close(sock);
	// Duplikat des Socket-Deskriptors anlegen
	int sock_copy = dup(sock);
	if (sock_copy < 0)
		close(sock);
	// FILE * fuers Senden erstellen
	tx = fdopen(sock_copy, "w");
	if(!tx){
		if(fclose(rx)) perror("fclose: rx");
		close(sock);
		close(sock_copy);
		die("fdopen: rx");
	}
    
	//handle dialogue with server
	handle_dialog(uaddr, argv) 
	
	//close all files free all data
    if( fclose(rx) || fclose(tx) ) perror("fclose");
	close(sock); close(sock_copy);
	free(uaddr);
    freeaddrinfo(host_info);
    
	return EXIT_STATUS;
}
