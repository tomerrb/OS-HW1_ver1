#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <limits.h>
#include <algorithm>
#include <stdio.h>
#include <fcntl.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif
#define WHITESPACE " "

char* prevPwd = nullptr; // new char*[PATH_MAX];

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

//SmallShell::SmallShell() {
//// TODO: add your implementation
//}
//
//SmallShell::~SmallShell() {
//// TODO: add your implementation
//}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  string cmd_to_execute_s = cmd_s;
  std::size_t found = cmd_s.find(">>");
  int file_int = 1; // std out
  if(found != string::npos){
      cmd_to_execute_s = _trim(cmd_s.substr(0, found));
      string file_name = _trim(cmd_s.substr(found+2));
      file_int = open(file_name.c_str(), O_WRONLY|O_APPEND|O_CREAT, 0666);
  }else{
      found = cmd_s.find(">");
      if(found != string::npos){
          cmd_to_execute_s = _trim(cmd_s.substr(0, found));
          string file_name = _trim(cmd_s.substr(found+1));
          file_int = open(file_name.c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0666);
        }
  }

   if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(file_int, cmd_line);
   }
   else if (firstWord.compare("showpid") == 0) {
     return new ShowPidCommand(file_int, cmd_line);
   }
  else if (firstWord.compare("chprompt") == 0){
  return new ChangePromptCommand(file_int, cmd_line);
  }
  else if (firstWord.compare("cd") == 0)
  {
    return new ChangeDirCommand(file_int, cmd_line, nullptr);
  }
  else if (firstWord.compare("jobs") == 0)
  {
      return new JobsCommand(file_int, cmd_line, &(this->jobs));
  }
  else if (firstWord.compare("kill") == 0)
  {
      return new KillCommand(file_int, cmd_line, &(this->jobs));
  }
  else if (firstWord.compare("fg") == 0)
  {
      return new ForegroundCommand(file_int, cmd_line, &(this->jobs));
  }
  else if (firstWord.compare("bg") == 0)
  {
      return new BackgroundCommand(file_int, cmd_line, &(this->jobs));
  }
  else if (firstWord.compare("quit") == 0)
  {
      return new QuitCommand(file_int, cmd_line, &(this->jobs));
  }

  // else if ...
  // .....
  else {
    return new ExternalCommand(file_int, cmd_line);
  }
  return nullptr;
}



void SmallShell::executeCommand(const char *cmd_line) {
//    write(1, "This Works", 10);
  // TODO: Add your implementation here
  // for example:
  this->jobs.removeFinishedJobs();
  Command* cmd = CreateCommand(cmd_line);
  if(cmd->isExternal()){
      pid_t pid = fork();
      if (pid == 0) {
          // This is the child.
          setpgrp();
          try {
              cmd->execute();
          }
          catch(const std::exception& e) {
              std::cout << e.what() << '\n';
          }
      }
      else
      {
          // This is the parent process.
          if(_isBackgroundComamnd(cmd_line)){
              // add to job list
              cmd -> changePID(pid);
              this -> jobs.addJob(cmd);
          }else{
              this->fgJobEntry = JobsList::JobEntry(0, cmd->getCMDLine(), pid, time(NULL));
              int status;
              waitpid(pid, &status, WUNTRACED);
          }
      }
  }else{
      try {
          cmd->execute();
      }
      catch(const std::exception& e) {
          std::cout << e.what() << '\n';
      }
  }

  if(cmd->getFileInt() != 1){
      close(cmd->getFileInt());
  }


  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

QuitCommand::QuitCommand(int file_int, const char* cmd_line, JobsList* jobs): jobs(jobs){
    this ->cmd_line=cmd_line;
    this -> file_int = file_int;
}

void QuitCommand::execute(){
    char** cmd_args = new char* [COMMAND_MAX_ARGS];
    int args_num = _parseCommandLine(this->cmd_line, cmd_args);
    if (args_num > 1) {
        if(strcmp(cmd_args[1],"kill") == 0){
            std::cout << "smash: sending SIGKILL to " << jobs -> getNumOfJobs()<< " jobs:" << endl;
            jobs -> killAllJobs();
        }
    }

    delete[] cmd_args;

    exit(0);
}

void JobsList::killAllJobs(){
    std::list<JobEntry>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ){
        JobEntry temp = *it;
        std::cout  << temp.getProcessID() << ": " << temp.getCMDLine() << endl;
        kill(temp.getProcessID(), SIGKILL);
        ++it;
        this->jobs.remove(temp);
    }
}

