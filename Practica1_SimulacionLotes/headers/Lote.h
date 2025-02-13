#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
using namespace std;

struct Process {
    int id_program;
    string programmer;
    char operation;
    vector<int> data;
    int maxTime;
    int result;

    //Process();
    Process(int id_, string programmer_, char operation_, vector<int> data_, int maxTime_) : id_program(id_), programmer(programmer_), operation(operation_), data(data_), maxTime(maxTime_) {}

    void execute() {
        this_thread::sleep_for(chrono::seconds(maxTime));
        switch(operation) {
            case '+':
                result = data[0] + data[1];
                break;
            case '-':
                result = data[0] - data[1];
                break;
            case '*':
                result = data[0] * data[1];
                break;
            case '/':
                result = data[0] / data[1];
                break;
            case '%':
                result = data[0] % data[1];
                break;
            case '^':
                result = pow(data[0], data[1]);
                break;
        }
    }
};

class Lote {
    private:
        /* data */
    public:
        Lote(/* args */);
        ~Lote();
};