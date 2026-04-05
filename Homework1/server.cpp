#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>
#include <sys/wait.h>
#include <cmath>
#include <signal.h>

struct CharacterInfo {
	// The character to be decoded
	char ch;
	// The frequency of the character in the message
	int frequency;
	// The decoded message vector
	int start;
	// length of the binary string
	int binaryLength;
};

// Fireman function from Professor Rincon
// Used to handle zombie processes
void fireman(int) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char* argv[]) {
	// Server code from Professor Rincon
	// sockfd is the socket file descriptor
	// newsockfd is the socket file descriptor for specific client connections
	// portno is the port number
	// clilen is the length of the client address
	int sockfd = 0, newsockfd = 0, portno = 0, clilen = 0;
	// cliAddr is the client address info
	// servAddr is the server address info
	struct sockaddr_in servAddr, cliAddr;
	// n is used to store the number of bytes read or written
	int n = 0;

	// Creating a signal to execute the fireman function when a child process end its execution
	signal(SIGCHLD, fireman);

	// Check for the port number from the command line
	if (argc < 2) {
		std::cerr << "Port not provided" << std::endl;
		exit(0);
	}

	// Create a socket descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "Error opening socket" << std::endl;
		exit(0);
	}

	// Populate the sockaddr_in structure
	bzero((char*)&servAddr, sizeof(servAddr));
	portno = atoi(argv[1]);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(portno);

	// Bind the socket descriptor with the sockaddr_in structure
	if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
		std::cerr << "Error binding" << std::endl;
		exit(0);
	}

	// Set the max number of concurrent connections
	listen(sockfd, 20);
	clilen = sizeof(cliAddr);

	while (true) {
		// Accept a request from the client. A new socket descriptor is created to handle the request
		newsockfd = accept(sockfd, (struct sockaddr*)&cliAddr, (socklen_t*)&clilen);

		// Create a child process to answer the request.
		if (fork() == 0) {
			if (newsockfd < 0) {
				std::cerr << "Error on accept" << std::endl;
				exit(0);
			}

			// Create an Args structure to store the data sent by the client
			CharacterInfo characterInfo;

			// Read the data sent by the client and store it in the Args structure
			n = read(newsockfd, &characterInfo, sizeof(characterInfo));
			if (n < 0) {
				std::cerr << "Error reading from socket" << std::endl;
				exit(0);
			}

			// Create a temporary buffer to store the encoded message sent by the client
			char* tempBuffer = new char[characterInfo.binaryLength + 1];
			bzero(tempBuffer, characterInfo.binaryLength + 1);
			n = read(newsockfd, tempBuffer, characterInfo.binaryLength + 1);
			if (n < 0) {
				std::cerr << "Error reading from socket" << std::endl;
				exit(0);
			}

			// Convert the temporary buffer to a string and delete the temporary buffer
			std::string binary = tempBuffer;
			delete[] tempBuffer;

			// A vector containing all positions that is obtained from the binary string
			std::vector<int> allPositions;

			// Iterates through the binary string and extracts the positions in the decoded message
			int i = 0;
			while (i < binary.size()) {
				int n = 0;

				// Counts the number of leading zeros in the binary string
				while (i < binary.size() && binary.at(i) == '0') {
					n++;
					i++;
				}

				// Gets a substring of the string which length is equal to the number of leading zeros
				std::string temp = binary.substr(i + 1, n);
				// Converts the binary string to an integer value
				int value = 0;
				for (char c : temp)
					value = value * 2 + (c - '0');
				// Calculates the position and adds it to the allPositions vector
				allPositions.push_back((int)pow(2, n) + value);

				// Move the index to the next Elias-Gamma code
				i += 1 + n;
			}

			// A vector containing all positions that is obtained from the binary string
			std::vector<int> positions;
			// The number of bits required to represent the position(s) of the character
			int bits = 0;

			// Starts loop at the start variable and iterates until the end of the positions for the character
			for (int i = characterInfo.start; i < characterInfo.start + characterInfo.frequency; i++) {
				// Get the position from the allPositions vector and store it in the positions vector
				int pos = allPositions.at(i);
				positions.push_back(pos - 1);
				// Calculate the number of bits required to represent the position and add it to the bits variable
				bits += 2 * (int)log2(pos) + 1;
			}

			// Write the number of bits required to represent the position(s) of the character to the client
			n = write(newsockfd, &bits, sizeof(int));
			if (n < 0) {
				std::cerr << "Error writing to socket" << std::endl;
				exit(0);
			}

			// Write the number of positions to the client
			int nOfPositions = positions.size();

			n = write(newsockfd, &nOfPositions, sizeof(int));
			if (n < 0) {
				std::cerr << "Error writing to socket" << std::endl;
				exit(0);
			}

			// Write the positions to the client
			n = write(newsockfd, positions.data(), nOfPositions * sizeof(int));
			if (n < 0) {
				std::cerr << "Error writing to socket" << std::endl;
				exit(0);
			}

			// close the newsocket descriptor
			close(newsockfd);

			// Terminate the child process
			_exit(0);
		}
	}
	// close the socket descriptor
	close(sockfd);

	return 0;
}