#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>

using namespace std;

extern pid_t fg_pid;
extern char* fg_cmd_line;

void ctrlZHandler(int sig_num) {

    SmallShell& smash = SmallShell::getInstance();

    std::cout << "smash: got ctrl-Z" << endl;

    if(smash.getFGJobEntry().getJobID() != -1) {
        int current_job_id;
        if (smash.getFGJobEntry().getJobID() == 0) {
            current_job_id = smash.getLastJobID();
            current_job_id++;
        } else {
            current_job_id = smash.getFGJobEntry().getJobID();
        }
        JobsList::JobEntry je = JobsList::JobEntry(current_job_id, smash.getFGJobEntry().getCMDLine(),
                                                   smash.getFGJobEntry().getProcessID(), time(NULL));

        smash.addJobEntry(je, true);
        int kill_result = kill(smash.getFGJobEntry().getProcessID(), SIGSTOP);

        std::cout << "smash: process " << smash.getFGJobEntry().getProcessID() << " was stopped" << endl;

        smash.updateFGJobEntry(JobsList::JobEntry());
    }

}

void ctrlCHandler(int sig_num) {

    SmallShell& smash = SmallShell::getInstance();

    std::cout << "smash: got ctrl-C" << endl;
    if(smash.getFGJobEntry().getProcessID() != 0 || smash.getFGJobEntry().getJobID() != -1){
        kill(smash.getFGJobEntry().getProcessID(), SIGKILL);
        std::cout << "smash: process " << smash.getFGJobEntry().getProcessID() << " was killed" << endl;
    }
//    std::cout << "smash.getFGJobEntry().getProcessID() is: " << smash.getFGJobEntry().getProcessID() << endl;
}

void alarmHandler(int sig_num) {

    SmallShell& smash = SmallShell::getInstance();
    std::cout  <<  "smash: got an alarm" << endl;

    // Go to all timeout processes, if anyone has a duration that's too big, kill it.
    std::list<TimeOutList::TimeOutEntry>::iterator it;
    for (it = smash.timeouts.timeoutJobs.begin(); it != smash.timeouts.timeoutJobs.end(); ){
        TimeOutList::TimeOutEntry temp = *it;
//        std::cout << "is it timed out: " << temp.isTimedOut() << endl;
        if(temp.isTimedOut()){
            std::cout  <<  "smash: " << temp.getCMDLine() << " timed out!" << endl;
            kill(temp.getProcessID(), SIGKILL);
            // TODO: if i understand correctly, we do not need to wait here to the process.
            //  it will eneter zombie and somewhere else we wait for it.
//            kill(temp.getTimerProcessID(), SIGKILL);
//            int status;
//            waitpid(temp.getTimerProcessID(), &status, WUNTRACED);
        }
        ++it;
        if(temp.isTimedOut()) {
            smash.timeouts.timeoutJobs.remove(temp);
        }
    }

    std::list<pid_t>::iterator pid_it;
    for (pid_it = smash.timers_pids.begin(); pid_it != smash.timers_pids.end(); ){
        pid_t temp = *pid_it;
        int status;
        waitpid(temp, &status, WNOHANG);
        ++pid_it;
        smash.timers_pids.remove(temp);
    }
}

