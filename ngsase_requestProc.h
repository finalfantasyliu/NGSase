#ifndef NGSASE_REQUESTPROC_H
#define NGSASE_REQUESTPROC_H

#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>
#include "ngsase_database.h"
#include "ngsase_json.h"
#include "http_request.h"
#include <sys/wait.h>


enum class Action {
    LOGIN,
    REGISTER,
    PIPELINE,
    PROJECT,
    NO_ACTION
};
enum class PipelineTask {
    PIPELINE_SYNC,
    PIPELINE_FILE_UPLOAD,
    PIPELINE_CHECK,
    PIPELINE_INSERT,
    ERROR
};

enum class ProjectTask {
    PROJECT_PREVIOUS,
    PROJECT_FILE_DOWNLOAD,
    PROJECT_FILE_UPLOAD,
    PROJECT_INSERT,
    PROJECT_EXECUTE,
    ERROR
};


class Task {
public:
    Task() {};

    Task(int kqfd, int filefd, HTTPRequest *request, Action actionType) : kqueuefd{kqfd},
                                                                          filefd{filefd},
                                                                          request{request},
                                                                          actionType{actionType} {};

    Task(int kqueuefd, int filefd, HTTPRequest *request, Action actionType, const std::string &executePath) : kqueuefd(
            kqueuefd), filefd(filefd), request(request), actionType(actionType), executePath(executePath) {};

    Task(int kqfd, int filefd, HTTPRequest *request) : kqueuefd{kqfd}, filefd{filefd},request{request} {};

public:
    int kqueuefd;
    int filefd;
    HTTPRequest *request = nullptr;
    Action actionType = Action::NO_ACTION;
    std::string executePath;

};

class ProcResult {
public:
    ContentType responseType;
    std::string responseBody;
    std::string fileName;
    int filefd=-1;
};


class LoginReqProc {
public:
    ProcResult taskProcess(Task *fetchTask, ngsase_database &ngsaseDB);

};

class RegisterReqProc {
public:
    ProcResult taskProcess(Task *fetchTask, ngsase_database &ngsaseDB);

};

class PipelineReqProc {
public:
    PipelineTask assignTask(std::string &URL);

    ProcResult taskProcess(Task *fetchTask, ngsase_database &ngsaseDB);
};

class ProjectReqProc {
public:
    ProjectTask assignTask(std::string &URL);

    ProcResult taskProcess(Task *fetchTask, ngsase_database &ngsaseDB);

    bool removeProjectRawData(std::filesystem::path inputDir);
};

#endif