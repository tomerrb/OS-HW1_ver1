#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

extern pid_t fg_pid;
extern char* fg_cmd_line;

void JobsList::ctrlZHandler(int sig_num) {

//    SmallShell& smash = SmallShell::getInstance();

    std::cout << "smash: got ctrl-Z" << endl;
    if(fg_cmd_line != nullptr){
        delete[] fg_cmd_line;
    }
    fg_cmd_line = new char[strlen(cmd_line)];
    strcpy(fg_cmd_line, cmd_line);

    JobsList::JobEntry(new_job_id, cmd->getCMDLine(), cmd->getPID(), time(NULL));

    fg_pid = 0;
}

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << endl;
    if(fg_pid != 0){
        kill(fg_pid, SIGKILL);
        std::cout << "smash: process: " << fg_pid << " was killed" << endl;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

