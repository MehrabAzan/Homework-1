#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <pthread.h>
using namespace std;

struct Arguments {
    string* binary;
    char ch;
    int frequency;
};

void* Thread(void* voidPtr) {
    Arguments* ptr = (Arguments*) voidPtr;
    int totalN;
    vector<int> positions;
    int bits;

    for (int i = 0; i < ptr->frequency; i++) {
        int n;

        for (int j = 0; j < ptr->binary->length(); j++) {
            n++;

            if (ptr->binary->at(j) == '1') {
                string temp = ptr->binary->substr(j, n);
                int value;
                for (char c : temp)
                    value = value * 2 + (c - '0');

                positions.push_back(pow(2, n) + value);

                j += n - 1;
                bits += 2 * n + 1;
                n = 0;
            }
        }
    }

    cout << "Symbol: " << ptr->ch << ", Frequency: " << ptr->frequency << endl;
    cout << "Positions: ";
    for (int pos : positions)
        cout << pos - 1 << " ";
    cout << endl;
    cout << "Bits to represent the position(s): " << bits << endl;

    return nullptr;
}

int main() {
    int m;
    string originalBinary;
    char symbol;
    int originalFrequency;
    vector<pair<char, int>> symbols;

    cin >> m;

    for (int i = 0; i < m; i++) {
        cin >> symbol >> originalFrequency;
        symbols.push_back({originalFrequency, symbol});
    }

    cin >> originalBinary;

    sort(symbols.begin(), symbols.end());

    vector<pthread_t> tid(m);
    vector<Arguments> result(m);

    for (int i = 0; i < m; i++) {
        result.at(i).binary = &originalBinary;
        result.at(i).ch = symbols[i].second;
        result.at(i).frequency = symbols[i].first;
        if (pthread_create(&tid.at(i), nullptr, Thread, (void*)&result.at(i)) != 0) {
            cerr << "Error creating the thread" << endl;
            exit(0);
        }
    }
    for (int i = 0; i < m; i++) {
        pthread_join(tid.at(i), nullptr);
    }
}