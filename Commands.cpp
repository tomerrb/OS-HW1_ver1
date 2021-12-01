#include <unistd.h>
#include <string.h>
#include "string.h"
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
#include <string>


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

string prevPwd;

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

std::vector<string> _parseCommandLine(const string& cmd_line) {
    std::vector<string> result;
    std::istringstream iss(cmd_line);
    for(string s; iss >> s;){
        result.push_back(s);
    }
    return result;
}

bool _isBackgroundComamnd(string cmd_line) {
  return cmd_line[cmd_line.find_last_not_of(WHITESPACE)] == '&';
}

string _removeBackgroundSign(string cmd_line) {

  // find last character other than spaces
  unsigned int idx = cmd_line.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return cmd_line;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return cmd_line;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  string result = cmd_line.substr(0, idx);
  return result;
  // truncate the command line string up to the last non-space character
//  cmd_line[cmd_line.find_last_not_of(WHITESPACE, idx) + 1] = 0;
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
Command * SmallShell::CreateCommand(string cmd_line) {

  string cmd_s = _trim(cmd_line);
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

    found = cmd_s.find("|&");
    if(found != string::npos){
        cmd_to_execute_s = _trim(cmd_s.substr(0, found));
        string file_name = _trim(cmd_s.substr(found+2));
        file_int = open(file_name.c_str(), O_WRONLY|O_APPEND|O_CREAT, 0666);
    }else{
        found = cmd_s.find("|");
        if(found != string::npos){
            return new PipeCommand(1, cmd_to_execute_s,
                                   cmd_to_execute_s.substr(0, found),
                                   cmd_to_execute_s.substr(found+1));
        }
    }

   if (firstWord == "pwd") {
      return new GetCurrDirCommand(file_int, cmd_to_execute_s);
   }
   else if (firstWord == "showpid") {
     return new ShowPidCommand(file_int, cmd_to_execute_s);
   }
  else if (firstWord == "chprompt"){
  return new ChangePromptCommand(file_int, cmd_to_execute_s);
  }
  else if (firstWord == "cd")
  {
    return new ChangeDirCommand(file_int, cmd_to_execute_s);
  }
  else if (firstWord == "jobs")
  {
      return new JobsCommand(file_int, cmd_to_execute_s, &(this->jobs));
  }
  else if (firstWord == "kill")
  {
      return new KillCommand(file_int, cmd_to_execute_s, &(this->jobs));
  }
  else if (firstWord == "fg")
  {
      return new ForegroundCommand(file_int, cmd_to_execute_s, &(this->jobs));
  }
  else if (firstWord == "bg")
  {
      return new BackgroundCommand(file_int, cmd_to_execute_s, &(this->jobs));
  }
  else if (firstWord == "quit")
  {
      return new QuitCommand(file_int, cmd_to_execute_s, &(this->jobs));
  }
  else if (firstWord =="head")
  {
      return new HeadCommand(file_int, cmd_to_execute_s);
  }

  // else if ...
  // .....
  else {
      return new ExternalCommand(file_int, cmd_to_execute_s); //cmd_to_execute_s.c_str());
  }
  return nullptr;
}

PipeCommand::PipeCommand(int file_int, string cmd_line, string cmd_line_1, string cmd_line_2){
    this -> file_int = file_int;
    this -> cmd_line = cmd_line;
    this -> cmd_line_1 = cmd_line_1;
    this -> cmd_line_2 = cmd_line_2;
}

void PipeCommand::execute(){

    SmallShell& smash = SmallShell::getInstance();
    int fd[2];
    pipe(fd);
    if(fork() == 0){
        setpgrp();
        // Son
        close(fd[0]);
//        Command* cmd1 = smash.CreateCommand(this -> cmd_line_1);
//        cmd1 -> changeFileInt(fd[1]);
//        cmd1 -> execute();
////        close(fd[1]);
//        exit(0);
        dup2(fd[1], 1);
        smash.executeCommand(this -> cmd_line_1);
        exit(0);

    }else{
        close(fd[1]);
        //Father
//        Command* cmd2 = smash.CreateCommand(this -> cmd_line_2);
        int old_fd = dup(0);
        dup2(fd[0], 0);
//        cmd2 -> execute();
        smash.executeCommand(this -> cmd_line_2);
        dup2(old_fd, 0);
//        close(fd[0]);
    }
}

void SmallShell::executeCommand(string cmd_line) {

//    std::cout << "cmd_line before" << cmd_line << endl;
//    _removeBackgroundSign(cmd_line);
//    std::cout << "cmd_line after" << cmd_line << endl;

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

          exit(0);

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

  delete cmd;

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

QuitCommand::QuitCommand(int file_int, string cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void QuitCommand::execute(){
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if (cmd_args.size() > 1) {
        if(cmd_args[1] == "kill"){
            std::cout << "smash: sending SIGKILL to " << jobs -> getNumOfJobs()<< " jobs:" << endl;
            jobs -> killAllJobs();
        }
    }

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

JobsCommand::JobsCommand(int file_int, string cmd_line, JobsList* jobs): jobs(jobs){
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

KillCommand::KillCommand(int file_int, string cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void KillCommand::execute(){
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if (cmd_args.size() != 3) {
        throw std::invalid_argument("smash error: kill: invalid arguments");
    }

    int job_id_to_kill = stoi(cmd_args[2]);

    JobsList::JobEntry* je_ptr = this -> jobs -> getJobById(job_id_to_kill);

    if(je_ptr == NULL){
        string err_msg = "smash error: kill: job-id " + to_string(job_id_to_kill) + " does not exist";
        throw std::invalid_argument(err_msg);
    }

    string sig_str = cmd_args[1];
    sig_str = sig_str.substr(1);

    int signum = stoi(sig_str);
    pid_t pid_to_kill = je_ptr -> getProcessID();

    int result = kill(pid_to_kill, signum);

    if(result != 0){
        perror("smash error: kill failed");
    } else{
        string temp = "signal number " + to_string(signum) + " was sent to pid " + to_string(pid_to_kill) + "\n";
        write(this -> getFileInt(), temp.c_str(), strlen(temp.c_str()));
    }
}

bool isDigits(string str){
    for (char c : str) if (!isdigit(c)) return false;
    return true;
}

ForegroundCommand::ForegroundCommand(int file_int, string cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void ForegroundCommand::execute(){
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if (cmd_args.size() > 2) {
        throw std::invalid_argument("smash error: kill: invalid arguments");
    }

    int job_id_to_fg;
    JobsList::JobEntry* je_ptr;

    if(cmd_args.size() == 2){

        if (!(isDigits(cmd_args[1]))){
            string err_msg = "smash error: fg: invalid arguments";
            throw std::invalid_argument(err_msg);
        }

        job_id_to_fg = stoi(cmd_args[1]);

        je_ptr = this -> jobs -> getJobById(job_id_to_fg);

        if(je_ptr == NULL){
            string err_msg = "smash error: fg: job-id " + to_string(job_id_to_fg) + " does not exist";
            throw std::invalid_argument(err_msg);
        }
    } else {
        if(this -> jobs->isEmpty()){
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
        perror("smash error: kill failed");
    } else{
        std::cout << je_ptr -> getCMDLine() << " : " << pid_to_fg << endl;
    }

    int status;
    waitpid(pid_to_fg, &status, WUNTRACED);

}

BackgroundCommand::BackgroundCommand(int file_int, string cmd_line, JobsList* jobs): jobs(jobs){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

void BackgroundCommand::execute(){
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if (cmd_args.size() > 2) {

        throw std::invalid_argument("smash error: bg: invalid arguments");
    }

    int job_id_to_bg;
    JobsList::JobEntry* je_ptr;

    if(cmd_args.size() == 2){

        if (!(isDigits(cmd_args[1]))){

            string err_msg = "smash error: kill: invalid arguments";
            throw std::invalid_argument(err_msg);
        }

        job_id_to_bg = stoi(cmd_args[1]);

        je_ptr = this -> jobs -> getJobById(job_id_to_bg);

        if(je_ptr == NULL){

            string err_msg = "smash error: bg: job-id " + to_string(job_id_to_bg) + " does not exist";
            throw std::invalid_argument(err_msg);
        }else if(!(je_ptr->isStopped())){

            string err_msg = "smash error: bg: job-id " + to_string(job_id_to_bg) + " is already running in the background";
            throw std::invalid_argument(err_msg);
        }
    } else {
        if(this -> jobs->isEmpty()){

            string err_msg = "smash error: bg: jobs list is empty";
            throw std::invalid_argument(err_msg);
        }

        je_ptr = this -> jobs -> getLastStoppedJob(&job_id_to_bg);

        if(je_ptr == NULL){

            string err_msg = "smash error: bg: there is no stopped jobs to resume";
            throw std::invalid_argument(err_msg);
        }

    }

    pid_t pid_to_bg = je_ptr -> getProcessID();
    string cmd_line = je_ptr -> getCMDLine();

    std::cout << cmd_line << " : " << pid_to_bg << endl;
    int result = kill(pid_to_bg, SIGCONT);

    if(result != 0){

        perror("smash error: kill failed");
    }

    // Change stopped status in JobsList;
    je_ptr -> resume();

    waitid(P_PID, pid_to_bg, nullptr, WCONTINUED);

}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
    std::list<JobEntry>::reverse_iterator it;
    for(it = this -> jobs.rbegin(); it != this -> jobs.rend(); ++it){
        if(it->isStopped()) {
            return &*it;
        }
    }

    return nullptr;
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
    JobsList::JobEntry temp(jobId, "temp", 0, time(nullptr));
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
Command::Command(int file_int, string cmd_line){
    this -> cmd_line = cmd_line;
    this -> file_int = file_int;
}

BuiltInCommand::BuiltInCommand(int file_int, string cmd_line){
    this -> cmd_line = cmd_line;
    this -> is_external = false;
    this -> file_int = file_int;
}

ExternalCommand::ExternalCommand(int file_int, string cmd_line){
    this ->cmd_line = cmd_line;
    this -> is_external = true;
    this -> file_int = file_int;
}
/**
 * actual commands classes
 * 
 */
GetCurrDirCommand::GetCurrDirCommand(int file_int, string cmd_line){
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

ShowPidCommand::ShowPidCommand(int file_int, string cmd_line){
    this -> file_int = file_int;
}
void ShowPidCommand::execute(){
    string temp =  "smash pid is " + to_string(getpid()) + "\n";
    write(this -> file_int, temp.c_str(), strlen(temp.c_str()));
}

extern std::string small_shell_name;

ChangePromptCommand::ChangePromptCommand(int file_int, string cmd_line): cmd_line(cmd_line){
    this -> file_int = file_int;
}
void ChangePromptCommand::execute(){
//    char** args = (char**)malloc(COMMAND_MAX_ARGS*sizeof(char*));
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if(cmd_args[1] == "-"){
        small_shell_name = INIT_SMALL_SHELL_NAME;
    }else{
        small_shell_name = cmd_args[1];
    }
}

ChangeDirCommand::ChangeDirCommand(int file_int, string cmd_line): cmd_line(cmd_line)
{
    this->file_int = file_int;
}

void ChangeDirCommand::execute()
{

    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);
  if (cmd_args.size() > 2)
  {
    throw std::invalid_argument("smash error: cd: too many arguments");
  }

  if (cmd_args[1] == "-")
  {
    char* buf = new char [PATH_MAX] ;
    getcwd(buf, PATH_MAX);
    if(prevPwd != prevPwd) {
        prevPwd = "";
    }
    prevPwd = buf;
    string new_pwd = cmd_args[1];
    int ret = chdir(new_pwd.c_str());
    if (ret == -1)
    {
      perror("smash error: cd failed");
    }
  }
  else
  {
    if (!(prevPwd == ""))
    {
      throw std::invalid_argument("smash error: cd: OLDPWD not set");
    }
    int ret = chdir(prevPwd.c_str());
    prevPwd = "";
    if (ret == -1)
    {
      perror("smash error: cd failed");
    }
  }
}

void ExternalCommand::execute()
{
    string temp = this -> cmd_line;
    // This is the child process.
    if (_isBackgroundComamnd(this -> cmd_line)) {
        temp = _removeBackgroundSign(this -> cmd_line);
    }

    dup2(this -> file_int, 1);

    char* temp_str = new char[strlen(temp.c_str())];
    strcpy(temp_str, temp.c_str());
    char* args[]= {"bash", "-c", temp_str, NULL};
    execv("/bin/bash", args);
    delete[] temp_str;

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
        int result = waitpid(it->getProcessID(), &status, WNOHANG);
        if (result > 0){
            JobEntry temp = *it;
            ++it;
            this->jobs.remove(temp);
        }else{
            ++it;
        }
    }
}

HeadCommand::HeadCommand(int file_int, string cmd_line)
{
    this->file_int = file_int;
    this->cmd_line = cmd_line;
}

void HeadCommand::execute()
{
    std::vector<string> cmd_args = _parseCommandLine(this->cmd_line);

    if ((cmd_args.size() < 2) || (cmd_args[1][0] == '-' && cmd_args.size() == 2)) {
        throw std::invalid_argument("smash error: head: not enough arguments\n");
    }
    string file_to_read_s;
    if (cmd_args.size() == 3)
    {
        file_to_read_s = cmd_args[2];
    }
    else
    {
        file_to_read_s = cmd_args[1];
    }
    int rowsNum = 10;
    if (cmd_args.size() == 3)
    {
        string temp = cmd_args[1];
        if (temp.size() < 2 && temp[0] == '-')
        {
            return;
        }else{
            rowsNum = stoi(temp.substr(1));
        }

    }
    int open_res = open(file_to_read_s.c_str(), O_RDONLY);
    if (open_res == -1)
    {
        perror("smash error: open failed");
    }
    char* character = new char [1];
    int read_res = read(open_res,character, 1);
    int write_res = 0;
    int rows_count = 0;
    while (read_res != 0)
    {
        if (read_res == -1)
        {
            delete[] character;
            close(open_res);
            perror("smash error: read failed");
        }
        write_res = write (this->getFileInt(), character, 1);
        if  (write_res == -1)
        {
            delete[] character;
            close(open_res);
            perror("smash error: write failed");  
        }
        if (*character == '\n') rows_count++;
        if (rows_count == rowsNum) break;
        read_res = read(open_res,character, 1);
    }

    delete[] character;
    close(open_res);
    
}