#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	std::cout << "got to ctrl z" << endl;
}

void ctrlCHandler(int sig_num) {
    printf("Caught signal %d\n",sig_num);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

