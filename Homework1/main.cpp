#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <pthread.h>

// Arguments to be passed to the thread function
struct Args {
	// The character to be decoded
    char ch;
	// The number of bits required to represent the position(s) of the character
    int bits;
    // The encoded message
    std::string binary;
	// The frequency of the character in the message
    int frequency;
	// A pointer to the decoded message vector
    std::vector<char>* decodedMsg;
	// A pointer to the vector containing all positions that is obtained from the binary string
    std::vector<int>* allPositions;
	// Used to determine the starting index in the allPositions vector and assign the correct positions to the character
    int start;
    // The mutex to protect the shared resources
    pthread_mutex_t mutex;
    // The condition variable to signal the threads when the shared resources are updated
    pthread_cond_t condition;
    // Used to determine the index of the thread and the order of printing the results
    int index;
    // Used to determine the next thread to print its results
    int nextToPrint;
    // Used to determine if the thread has copied the arguments from the main thread
    bool copied;
};

// The thread function
void* Thread(void* voidPtr) {
	// Cast the void pointer to the structure pointer
    Args* ptr = (Args*) voidPtr;

    // Lock the mutex
    pthread_mutex_lock(&ptr->mutex);

    // Copy the arguments from the main thread to local variables
    char ch = ptr->ch;
    int frequency = ptr->frequency;
    int start = ptr->start;
    int index = ptr->index;
    std::string binary = ptr->binary;
    std::vector<char>* decodedMsg = ptr->decodedMsg;

    // Set copied to true to indicate that the thread has copied the arguments from the main thread
    ptr->copied = true;

    // Wake up any thread waiting on the condition variable
    pthread_cond_broadcast(&ptr->condition);

    // Unlock the mutex
    pthread_mutex_unlock(&ptr->mutex);

    // A vector containing all positions that is obtained from the binary string
    std::vector<int> allPositions;

    // Iterates through the binary string and extracts the positions in the decoded message
    int i = 0;
    while (i < binary.length()) {
        int n = 0;

		// Counts the number of leading zeros in the binary string
        while (i < binary.length() && binary.at(i) == '0') {
            n++;
            i++;
        }

		// gets a substring of the string which length is equal to the number of leading zeros
        std::string temp = binary.substr(i + 1, n);
		// Converts the binary string to an integer value
        int value = 0;
        for (char c : temp)
            value = value * 2 + (c - '0');
		// Calculates the position and adds it to the positions vector
        allPositions.push_back((int)std::pow(2,n) + value);

        // Move the index to the next Elias-Gamma code
        i += 1 + n;
    }

	// Initialize bits and positions vector
    int bits = 0;
    std::vector<int> positions;

	// Starts loop at the start variable and iterates until the end of the positions for the character
    for (int i = start; i < start + frequency; i++) {
		// Get the position from the allPositions vector and store it in the positions vector
        int pos = allPositions.at(i);
        positions.push_back(pos - 1);
		// Calculate the number of bits required to represent the position and add it to the bits variable
        bits += 2 * (int)log2(pos) + 1;
    }

    // Lock the mutex
    pthread_mutex_lock(&ptr->mutex);

    // Assign the character to the correct positions in the decoded message vector
    for (int pos : positions)
        ptr->decodedMsg->at(pos) = ch;

    // Unlock the mutex
    pthread_mutex_unlock(&ptr->mutex);

    // Lock the mutex to protect nextToPrint and guarantee the order of printing the results
    pthread_mutex_lock(&ptr->mutex);

    // Wait until it is the thread's turn to print its results
    while (ptr->nextToPrint != index)
        pthread_cond_wait(&ptr->condition, &ptr->mutex);
    
    // Print the symbol, frequency, positions, and bits required to represent the positions for each character
    std::cout << "Symbol: " << ch << ", Frequency: " << frequency << std::endl;
    std::cout << "Positions: ";
    for (int pos : positions)
        std::cout << pos << " ";
    std::cout << std::endl;
    std::cout << "Bits to represent the position(s): " << bits << std::endl;
    std::cout << std::endl;

    // Allow the next thread to print its results by incrementing nextToPrint
    ptr->nextToPrint++;

    // Broadcast to the waiting threads so that the next thread can print its results
    pthread_cond_broadcast(&ptr->condition);

    // Unlock the mutex
    pthread_mutex_unlock(&ptr->mutex);

    return nullptr;
}

int main() {
    // m is the number of threads to be created
    int m = 0;
    // binary is the encoded message
    std::string binary = "";
	// vector of pairs to store the symbol and its frequency
    std::vector<std::pair<char, int>> symbols;
	// vector to store the decoded message
    std::vector<char> decodedMsg;

    // Reads user input
    std::cin >> m;

	// Ignore the newline character after reading m to ensure that getline reads the correct lines for symbols and frequencies
	std::cin.ignore();

    for (int i = 0; i < m; i++) {
		// Reads a line of input and extracts the symbol and its frequency
		std::string line;
		std::getline(std::cin, line);

		// The first character of the line is the symbol, and the rest of the line is the frequency
		char symbol = line.at(0);
		int frequency = std::stoi(line.substr(1));

		// Adds the symbol and its frequency to the symbols vector
        symbols.push_back({symbol, frequency});
	}

    std::cin >> binary;

	// Sorts the message according to lexicographical order (ascending based on ASCII value)
    std::sort(symbols.begin(), symbols.end(),
        [](const std::pair<char, int>& a,const std::pair<char, int>& b) {
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

    // Create m threads
    std::vector<pthread_t> tid(m);
	// Args structures to store the arguments for each thread
    Args result;
    // Store values that can be used by all threads
    result.binary = binary;
    result.decodedMsg = &decodedMsg;
    result.copied = false;
    result.nextToPrint = 0;
    
    // Initialize the mutex and condition variable
    pthread_mutex_init(&result.mutex, nullptr);
    pthread_cond_init(&result.condition, nullptr);

	// Initialize the start variable in the main thread
    int start = 0;

    // For loop to create the threads
    for (int i = 0; i < m; i++) {
        // Lock the mutex before writing the to the shared argument structure
        pthread_mutex_lock(&result.mutex);

		// Assign the arguments for the thread function
        result.ch = symbols[i].first;
        result.frequency = symbols[i].second;
        result.start = start;
        result.index = i;
        result.copied = false;

		// Create the thread and pass the arguments to the thread function
        if (pthread_create(&tid.at(i), nullptr, Thread, (void*)&result) != 0) {
			// If thread creation fails, print an error message and exit the program
            std::cerr << "Error creating the thread" << std::endl;
            exit(0);
        }

        // Wait until the child thread copied the arguments
        while (!result.copied)
            pthread_cond_wait(&result.condition, &result.mutex);

        // Unlock the mutex
        pthread_mutex_unlock(&result.mutex);

        // Update the start variable for the next thread
        start += symbols[i].second;
    }

	// Wait for all threads to finish
    for (int i = 0; i < m; i++)
        pthread_join(tid.at(i), nullptr);

	// Print the decoded message
    std::cout << "Decoded message: ";
    for (char c : decodedMsg)
        std::cout << c;
    std::cout << std::endl;

    return 0;
}