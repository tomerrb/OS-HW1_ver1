#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <limits.h>


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

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
   else if (firstWord.compare("showpid") == 0) {
     return new ShowPidCommand(cmd_line);
   }
  else if (firstWord.compare("chprompt") == 0){
  return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0)
  {
    return new ChangeDirCommand(cmd_line, nullptr);
  }
  
  // else if ...
  // .....
  else {
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  Command* cmd = CreateCommand(cmd_line);
  try
  {
    cmd->execute();
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << '\n';
  }
  
  
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}


/**
 * commands types classes
 */
Command::Command(const char* cmd_line)
{}

BuiltInCommand::BuiltInCommand(const char* cmd_line)
{}

ExternalCommand::ExternalCommand(const char* cmd_line) : cmd_line(cmd_line)
{}
/**
 * actual commands classes
 * 
 */
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line)
{}
void GetCurrDirCommand::execute()
{
  char* buf = new char [PATH_MAX] ; 
  std::cout << getcwd(buf, PATH_MAX) << endl;
  delete[] buf;
}

ShowPidCommand::ShowPidCommand(const char* cmd_line){}
void ShowPidCommand::execute(){
    std::cout << "smash pid is " << getpid() << endl;
}

extern std::string small_shell_name;

ChangePromptCommand::ChangePromptCommand(const char* cmd_line):cmd_line(cmd_line){}
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

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd): cmd_line(cmd_line)
{}

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
  pid_t pid = fork();
  if (pid == 0)
  {
        char* temp = new char[strlen(this -> cmd_line)];
        strcpy(temp, this -> cmd_line);
        char* args[]= {"bash", "-c", temp, NULL};
//    args[0] = "-c";
//    args[1] = this->cmd_line;
    execv("/bin/bash", args);
  }
  else
  {
    wait(NULL);
  }
}