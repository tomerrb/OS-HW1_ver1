#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <string.h>
#include <time.h>
using std::string;
using std::list;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define INIT_SMALL_SHELL_NAME "smash"

class Command {
// TODO: Add your data members
protected:
    int file_int;
    pid_t pid;
    string cmd_line;
    bool is_external;
 public:
  Command() = default;
  Command(int file_int, string cmd_line);
  virtual ~Command() = default;
  virtual void execute() = 0;
  bool isExternal(){return is_external;};
  void changePID(pid_t pid){this -> pid = pid;};
  pid_t getPID(){return pid;};
  string getCMDLine(){return cmd_line;};
  int getFileInt(){return file_int;};
  void changeFileInt(int new_file_int){ this -> file_int = new_file_int;}
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 private:
  
 public:
  BuiltInCommand();
  BuiltInCommand(int file_int, string cmd_line);
  virtual ~BuiltInCommand() =default;
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(int file_int, string cmd_line);
  ExternalCommand();
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
private:
  string cmd_line_1;
  string cmd_line_2;
  int fd_redirect;
 public:
  PipeCommand(int file_int, string cmd_line, string cmd_line_1, string cmd_line_2, int fd_redirect);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(int file_int, string cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
  private:
  string cmd_line;
  public:
// TODO: Add your data members public:
  ChangeDirCommand(int file_int, string cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(int file_int, string cmd_line);
  virtual ~GetCurrDirCommand() = default;
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(int file_int, string cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
private:
    string cmd_line;
public:
    ChangePromptCommand(int file_int, string cmd_line);
    virtual ~ChangePromptCommand() {}
    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
private:
    JobsList* jobs;
public:
  QuitCommand(int file_int, string cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class TimeOutList {
public:
    class TimeOutEntry {
    private:
        pid_t processID;
        string cmd_line;
        pid_t timerProcessID;
        time_t timestamp;
        int duration;
    public:
        TimeOutEntry(int processID, string cmd_line, time_t timestamp, int duration, int timerProcessID);
        TimeOutEntry() = default;
        bool operator<(TimeOutEntry const& je) const;
        bool operator==(TimeOutEntry const& je) const;
        int getProcessID(){return processID;};
        int getTimerProcessID(){return timerProcessID;};
        int getDuration(){return duration;};
        time_t getTimestamp(){return timestamp;};
        bool isTimedOut(){
            time_t elapsed_time = difftime(time(nullptr), timestamp);
//            std::cout << "elapsed time is: " << elapsed_time << "\n";
            return elapsed_time >= duration;
        };
        string getCMDLine(){return cmd_line;};
        void changeProcessID(pid_t new_pid){this ->processID = new_pid;};
    };
private:
    list<TimeOutEntry> timeoutJobs;
public:
    TimeOutList() = default;
    ~TimeOutList() = default;
    void addTimeOutProcess(int processID, string cmd_line, time_t timestamp, int duration, int timerProcessID);
    void addTimeOutEntry(TimeOutEntry toe);
    void removeTimeOutEntry(pid_t pid_to_remove);
    friend void alarmHandler(int sig_num);
//    void removeFinishedJobs();
//    bool isEmpty(){return jobs.empty();};
//    int getNumOfJobs(){return jobs.size();};
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsList {
public:
  class JobEntry {
  private:
   int jobID;
   string cmd_line;
   pid_t processID;
   time_t begin_time;
   bool stopped;
  public:
   JobEntry(int jobID, string cmd_line, int processID, time_t begin_time);
   JobEntry() = default;
   bool operator<(JobEntry const& je) const;
   bool operator==(JobEntry const& je) const;
   int getJobID(){return jobID;};
   string getCMDLine(){return cmd_line;};
   int getProcessID(){return processID;};
   time_t getBeginTime(){return begin_time;};
   bool isStopped(){return stopped;};
   void resume(){this -> stopped = false;};
   void stop(){this -> stopped = true;};
  };
private:
    list<JobEntry> jobs;
//    int nextJobID;
public:
  JobsList() = default;
  ~JobsList() = default;
  void addJob(Command* cmd, bool isStopped = false);
  void addJobEntry(JobEntry je, bool isStopped);
  void printJobsList(int file_int);
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  bool isEmpty(){return jobs.empty();};
  int getNumOfJobs(){return jobs.size();};
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
private:
    JobsList* jobs;
 public:
  JobsCommand(int file_int, string cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
private:
    JobsList* jobs;
 public:
  KillCommand(int file_int, string cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 private:
    JobsList* jobs;
 public:
  ForegroundCommand(int file_int, string cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
private:
    JobsList* jobs;
 public:
  BackgroundCommand(int file_int, string cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(int file_int, string cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};

class TimeOutCommand : public BuiltInCommand {
private:
    int duration;
    string cmd_to_execute;
public:
    TimeOutCommand(int duration, string cmd_line);
    virtual ~TimeOutCommand() {}
    void execute() override;
};


class SmallShell {
 private:
  JobsList jobs;
  JobsList::JobEntry fgJobEntry;
  TimeOutList timeouts;

 public:
    SmallShell() = default;
  Command *CreateCommand(string cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell() = default;
  pid_t executeCommand(string cmd_line, bool is_timeout = false, TimeOutList::TimeOutEntry* toe_ptr = nullptr);
  void addJobEntry(JobsList::JobEntry je, bool isStopped){jobs.addJobEntry(je, isStopped);};
  int getLastJobID(){
      int new_job_id;
      jobs.getLastJob(&new_job_id);
      return new_job_id;
  }
  void updateFGJobEntry(JobsList::JobEntry je){fgJobEntry = je;};
  JobsList::JobEntry getFGJobEntry(){return fgJobEntry;};
  void addTimeOut(TimeOutList::TimeOutEntry toe){
      this->timeouts.addTimeOutProcess(toe.getProcessID(), toe.getCMDLine(),
                                       toe.getTimestamp(), toe.getDuration(), toe.getTimerProcessID());
  };
  void removeTimeOut(pid_t pid_to_remove){
      timeouts.removeTimeOutEntry(pid_to_remove);
  }

  friend void alarmHandler(int sig_num);

  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