JobsCommand::JobsCommand(int file_int, const char* cmd_line, JobsList* jobs): jobs(jobs){
    this -> file_int = file_int;
}

void JobsCommand::execute(){
    jobs->printJobsList(this -> getFileInt());
}

JobsList::JobEntry* JobsList::getJobById(int jobId){
    JobsList::JobEntry temp(jobId, "temp", 0, time(NULL));
    std::list<JobsList::JobEntry>::iterator it;
    it = std::find(this -> jobs.begin(),this -> jobs.end(), temp);
    return &(*it);
}

KillCommand::KillCommand(int file_int, const char* cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void KillCommand::execute(){
    char** cmd_args = new char* [COMMAND_MAX_ARGS];
    int args_num = _parseCommandLine(this->cmd_line, cmd_args);
    if (args_num != 3) {
        delete[] cmd_args;
        throw std::invalid_argument("smash error: kill: invalid arguments");
    }

    int job_id_to_kill = atoi(cmd_args[2]);

    JobsList::JobEntry* je_ptr = this -> jobs -> getJobById(job_id_to_kill);

    if(je_ptr == NULL){
        delete[] cmd_args;
        string err_msg = "smash error: kill: job-id " + to_string(job_id_to_kill) + " does not exist";
        throw std::invalid_argument(err_msg);
    }

    string sig_str = string(cmd_args[1]);
    sig_str = sig_str.substr(1);

    int signum = stoi(sig_str);
    pid_t pid_to_kill = je_ptr -> getProcessID();

    int result = kill(pid_to_kill, signum);

    if(result != 0){
        delete[] cmd_args;
        perror("smash error: kill failed");
    } else{
       std::cout << "signal number " << signum << " was sent to pid " << pid_to_kill << endl;
    }

    delete[] cmd_args;
}

bool isDigits(const char* c_array){
    string temp_str(c_array);
    for (char c : temp_str) if (!isdigit(c)) return false;
    return true;
}

ForegroundCommand::ForegroundCommand(int file_int, const char* cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void ForegroundCommand::execute(){
    char** cmd_args = new char* [COMMAND_MAX_ARGS];
    int args_num = _parseCommandLine(this->cmd_line, cmd_args);
    if (args_num > 2) {
        delete[] cmd_args;
        throw std::invalid_argument("smash error: kill: invalid arguments");
    }

    int job_id_to_fg;
    JobsList::JobEntry* je_ptr;

    if(args_num == 2){

        if (!(isDigits(cmd_args[1]))){
            delete[] cmd_args;
            string err_msg = "smash error: fg: invalid arguments";
            throw std::invalid_argument(err_msg);
        }

        job_id_to_fg = atoi(cmd_args[1]);

        je_ptr = this -> jobs -> getJobById(job_id_to_fg);

        if(je_ptr == NULL){
            delete[] cmd_args;
            string err_msg = "smash error: fg: job-id " + to_string(job_id_to_fg) + " does not exist";
            throw std::invalid_argument(err_msg);
        }
    } else {
        if(this -> jobs->isEmpty()){
            delete[] cmd_args;
            string err_msg = "smash error: fg: jobs list is empty";
            throw std::invalid_argument(err_msg);
        }

        je_ptr = this -> jobs -> getLastJob(&job_id_to_fg);

    }

    SmallShell& smash = SmallShell::getInstance();
    smash.updateFGJobEntry(*je_ptr);
    pid_t pid_to_fg = je_ptr->getProcessID();

    this -> jobs -> removeJobById(job_id_to_fg);

    int result = kill(pid_to_fg, SIGCONT);

    if(result != 0){
        delete[] cmd_args;
        perror("smash error: kill failed");
    } else{
        std::cout << je_ptr -> getCMDLine() << " : " << pid_to_fg << endl;
    }
    delete[] cmd_args;

//    waitid(P_PID, pid_to_fg, NULL, WCONTINUED);
    int status;
    waitpid(pid_to_fg, &status, WUNTRACED);

}

BackgroundCommand::BackgroundCommand(int file_int, const char* cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void BackgroundCommand::execute(){
    char** cmd_args = new char* [COMMAND_MAX_ARGS];
    int args_num = _parseCommandLine(this->cmd_line, cmd_args);
    if (args_num > 2) {
        delete[] cmd_args;
        throw std::invalid_argument("smash error: bg: invalid arguments");
    }

    int job_id_to_bg;
    JobsList::JobEntry* je_ptr;

    if(args_num == 2){

        if (!(isDigits(cmd_args[1]))){
            delete[] cmd_args;
            string err_msg = "smash error: kill: invalid arguments";
            throw std::invalid_argument(err_msg);
        }

        job_id_to_bg = atoi(cmd_args[1]);

        je_ptr = this -> jobs -> getJobById(job_id_to_bg);

        if(je_ptr == NULL){
            delete[] cmd_args;
            string err_msg = "smash error: bg: job-id " + to_string(job_id_to_bg) + " does not exist";
            throw std::invalid_argument(err_msg);
        }else if(!(je_ptr->isStopped())){
            delete[] cmd_args;
            string err_msg = "smash error: bg: job-id " + to_string(job_id_to_bg) + " is already running in the background";
            throw std::invalid_argument(err_msg);
        }
    } else {
        if(this -> jobs->isEmpty()){
            delete[] cmd_args;
            string err_msg = "smash error: bg: jobs list is empty";
            throw std::invalid_argument(err_msg);
        }

        je_ptr = this -> jobs -> getLastStoppedJob(&job_id_to_bg);

        if(je_ptr == NULL){
            delete[] cmd_args;
            string err_msg = "smash error: bg: there is no stopped jobs to resume";
            throw std::invalid_argument(err_msg);
        }

    }

    pid_t pid_to_bg = je_ptr -> getProcessID();
    string cmd_line = je_ptr -> getCMDLine();

    std::cout << cmd_line << " : " << pid_to_bg << endl;
    int result = kill(pid_to_bg, SIGCONT);

    if(result != 0){
        delete[] cmd_args;
        perror("smash error: kill failed");
    }

    // Change stopped status in JobsList;
    je_ptr -> resume();

    delete[] cmd_args;
    waitid(P_PID, pid_to_bg, nullptr, WCONTINUED);

}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
    std::list<JobEntry>::reverse_iterator it;
    for(it = this -> jobs.rbegin(); it != this -> jobs.rend(); ++it){
        if(it->isStopped()) {
            return &*it;
        }
    }

    return NULL;
}

void JobsList::printJobsList(int file_int) {

    string temp = "";

    std::list<JobEntry>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it){
        temp = temp + "[" + to_string(it-> getJobID()) + "] " + it -> getCMDLine() + " : "
        +  to_string(it -> getProcessID()) + " " + to_string(int(difftime(time(NULL), it -> getBeginTime())))
        + " secs";
        if(it->isStopped()) {
            temp = temp + " (stopped)";
        }
        temp = temp + "\n";
    }

    write(file_int, temp.c_str(), strlen(temp.c_str()));

}

void JobsList::addJob(Command *cmd, bool isStopped) {
    int new_job_id;
    this->getLastJob(&new_job_id);
    new_job_id++;
    JobEntry je = JobEntry(new_job_id, cmd->getCMDLine(), cmd->getPID(), time(NULL));
    this -> jobs.push_back(je);//.insert(je);
    this -> jobs.sort();
}

void JobsList::addJobEntry(JobEntry je, bool isStopped) {
    if(isStopped){
        je.stop();
    }else{
        je.resume();
    }
    this -> jobs.push_back(je);//.insert(je);
    this -> jobs.sort();
}

void JobsList::removeJobById(int jobId){
    JobsList::JobEntry temp(jobId, "temp", 0, time(NULL));
    this->jobs.remove(temp);
}

JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
    if(this->jobs.empty()){
        *lastJobId = 0;
        return nullptr;
    }else {
        JobsList::JobEntry *result = &*jobs.rbegin();
        *lastJobId = result->getJobID();
        return result;
    }
}

