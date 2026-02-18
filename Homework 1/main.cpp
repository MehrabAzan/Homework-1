#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <pthread.h>

using namespace std;

// Arguments to be passed to the thread function
struct Args {
	// The character to be decoded
    char ch;
	// The number of bits required to represent the position(s) of the character
    int bits;
	// The positions of the character in the decoded message
    vector<int> positions;
	// The frequency of the character in the message
    int frequency;
	// A pointer to the decoded message vector
    vector<char>* decodedMsg;
	// A pointer to the vector containing all positions that is obtained from the binary string
    vector<int>* allPositions;
	// Used to determine the starting index in the allPositions vector and assign the correct positions to the character
    int start;
};

// The thread function
void* Thread(void* voidPtr) {
	// Cast the void pointer to the structure pointer
    Args* ptr = (Args*)voidPtr;

	// Initialize bits and clear positions vector
    ptr->bits = 0;
    ptr->positions.clear();

	// Starts loop at the start variable and iterates until the end of the positions for the character
    for (int i = ptr->start; i < ptr->start + ptr->frequency; i++) {
		// Get the position from the allPositions vector and store it in the positions vector
        int pos = ptr->allPositions->at(i);
        ptr->positions.push_back(pos - 1);
		// Calculate the number of bits required to represent the position and add it to the bits variable
        ptr->bits += 2 * (int)log2(pos) + 1;
		// Assign the character to the correct position in the decoded message vector
        ptr->decodedMsg->at(pos - 1) = ptr->ch;
    }

    return nullptr;
}

int main() {
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

    for (int i = 0; i < m; i++) {
        char symbol = ' ';
        int frequency = 0;
        cin >> symbol >> frequency;
        symbols.push_back({ symbol, frequency });
    }

    cin >> binary;

	// sorts the message according to lexicographical order (ascending based on ASCII value)
    sort(symbols.begin(), symbols.end(),
        [](const pair<char, int>& a, const pair<char, int>& b) {
            if (a.second != b.second)
                return a.second > b.second;
            return a.first < b.first;
        });

	// vector to store the postitions of each character
    vector<int> positions;

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
        string temp = binary.substr(i + 1, n);
		// Converts the binary string to an integer value
        int value = 0;
        for (char c : temp)
            value = value * 2 + (c - '0');
		// Calculates the position and adds it to the positions vector
        positions.push_back((int)pow(2, n) + value);

        // Move the index to the next Elias-Gamma code
        i += 1 + n;
    }

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
        result.at(i).ch = symbols[i].first;
        result.at(i).frequency = symbols[i].second;
        result.at(i).allPositions = &positions;
        result.at(i).start = start;
        result.at(i).decodedMsg = &decodedMsg;

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

	// Print the symbol, frequency, positions, and bits required to represent the positions for each character
    for (int i = 0; i < m; i++) {
        cout << "Symbol: " << result.at(i).ch << ", Frequency: " << result.at(i).frequency << endl;
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
}