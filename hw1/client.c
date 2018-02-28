#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

int main (int argc, char *argv[]) {
	int sd;
	struct sockaddr_in server;	
	int pkgs_sent = 0;
	long long bytes_sent = 0;

	char filename[100], msg[65537];
	strcpy(filename, "Ozark.S01E03.My.Dripping.Sleep.1080p.NF.WEBRip.DD5.1.x264-NTb.mkv");
	// strcpy(filename, "Licenta_Splited.pdf");
	int file_in = open(filename, O_RDONLY);

	int port = 2025;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons (port);

	int buffer_size = atoi(argv[3]);
	struct pollfd poll_fd;

	if(!strcmp(argv[1], "tcp")) {
		if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
			perror ("Eroare la socket().\n");
			return errno;
		}
		
		if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
			perror ("[client]Eroare la connect().\n");
			return errno;
		}
		poll_fd.fd = sd;
		poll_fd.events = POLLIN;
		while(1) {
			bzero (msg, buffer_size);
			if(read (file_in, msg, buffer_size) <= 0) {
				printf("Done reading\n");
				break;
			}

			if (write (sd, msg, buffer_size) <= 0) {
				perror ("[client]Eroare la write() spre server.\n");
				return errno;
			}
			bytes_sent += buffer_size;
			pkgs_sent++;

			if(!strcmp(argv[2], "wait")) {
				int poll_ret = poll(&poll_fd, 1, 1000);
				if(poll_ret == 0) {
					//timeout, resend
					printf("Lost\n");
					if (write (sd, msg, buffer_size) <= 0) {
			    		perror ("[client]Eroare la sendto() spre server2.\n");
			    		return errno;
					}
				} else {
					read(sd, msg,3);	
				}
				
			}
		}
	} else { //udp
		if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
	    	perror ("Eroare la socket().\n");
	    	return errno;
    	}
    	poll_fd.fd = sd;
		poll_fd.events = POLLIN;
    	int length = sizeof(server);
    	int msglen;
    	while(1) {
	    	bzero (msg, 65537);
			if(read (file_in, msg, buffer_size) <= 0) {
				printf("Done reading\n");
				break;
			}
			if (sendto (sd, msg, buffer_size,0, (struct sockaddr*)&server, length) <= 0) {
			    perror ("[client]Eroare la sendto() spre server.\n");
			    return errno;
			}
			bytes_sent += buffer_size;
			pkgs_sent++;
			if(!strcmp(argv[2], "wait")) {
				int poll_ret = poll(&poll_fd, 1, 1000);
				if(poll_ret == 0) {
					//timeout, resend
					printf("Lost\n");
					if (sendto (sd, msg, buffer_size,0, (struct sockaddr*)&server, length) <= 0) {
			    		perror ("[client]Eroare la sendto() spre server2.\n");
			    		return errno;
					}
				} else {
					recvfrom(sd, msg,3,0,(struct sockaddr*)&server, &length);
				}
			}
		}
		sendto (sd, "", 0,0, (struct sockaddr*)&server, length);
	} 
	printf("Bytes sent: %lld\n", bytes_sent);
	printf("Pkgs sent: %d\n", pkgs_sent);
	close (sd);
}
