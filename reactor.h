//
// Created by 劉軒豪 on 2023/12/26.
//

#ifndef NGSASE_REACTOR_H
#define NGSASE_REACTOR_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include<sys/event.h>
#include <future>
#include <iostream>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <map>
#include <random>
#include "thread_pool.h"
#include "ngsase_requestProc.h"
#include "http_request.h"
#include "http_response.h"

enum class TriggerMode {
    EDGE = EV_CLEAR,
    EDGEONESHOT = EV_CLEAR | EV_ONESHOT,
    LEVEL = 0,
    LEVELONESHOT = LEVEL | EV_ONESHOT,
    NONE = 0
};
/* DatabaseInfo primarily stores information for the NGSase database,
 * especially to pass parameters into the ngsase_database constructor.
 * */
class DatabaseInfo {
public:
    DatabaseInfo() {};

    DatabaseInfo(std::string host, std::string username, std::string password, int port,
                 const char *unixSocket = nullptr,
                 CONNECT_SETTING setting = ROW_FOUND) : host{host},
                                                        username{username}, password{password}, unixSocket{unixSocket},
                                                        setting(setting) {};
public:
    void assign(std::string host, std::string username, std::string password, int port, const char *unixSocket,
                CONNECT_SETTING setting = ROW_FOUND);

    std::string host;
    std::string username;
    std::string password;
    int port;
    const char *unixSocket;
    CONNECT_SETTING setting;
};


class Reactor;

class Event;

//define function pointer for event callbackfunction
typedef void (*eventCallBackFunc)(void *fetchedEvent, void *reactor);

/* The Event class mainly stores information related to events, as shown below:
 * 1. Execution of event loop callback functions (read, write, accept)
 * 2. listenfd and kqfd
 * 3. Client address
 * */
class Event {
public:
    Event(int listenfd, int kqueuefd, std::string clientAddress, TriggerMode mode, int event, int registerAction,
          eventCallBackFunc readFunc,
          eventCallBackFunc writeFunc, eventCallBackFunc acceptFunc);

    Event(int listenfd, int kqueuefd, int event, int registerAction);

public:
    eventCallBackFunc readFunc = nullptr;
    eventCallBackFunc writeFunc = nullptr;
    eventCallBackFunc acceptFunc = nullptr;


public:
    int listenfd;
    int kqueuefd;
    std::string clientAddress;
    /* Here, it may be possible in the future to design a queue to write out data for some unfinished data that encountered problems
     * (however, the issue of data being incorrectly segmented and file-related issues need to be addressed),
     * because an event can only carry one udata,
     * it is not possible to re-register the same event with different udata on the same socket.*/

    // std::queue
    std::string backUpString;
    int triggerMode;
    int eventType;
    int monitorEvent;
    int action;


};


/* As the base class, Reactor has the following responsibilities:
 * 1. Establishing and initializing kqueue.
 * 2. Registering events.
 * 3. Cancelling events.
 * 4. Executing the event loop and dispatching tasks.
 * */
class Reactor {
public:
    Reactor() {};

    ~Reactor();

public:
    virtual bool reactorEventRegister(Event *event);

    virtual bool reactorEventCancel(Event *event);

    virtual void startEventMonitor(int second, int nanoSecond);

    virtual bool reactorInitialize(int serverfd, int maxEvent);

public:
    int kqueuefd = -1;
    int serverfd = -1;
    int maxEvent = -1;
    struct kevent *events = nullptr;
    std::mutex evetManagerLock;
    std::vector<Event *> eventManger;
};

template<class T>
class ReactorPanel {
public:
    ReactorPanel(int minThreadNum, int maxThreadNum, eventCallBackFunc readFunc, eventCallBackFunc writeFunc,
                 eventCallBackFunc acceptFunc, std::map<std::string, workerFunction> *workFuncMap);

    ReactorPanel();

public:
    bool assign(int minSiblingThreaddNum, int maxSiblingThreadNum, int minHeirThreadNum, int maxHeirThreadNum,
                eventCallBackFunc readFunc, eventCallBackFunc writeFunc,
                eventCallBackFunc acceptFunc, std::map<std::string, workerFunction> *workFuncMap);

