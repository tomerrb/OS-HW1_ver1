#include <iostream>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include "stdio.h"

std::string small_shell_name = INIT_SMALL_SHELL_NAME;

void my_handler(sig_atomic_t s){
    printf("Caught signal %d\n",s);
}

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if(signal(SIGALRM , alarmHandler)==SIG_ERR) {
        perror("smash error: failed to set alrm handler");
    }

    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::cout << small_shell_name << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line);
    }
    return 0;
}