bool JobsList::JobEntry::operator<(JobEntry const& je) const{
    return this->jobID < je.jobID;
}

bool JobsList::JobEntry::operator==(JobEntry const& je) const{
    return this->jobID == je.jobID;
}

/**
 * commands types classes
 */
Command::Command(int file_int, const char* cmd_line){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

BuiltInCommand::BuiltInCommand(int file_int, const char* cmd_line){
    this -> cmd_line = cmd_line;
    this -> is_external = false;
    this -> file_int = file_int;
}

ExternalCommand::ExternalCommand(int file_int, const char* cmd_line){
    this -> cmd_line = cmd_line;
    this -> is_external = true;
    this -> file_int = file_int;
}
/**
 * actual commands classes
 * 
 */
GetCurrDirCommand::GetCurrDirCommand(int file_int, const char* cmd_line){
    this -> file_int = file_int;
    this -> is_external = false;
}
void GetCurrDirCommand::execute()
{
  char* buf = new char [PATH_MAX] ;
  getcwd(buf, PATH_MAX);
  write(this -> file_int, buf, strlen(buf));
  write(this -> file_int, "\n", 1);
  delete[] buf;
}

ShowPidCommand::ShowPidCommand(int file_int, const char* cmd_line){
    this -> file_int = file_int;
}
void ShowPidCommand::execute(){
    string temp =  "smash pid is " + to_string(getpid()) + "\n";
    write(this -> file_int, temp.c_str(), strlen(temp.c_str()));
}

extern std::string small_shell_name;

ChangePromptCommand::ChangePromptCommand(int file_int, const char* cmd_line): cmd_line(cmd_line){
    this -> file_int = file_int;
}
void ChangePromptCommand::execute(){
//    char** args = (char**)malloc(COMMAND_MAX_ARGS*sizeof(char*));
    char** args = new char*[COMMAND_MAX_ARGS];
    int args_num = _parseCommandLine(cmd_line, args);
    if(strcmp(args[1], "-") == 0){
        small_shell_name = INIT_SMALL_SHELL_NAME;
    }else{
        small_shell_name = string(args[1]);
    }
    delete[] args;
}

ChangeDirCommand::ChangeDirCommand(int file_int, const char* cmd_line, char** plastPwd): cmd_line(cmd_line)
{
    this->file_int = file_int;
}

void ChangeDirCommand::execute()
{

  char** cmd_args = new char* [COMMAND_MAX_ARGS];
  int args_num = _parseCommandLine(this->cmd_line, cmd_args);
  if (args_num > 2)
  {
    delete[] cmd_args;
    throw std::invalid_argument("smash error: cd: too many arguments");
  }

  if (strcmp(cmd_args[1], "-") != 0)
  {
    char* buf = new char [PATH_MAX] ;
    getcwd(buf, PATH_MAX);
    if(prevPwd != nullptr) {
        delete[] prevPwd;
    }
    prevPwd = buf;
    char* new_pwd = cmd_args[1];
    int ret = chdir(new_pwd);
//    delete[] buf;
    if (ret == -1)
    {
      delete[] cmd_args;
      perror("smash error: cd failed");
    }
  }
  else
  {
    if (!prevPwd)
    {
      delete[] cmd_args;
      throw std::invalid_argument("smash error: cd: OLDPWD not set");
    }
    int ret = chdir(prevPwd);
    delete[] prevPwd;
    prevPwd = nullptr;
    if (ret == -1)
    {
      delete[] cmd_args;
      perror("smash error: cd failed");
    }
  }
  delete[] cmd_args;
}

void ExternalCommand::execute()
{
    // This is the child process.
    char *temp = new char[strlen(this->cmd_line)];
    strcpy(temp, this->cmd_line);
    if (_isBackgroundComamnd(temp)) {
        _removeBackgroundSign(temp);
    }

    dup2(this -> file_int, 1);

    char* args[]= {"bash", "-c", temp, NULL};
    execv("/bin/bash", args);
    exit(0);
}

BuiltInCommand::BuiltInCommand(){
    this -> is_external = false;
}

ExternalCommand::ExternalCommand(){
    this -> is_external = true;
}

JobsList::JobEntry::JobEntry(int jobID, string cmd_line, int processID, time_t begin_time):
jobID(jobID),cmd_line(cmd_line), processID(processID), begin_time(begin_time), stopped(false){}

void JobsList::removeFinishedJobs(){
    int status;
    std::list<JobEntry>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ){
        if (waitpid(it->getProcessID(), &status, WNOHANG) > 0){
            JobEntry temp = *it;
            ++it;
            this->jobs.remove(temp);
        }else{
            ++it;
        }
    }
}