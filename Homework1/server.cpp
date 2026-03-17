#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]) {
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in servAddr, cliAddr;
	
	if (argc < 2) {
		cerr << "Port not provided" << endl;
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error opening socket" << endl;
		exit(0);
	}

	memset(&servAddr, 0, sizeof(servAddr));
	portno = atoi(argv[1]);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
		cerr << "Error binding" << endl;
		exit(0);
	}

	listen(sockfd, 5);
	clilen = sizeof(cliAddr);

	newsockfd = accept(sockfd, (struct sockaddr*)&cliAddr, (socklen_t*)&clilen);
	if (newsockfd < 0) {
		cerr << "Error accepting new connections" << endl;
		exit(0);
	}
}