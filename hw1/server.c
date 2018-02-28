#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 2025


int main (int argc, char**argv) {

	struct sockaddr_in server; 
	struct sockaddr_in from;  
	char msg[65537];    
	char msgrasp[100]= " ";
	int sd;     
	char filename[100] = "out_Ozark.S01E03.My.Dripping.Sleep.1080p.NF.WEBRip.DD5.1.x264-NTb.mkv";
	int pkgs_received = 0;
	long long bytes_received = 0;

	int file_out = open(filename, O_WRONLY | O_CREAT, "0777");

	/* pregatirea structurilor de date */
	bzero (&server, sizeof (server));
	bzero (&from, sizeof (from));
	

	server.sin_family = AF_INET;  
	server.sin_addr.s_addr = htonl (INADDR_ANY);
	server.sin_port = htons (PORT);


	if(!strcmp(argv[1], "tcp")) {
		/* crearea unui socket */
		if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
			perror ("[server]Eroare la socket().\n");
			return errno;
		}
		
		if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
			perror ("[server]Eroare la bind().\n");
			return errno;
		}

		if (listen (sd, 5) == -1) {
			perror ("[server]Eroare la listen().\n");
			return errno;
		}

		int client;
		int length = sizeof (from);

		printf ("[server]Asteptam la portul %d...\n",PORT);
		fflush (stdout);

		client = accept (sd, (struct sockaddr *) &from, &length);

		if (client < 0) {
			perror ("[server]Eroare la accept().\n");
			exit(0);
		}

		while (1) {

			bzero (msg, 65537);		
			int len_read = read(client,msg,65537);
			if(len_read == 0) {
				printf("Client disconnected\n");
				break;
			} else if (len_read < 0) {
				perror ("[server]Eroare la read() de la client.\n");
				close (client); 
				continue; 
			}
		
			write(file_out, msg, len_read);
			bytes_received += len_read;
			pkgs_received++;

			if(!strcmp(argv[2], "wait")) {
				write(client, "ack", 3);
			}
		}
	} else {
		if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
    		perror ("[server]Eroare la socket().\n"); 
      		return errno;
    	}
    	if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
	    	perror ("[server]Eroare la bind().\n");
	    	return errno;
    	}
    	int msglen;
    	struct sockaddr_in client_in;
      	int length = sizeof (client_in);
      	while(1) {
	      	bzero (msg, 100);
	      	msglen = recvfrom(sd, msg, 65535, 0,(struct sockaddr*) &client_in, &length);
	    	if (msglen == 0) {
		 		printf("And we're done\n");
		 		break;
			}
			write(file_out, msg, msglen);
			bytes_received += msglen;
			pkgs_received++;
			if(!strcmp(argv[2], "wait")) {
				sendto(sd, "ack", 3, 0, (struct sockaddr*) &client_in, length);
			}
		}
	}
	printf("Bytes received: %lld\n", bytes_received);
	printf("Pkgs received: %d\n", pkgs_received);
}