    Reactor reactor;
    ThreadPool<T> threadPool;
    int minSiblingThreadNum = 1;
    int maxSiblingThreadNum = 3;
    int minHeirThreadNum = 1;
    int maxHeirThreadNum = 3;
    eventCallBackFunc readFunc = nullptr;
    eventCallBackFunc writeFunc = nullptr;
    eventCallBackFunc acceptFunc = nullptr;
    std::map<std::string, workerFunction> *workFuncMap= nullptr;
    DatabaseInfo databaseInfo;
};
// FilePathis used to store file abbreviations and paths.
class FilePath {
public:
    FilePath() {};

    ~FilePath() {
        /*    for (auto &it: filePathDescriptorMapper) {
                close(it.second);
            }*/
        abbreFilePathMapper.clear();
        filePathDescriptorMapper.clear();
    }

public:
    std::map<std::string, std::string> abbreFilePathMapper;
    std::map<std::string, int> filePathDescriptorMapper;

public:
    bool addFilePath(std::string abbreviation, std::string filePath);

    std::string getFilePath(std::string abbreviation);

    // int getFileDescriptor(std::string abbreviation);

    ContentType getFileContentType(std::string abbreviation);
};
class NGSaseParameter{
public:
    std::string userAddress;
    int userPort=-1;
    int userBacklog=-1;
    int userMainReactorMinWorkerNum=-1;
    int userMainReactorMaxWorkerNum=-1;
    int userSubReactorMinWorkerNum=-1;
    int userSubReactorMaxWorkerNum=-1;
    std::string userMySQLHost;
    std::string userMySQLAccount;
    std::string userMySQLPassword;
    bool assignUserInput(int argc,char *argv[]);
};



/* NGSaseServer serves as an entry point for constructing the NGSase server. */
class NGSaseServer {
public :
    NGSaseServer(std::string address, int port, int backlog,
                 int mainReactorMaxWorkerNum, int mainReactorMinWorkerNum, int subReactorMaxWorkerNum,
                 int subReactorMinWorkerNum, std::string mysqlHost, std::string mysqlUsername,
                 std::string mysqlpassword, int mysqlPort = 0, const char *unixSocket = nullptr);

    ~NGSaseServer();

public:
    void start();


public:
    int mainReactorMaxWorkerNum;
    int mainReactorMinWorkerNum;
    int subReactorMaxWorkerNum;
    int subReactorMinWorkerNum;
    ReactorPanel<Event *> *mainReactorPanel=nullptr;
    std::string serverAddress;
    DatabaseInfo ngsaseDBInfo;
    int port;
    int backlog;

    static std::string getFilePath(std::string fileName);
    static ContentType getFileContentType(std::string fileName);


private:
    int createServer();

    void funcAssign();

    bool filePathAssign();

    void checkFileExisted(const std::filesystem::path &directory);

    bool ngsaseDatabaseInitialize();

    std::map<std::string, workerFunction> *workFunctionMap= nullptr;
    eventCallBackFunc readFunc = nullptr;
    eventCallBackFunc writeFunc = nullptr;
    eventCallBackFunc acceptFunc = nullptr;
    static FilePath filePathCollection;
    static std::map<std::string, bool> fileNameCollection;


};



bool eventRegister(Event *registerEvent);

bool eventCancel(Event *previousEvent);

void eventLoop(Reactor *r, timespec *timeLimit);

bool setNonBlock(int fd);

bool setSocketNOSIGPIPE(int fd);

void mainReactorWorkFunc(void *threadPool, void *mainReactor);

void subReactorWorkFunc(void *threadPool, void *reactorPanel);

void acceptSocket(void *event, void *reactor);

void readSocket(void *event, void *reactor);

/* For writeSocket(), actual test scenarios need to be encountered before writing code.
 * It's necessary to address errors besides buffer fullness.
 * Additionally, in the future, it may be possible to add a queue to events to store information waiting to be written out.
 * However, the issue of udata needs to be considered*/
void writeSocket(void *event, void *reactor);

void writeHTTPResp(int fd, std::string &totalString, int filefd, int sendOption);

void writeHTTPResp(int fd, int filefd, int fileStartPos);


#endif //NGSASE_REACTOR_H
