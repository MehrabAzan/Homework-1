#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

struct CharacterInfo {
    // The character to be decoded
    char ch;
    // The frequency of the character in the message
    int frequency;
    // The decoded message vector
    int start;
    // Length of the binary string
    int binaryLength;
};

struct Args {
    // Data from the CharacterInfo structure
    CharacterInfo characterInfo;
    // binary string containing the encoded message
    string binary;
    // hostname is the server address to connect to
    string hostname;
    // portno is the port number
    int portno;
    // A vector of positions
    vector<int> positions;
    // The amount of bits required to represent the position(s) of the character
    int bits;
};

void* Thread(void* voidPtr) {
    // Client code from Professor Rincon
    // void pointer being casted to the Args pointer
    Args* ptr = (Args*) voidPtr;

	// sockfd is the socket file descriptor
	// n is used to store the number of bytes read or written
	int sockfd = 0, n = 0;

    // servAddr is the server address info
	struct sockaddr_in servAddr;
    // server is to store the hostname information
	struct hostent* server;

    // Creating a socket descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        pthread_exit(NULL);
    }

    // Storing the hostname information
    server = gethostbyname(ptr->hostname.c_str());
    if (server == NULL) {
        cerr << "Error, no such host" << endl;
        pthread_exit(NULL);
    }

    // Populating the sockaddr_in structure
    bzero((char *) &servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&servAddr.sin_addr.s_addr,
         server->h_length);
    servAddr.sin_port = htons(ptr->portno);

    // Connecting to the server
    if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        cerr << "Error connecting";
        pthread_exit(NULL);
    }

    // Sending the CharacterInfo structure to the server
    n = write(sockfd, &ptr->characterInfo, sizeof(ptr->characterInfo));
    if (n < 0) {
        cerr << "Error writing to socket" << endl;
        pthread_exit(NULL);
    }

    // Sending the binary string to the server
    n = write(sockfd, ptr->binary.c_str(), ptr->characterInfo.binaryLength + 1);
    if (n < 0) {
        cerr << "Error writing to socket" << endl;
        pthread_exit(NULL);
    }

    // Reading the number of bits from the server
    n = read(sockfd, &ptr->bits, sizeof(ptr->bits));
    if (n < 0) {
        cerr << "Error reading from socket" << endl;
        pthread_exit(NULL);
    }

    // Reading the number of positions from the server and storing it in a variable
    int nOfPositions = 0;

    n = read(sockfd, &nOfPositions, sizeof(nOfPositions));
    if (n < 0) {
        cerr << "Error reading from socket" << endl;
        pthread_exit(NULL);
    }

    // Resizing the positions vector to the number of positions received from the server
    ptr->positions.resize(nOfPositions);
    // Reading the positions from the server and storing it in the positions vector
    n = read(sockfd, ptr->positions.data(), nOfPositions * sizeof(int));
    if (n < 0) {
        cerr << "Error reading from socket" << endl;
        pthread_exit(NULL);
    }
    
    // Close the socket descriptor
    close(sockfd);

    return nullptr;
}

int main(int argc, char* argv[]) {
    // Checking for the hostname and port number from the command line.
    if (argc < 3) {
        cerr << "usage " << argv[0] << "hostname port" << endl;
        exit(0);
    }

    // Create hostname to store the server address
    string hostname = argv[1];
    // Transforming the port number to int and storing it in a variable
    int portno = atoi(argv[2]);

    // m is the number of threads to be created
    int m = 0;
    // binary is the encoded message
    string binary = "";
    // vector of pairs to store the symbol and its frequency
    vector<pair<char, int>> symbols;
    // vector to store the decoded message
    vector<char> decodedMsg;

    // Reads user input
    cin >> m;

    // Ignore the newline character after reading m to ensure that getline reads the correct lines for symbols and frequencies
    cin.ignore();
    for (int i = 0; i < m; i++) {
        // Reads a line of input and extracts the symbol and its frequency
        string line;
        getline(cin, line);

        // The first character of the line is the symbol, and the rest of the line is the frequency
        char symbol = line.at(0);
        int frequency = stoi(line.substr(1));

        // Adds the symbol and its frequency to the symbols vector
        symbols.push_back({ symbol,frequency });
    }

    cin >> binary;

    // sorts the message according to lexicographical order (ascending based on ASCII value)
    sort(symbols.begin(), symbols.end(),
        [](const pair<char, int>& a, const pair<char, int>& b) {
            if (a.second != b.second)
                return a.second > b.second;
            return a.first < b.first;
    });

    // Calculate the total number of positions
    int totalPositions = 0;
    for (auto& symbol : symbols)
        totalPositions += symbol.second;
    // Resize the decoded message vector to the length of the total number of positions
    decodedMsg.resize(totalPositions);

    // create m threads
    vector<pthread_t> tid(m);
    // vector of Args structures to store the arguments for each thread
    vector<Args> result(m);

    // intialize the start variable in the main thread
    int start = 0;

    // For loop to create the threads
    for (int i = 0; i < m; i++) {
        // Assign the arguments for the thread function
        result.at(i).characterInfo.ch = symbols[i].first;
        result.at(i).characterInfo.frequency = symbols[i].second;
        result.at(i).characterInfo.start = start;
        result.at(i).characterInfo.binaryLength = binary.size();
        result.at(i).binary = binary;
        result.at(i).hostname = hostname;
        result.at(i).portno = portno;
        result.at(i).bits = 0;

        // Adds the frequency of the current symbol to the start variable to determine the starting index for the next symbol
        start += symbols[i].second;

        // Create the thread and pass the arguments to the thread function
        if (pthread_create(&tid.at(i), nullptr, Thread, (void*)&result.at(i)) != 0) {
            // If thread creation fails, print an error message and exit the program
            cerr << "Error creating the thread" << endl;
            exit(0);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < m; i++)
        pthread_join(tid.at(i), nullptr);

    for (int i = 0; i < m; i++) {
        // For each position of the character, assign the character to the correct position in the decoded message vector
        for (int pos : result.at(i).positions)
            decodedMsg.at(pos) = result.at(i).characterInfo.ch;
    }

    // Print the symbol, frequency, positions, and bits required to represent the positions for each character
    for (int i = 0; i < m; i++) {
        cout << "Symbol: " << result.at(i).characterInfo.ch << ", Frequency: " << result.at(i).characterInfo.frequency << endl;
        cout << "Positions: ";
        for (int pos : result.at(i).positions)
            cout << pos << " ";
        cout << endl;
        cout << "Bits to represent the position(s): " << result.at(i).bits << endl;
        cout << endl;
    }

    // Print the decoded message
    cout << "Decoded message: ";
    for (char c : decodedMsg)
        cout << c;
    cout << endl;

	return 0;
}