#include "reactor.h"
//Please remember One Definition Rule(ODR)
/* static member variable must be defined exactly once in the entire program.
 * If it's defined in multiple translation units (source files),
 * it violates the ODR and leads to linker errors.
 *
 * ****if you forgot this rule, would get compile error****
 * Declaring static member variables in the header file (*.h) directly can lead to multiple definitions of these variables if the header file is included in multiple translation units (source files).
 * This happens because each source file that includes the header file will have its own copy of the static member variables, resulting in duplicate symbol linker errors during the linking phase.
 *
 * ****best practice********
 * 1 .h file only have declaration and use extern keyword to tell compiler that definition is in somewhere else.
 * 2 Definition implement in .cpp file.
 * 3 for class static member, you don't need to use extern keyword, just make sure you define in .cpp
 *
 * */

//review again declaration and definition
/* Declaration: In the header file, you declare the static member variable using the static keyword.
 * This informs the compiler about the existence of the variable.
 *
 *Definition: In the source file, you define (allocate memory for) the static member variable.
 * This ensures that there is only one instance of the variable across the entire program.
 * */
std::map<std::string, bool> NGSaseServer::fileNameCollection;
FilePath NGSaseServer::filePathCollection;

void
DatabaseInfo::assign(std::string host, std::string username, std::string password, int port, const char *unixSocket,
                     CONNECT_SETTING setting) {
    this->host = host;
    this->username = username;
    this->password = password;
    this->port = port;
    this->unixSocket = unixSocket;
    this->setting = setting;
}


Event::Event(int listenfd, int kqueuefd, std::string clientAddress, TriggerMode mode, int event, int registerAction,
             eventCallBackFunc readFunc,
             eventCallBackFunc writeFunc, eventCallBackFunc acceptFunc) {

    this->eventType = event;
    this->action = (int) mode | registerAction;
    this->listenfd = listenfd;
    this->kqueuefd = kqueuefd;
    this->readFunc = readFunc;
    this->writeFunc = writeFunc;
    this->acceptFunc = acceptFunc;
    this->clientAddress = clientAddress;

}

//如果單純想要刪除可以使用這個constructor
Event::Event(int listenfd, int kqueuefd, int event, int registerAction) {
    this->listenfd = listenfd;
    this->eventType = event;
    this->action = registerAction;
}


bool Reactor::reactorEventRegister(Event *event) {
    std::lock_guard<std::mutex> eventMangerLock(this->evetManagerLock);
    if (!event) {
        std::cerr << "event must be memory allocated" << std::endl;
        return false;
    }
    int eventExisted = false;
    for (auto const &eventReserve: eventManger) {
        if (eventReserve->listenfd == event->listenfd && eventReserve->eventType == event->eventType) {
            eventExisted = true;
            break;
        }
    }
    if (!eventExisted) {
        if (eventRegister(event)) {
            this->eventManger.push_back(event);
            std::cout << "event registration succeed" << std::endl;
            return true;
        } else {
            std::cerr << "event registration failed" << std::endl;
            return false;
        }
    } else {
        std::cout << "event already existed" << std::endl;
        return true;
    }
}

bool Reactor::reactorEventCancel(Event *event) {
    std::unique_lock<std::mutex> eventMangerLock(this->evetManagerLock);
    if (!event) {
        std::cout << "event has been deleted" << std::endl;
        return true;
    }
    if (eventCancel(event)) {

        this->eventManger.erase(std::remove_if(this->eventManger.begin(), this->eventManger.end(), [](auto &element) {
            if (element == nullptr)
                return true;
            else
                return false;
        }), this->eventManger.end());
        eventMangerLock.unlock();
        std::cout << "event deletion succeed" << std::endl;
        return true;
    } else {
        std::cerr << "event deletion failed" << std::endl;
        return false;
    }
}

bool Reactor::reactorInitialize(int serverfd, int maxEvent) {
    if (serverfd < 0) {
        std::cerr << "Please make sure that NGSaseServer::create work properly." << std::endl;
        return false;
    } else
        this->serverfd = serverfd;

    this->kqueuefd = kqueue();
    if (this->kqueuefd == -1) {
        std::cerr << "kqueue initialization failed in Reactor::reactorInitialize" << std::endl;
        return false;
    }
    if (maxEvent > 0)
        this->maxEvent = maxEvent;
    else {
        std::cerr << "maxEvent Number is less than 1" << std::endl;
        return false;
    }
    this->events = new struct kevent[maxEvent];
    return true;


}


Reactor::~Reactor() {
    std::unique_lock<std::mutex> eventMangerLock(this->evetManagerLock);
    for (auto &eventReserve: this->eventManger) {
        close(eventReserve->listenfd);
        if (eventReserve) {
            delete eventReserve;
            eventReserve = nullptr;
        }
    }
    eventMangerLock.unlock();
    if (kqueuefd > 0)
        close(kqueuefd);
    if (events) {
        delete events;
        events = nullptr;
    }
}

void Reactor::startEventMonitor(int second, int nanoSecond) {
    timespec *timeLimit = nullptr;
    if (this->eventManger.empty()) {
        std::cout << "There is no file descriptor in kqueue" << std::endl;
        return;
    }

    if (second <= 0 && nanoSecond <= 0) {
        std::cout << "eventLoop() start without timeout" << std::endl;
        eventLoop(this, timeLimit);
    } else if (nanoSecond > 0 || second > 0) {
        timeLimit = new timespec{second, nanoSecond};
        eventLoop(this, timeLimit);
        if (timeLimit) {
            delete timeLimit;
            timeLimit = nullptr;
        }

    }
}

template<class T>
ReactorPanel<T>::ReactorPanel(int minThreadNum, int maxThreadNum, eventCallBackFunc readFunc,
                              eventCallBackFunc writeFunc, eventCallBackFunc acceptFunc,
                              std::map<std::string, workerFunction> *workFuncMap) {};

template<class T>
ReactorPanel<T>::ReactorPanel() {};


template<class T>
bool
ReactorPanel<T>::assign(int minSiblingThreadNum, int maxSiblingThreadNum, int minHeirThreadNum, int maxHeirThreadNum,
                        eventCallBackFunc readFunc, eventCallBackFunc writeFunc, eventCallBackFunc acceptFunc,
                        std::map<std::string, workerFunction> *workFuncMap) {
    //make sure each thread number is correctly assigned.
    if (minSiblingThreadNum < 1 || minHeirThreadNum < 1) {
        std::cerr << "The minimum thread of sibling/heir must greater than 0" << std::endl;
        return false;
    } else {
        this->minSiblingThreadNum = minSiblingThreadNum;
        this->minHeirThreadNum = minHeirThreadNum;
    }
    if (maxSiblingThreadNum < 2 || maxHeirThreadNum < 2) {
        std::cerr << "The maximum thread of sibling/heir must greater than 1" << std::endl;
        return false;
    } else {
        this->maxSiblingThreadNum = maxSiblingThreadNum;
        this->maxHeirThreadNum = maxHeirThreadNum;
    }
    //make sure each callback function is correctly assigned.
    if (!readFunc || !writeFunc || !acceptFunc) {
        std::cerr << "read/write/accept function must not be nullptr" << std::endl;
        return false;
    } else {
        this->readFunc = readFunc;
        this->writeFunc = writeFunc;
        this->acceptFunc = acceptFunc;
    }
    if (!workFuncMap) {
        std::cerr << "the workerFunctionMap must be initialized" << std::endl;
        return false;
    } else
        this->workFuncMap = workFuncMap;

    return true;

}

bool FilePath::addFilePath(std::string abbreviation, std::string filePath) {

    auto abbreFilePathInsert = abbreFilePathMapper.insert(
            std::pair<std::string, std::string>(abbreviation, filePath));
    if (abbreFilePathInsert.second == false) {
        std::cout << "Insertion failed,the abbreviation is already existed" << std::endl;
        return false;
    }
//會有data race的情形所以先註釋，若每個都使用mutex速度太慢
/*    int filefd = open(filePath.c_str(), O_RDONLY);

    if (filefd < 0) {
        std::cout << "The file isn't existed" << std::endl;
        abbreFilePathMapper.erase(abbreviation);
        std::cout << "The insertion failed" << std::endl;
        return false;
    }*/

    /*auto filePathDescriptorInsert = filePathDescriptorMapper.insert(std::pair<std::string, int>(filePath, filefd));
    if (filePathDescriptorInsert.second == false) {
        std::cout << "The filePath is already existed" << std::endl;
        close(filefd);
        return false;
    }*/

    std::cout << abbreviation << " loading succeed" << std::endl;
    return true;

}

/*int FilePath::getFileDescriptor(std::string abbreviation) {

        auto it = abbreFilePathMapper.find(abbreviation);
        if (it == abbreFilePathMapper.end()) {
            std::cout << "There is no compatible key in the collection" << std::endl;
            return -1;
        }
        std::string filePath = abbreFilePathMapper[abbreviation];
        int filefd = filePathDescriptorMapper[filePath];
        return filefd;

}*/
std::string FilePath::getFilePath(std::string abbreviation) {
    auto it = abbreFilePathMapper.find(abbreviation);
    if (it != abbreFilePathMapper.end()) {
        // Key found, access the associated value
        return it->second;
    } else {
        return "KeyError";
    }

}

bool NGSaseParameter::assignUserInput(int argc, char *argv[]) {
    std::cout << "NGSase v1.0.0" << std::endl;
    if (argc == 2) {
        std::string argument = std::string(argv[1]);
        if (argument == "-h") {
            std::cout << std::left << std::setw(15) << "-a" << "IPv4 address with dot-decimal format" << std::endl;
            std::cout << std::left << std::setw(15) << "-p" << "port number" << std::endl;
            std::cout << std::left << std::setw(15) << "-b" << "backlog number" << std::endl;
            std::cout << std::left << std::setw(15) << "-minMainWT" << "Minimum number of MainReactor worker thread"
                      << std::endl;
            std::cout << std::left << std::setw(15) << "-maxMainWT" << "Maximum number of MainReactor worker thread"
                      << std::endl;
            std::cout << std::left << std::setw(15) << "-minSubWT" << "Minimum number of SubReactor worker thread"
                      << std::endl;
            std::cout << std::left << std::setw(15) << "-maxSubWT" << "Maximum number of SubReactor worker thread"
                      << std::endl;
            std::cout << std::left << std::setw(15) << "-sqlHost" << "Host Name of MySQL" << std::endl;
            std::cout << std::left << std::setw(15) << "-sqlUser" << "User Name of MySQL" << std::endl;
            std::cout << std::left << std::setw(15) << "-sqlPW" << "Password of MySQL" << std::endl;
            std::cout << std::left << std::setw(15) << "-h" << "argument information" << std::endl;
            return false;
        } else
            return false;
    }

    for (int i = 1; i < argc; i++) {
        std::string argument;
        std::string parameter;
        do {
            argument = std::string(argv[i]);
            if (argument == "-a") {
                parameter = std::string(argv[++i]);
                this->userAddress = parameter;
                break;
            } else if (argument == "-p") {

                parameter = std::string(argv[++i]);
                this->userPort = std::stoi(parameter);
                if (this->userPort < 0) {
                    std::cerr << "Please ensuer that you enter correct format of port" << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-b") {
                parameter = std::string(argv[++i]);
                this->userBacklog = std::stoi(parameter);
                if (this->userBacklog < 0) {
                    std::cerr
                            << "Please ensure that you enter correct format of backlog.it should be integer and greater than 0"
                            << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-minMainWT") {
                parameter = std::string(argv[++i]);
                this->userMainReactorMinWorkerNum = std::stoi(parameter);
                if (this->userMainReactorMinWorkerNum < 0) {
                    std::cerr
                            << "Please ensure that you enter correct format of MainReactorMinWorkerNum and greater than 0"
                            << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-maxMainWT") {
                parameter = std::string(argv[++i]);
                this->userMainReactorMaxWorkerNum = std::stoi(parameter);
                if (this->userMainReactorMaxWorkerNum < 0) {
                    std::cerr
                            << "Please ensure that you enter correct format of MainReactorMaxWorkerNum and greater than 0"
                            << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-minSubWT") {
                parameter = std::string(argv[++i]);
                this->userSubReactorMinWorkerNum = std::stoi(parameter);
                if (this->userSubReactorMinWorkerNum < 0) {
                    std::cerr
                            << "Please ensure that you enter correct format of SubReactorMinWorkerNum and greater than 0"
                            << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-maxSubWT") {
                parameter = std::string(argv[++i]);
                this->userSubReactorMaxWorkerNum = std::stoi(parameter);
                if (this->userSubReactorMaxWorkerNum < 0) {
                    std::cerr
                            << "Please ensure that you enter correct format of SubReactorMaxWorkerNum and greater than 0"
                            << std::endl;
                    return false;
                }
                break;
            } else if (argument == "-sqlHost") {
                parameter = std::string(argv[++i]);
                this->userMySQLHost = parameter;
                break;
            } else if (argument == "-sqlUser") {
                parameter = std::string(argv[++i]);
                this->userMySQLAccount = parameter;
                break;
            } else if (argument == "-sqlPW") {
                parameter = std::string(argv[++i]);
                this->userMySQLPassword = parameter;
                break;
            } else {
                std::cerr << "Unregoninized paramter" << std::endl;
                return false;
                break;
            }

        } while (0);


    }
    return true;

}

ContentType FilePath::getFileContentType(std::string abbreviation) {
    int extensionStart = abbreviation.find_last_of('.');
    if (extensionStart < 0) {
        std::cout << "the file extension isn't valid" << std::endl;
        return NONE;
    } else
        extensionStart += 1;

    std::string extension = abbreviation.substr(extensionStart);
    if (extension == "png")
        return IMAGE_PNG;
    else if (extension == "jpeg")
        return IMAGE_JPEG;
    else if (extension == "gif")
        return IMAGE_GIF;
    else if (extension == "svg")
        return IMAGE_SVG;
    else if (extension == "ico")
        return IMAGE_ICO;
    else if (extension == "js")
        return JAVASCRIPT;
    else if (extension == "html")
        return HTML;
    else if (extension == "css")
        return CSS;
    else if (extension == "json")
        return JSON;
    else {
        std::cout << "the file type haven't been supported" << std::endl;
        return NONE;
    }
}

NGSaseServer::~NGSaseServer() {
    if (mainReactorPanel) {
        delete mainReactorPanel;
        mainReactorPanel = nullptr;
    }
    if (workFunctionMap) {
        delete workFunctionMap;
        workFunctionMap = nullptr;
    }
}


std::string NGSaseServer::getFilePath(std::string fileName) {
    std::string filePath = filePathCollection.getFilePath(fileName);
    return filePath;
}

ContentType NGSaseServer::getFileContentType(std::string fileName) {
    ContentType contentType = filePathCollection.getFileContentType(fileName);
    return contentType;
}

bool NGSaseServer::filePathAssign() {
    //Check existence of directories
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path icons = currentPath / "icons";
    std::filesystem::path images = currentPath / "images";
    std::filesystem::path styles = currentPath / "styles";
    std::filesystem::path analysisStyles = styles / "analysis_styles";
    std::filesystem::path loginStyles = styles / "login_styles";
    std::filesystem::path scripts = currentPath / "scripts";
    std::filesystem::path html = currentPath / "htmls";
    std::filesystem::path ngsaseEXE = currentPath / "executeFiles";
//    if (!std::filesystem::exists(icons) || !std::filesystem::exists(images) ||
//        !std::filesystem::exists(analysisStyles) || !std::filesystem::exists(loginStyles) ||
//        !std::filesystem::exists(scripts) || !std::filesystem::exists(html)) {
//        std::cerr << "Please check NGSase dependency files in current execution path" << std::endl;
//        return false;
//    }
    std::string fileDirCheck;
    do {
        if (!std::filesystem::exists(icons)) {
            fileDirCheck = icons.string();
            break;
        }
        if (!std::filesystem::exists(images)) {
            fileDirCheck = images.string();
            break;
        }
        if (!std::filesystem::exists(analysisStyles)) {
            fileDirCheck = analysisStyles.string();
            break;
        }
        if (!std::filesystem::exists(loginStyles)) {
            fileDirCheck = loginStyles.string();
            break;
        }
        if (!std::filesystem::exists(scripts)) {
            fileDirCheck = scripts.string();
            break;
        }
        if (!std::filesystem::exists(html)) {
            fileDirCheck = html.string();
            break;
        }
        if (!std::filesystem::exists(ngsaseEXE)) {
            fileDirCheck = ngsaseEXE.string();
            break;
        }

    } while (0);
    if (!fileDirCheck.empty()) {
        std::cerr << "Please check NGSase dependency files in current execution path (" << fileDirCheck << ")"
                  << std::endl;
        return false;
    } else
        std::cout << "file directory check succeed" << std::endl;
    //check file existence and register to NGSaseServer::filePathCollection
    checkFileExisted(icons);
    checkFileExisted(images);
    checkFileExisted(analysisStyles);
    checkFileExisted(loginStyles);
    checkFileExisted(scripts);
    checkFileExisted(html);
    checkFileExisted(ngsaseEXE);
    bool filesCheck = true;
    for (const auto &it: NGSaseServer::fileNameCollection) {
        if (it.second == false) {
            std::cerr << it.first << " is missing in current execution path" << std::endl;
            filesCheck = false;
        }
    }
    return filesCheck;

}

bool NGSaseServer::ngsaseDatabaseInitialize() {
    ngsase_database ngsaseDB(ngsaseDBInfo.host, ngsaseDBInfo.username, ngsaseDBInfo.password, ngsaseDBInfo.port,
                             ngsaseDBInfo.unixSocket, ngsaseDBInfo.setting);
    if (ngsaseDB.connection_established)
        return true;
    else
        return false;
}

NGSaseServer::NGSaseServer(std::string address, int port, int backlog, int mainReactorMaxWorkerNum,
                           int mainReactorMinWorkerNum, int subReactorMaxWorkerNum, int subReactorMinWorkerNum,
                           std::string mysqlHost, std::string mysqlUsername, std::string mysqlpassword, int mysqlPort,
                           const char *unixSocket) {
    this->serverAddress = address;
    this->port = port;
    this->backlog = backlog;
    this->mainReactorMaxWorkerNum = mainReactorMaxWorkerNum;
    this->mainReactorMinWorkerNum = mainReactorMinWorkerNum;
    this->subReactorMaxWorkerNum = subReactorMaxWorkerNum;
    this->subReactorMinWorkerNum = subReactorMinWorkerNum;
    this->ngsaseDBInfo.assign(mysqlHost, mysqlUsername, mysqlpassword, mysqlPort, unixSocket);
    fileNameCollection.insert(std::pair<std::string, bool>("favicon.ico", false));
    fileNameCollection.insert(std::pair<std::string, bool>("NGSase.svg", false));
    fileNameCollection.insert(std::pair<std::string, bool>("NGSase_system_design.svg", false));
    fileNameCollection.insert(std::pair<std::string, bool>("analysis_script.js", false));
    fileNameCollection.insert(std::pair<std::string, bool>("analysis_general.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("analysis_main.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("analysis_sidebar.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("login_general.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("login_input.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("login_introduction.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("login_total_test.css", false));
    fileNameCollection.insert(std::pair<std::string, bool>("analysis.html", false));
    fileNameCollection.insert(std::pair<std::string, bool>("login.html", false));
    fileNameCollection.insert(std::pair<std::string, bool>("ngsase_execute", false));

    if (!this->mainReactorPanel)
        this->mainReactorPanel = new ReactorPanel<Event *>();
    if (!this->workFunctionMap)
        this->workFunctionMap = new std::map<std::string, workerFunction>();

}

int NGSaseServer::createServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Create NGSaseServer..." << std::endl;
    if (serverSocket == -1) {
        std::cerr << "socket() error in NGSaseServer::createServer(): " << strerror(errno) << std::endl;
        return -1;
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

// Convert IPv4 address from dotted-decimal notation to binary
    if (inet_aton(this->serverAddress.c_str(), &serv_addr.sin_addr) == 0) {
        std::cerr << "address conversion failed in NGSaseServer::createServer(): " << strerror(errno) << std::endl;
        return -1;
    }
    serv_addr.sin_port = htons(this->port);
    serv_addr.sin_family = AF_INET;
    if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        std::cerr << "bind() error in NGSaseServer::createServer(): " << strerror(errno) << std::endl;
        return -1;
    }

    if (listen(serverSocket, this->port) == -1) {
        std::cerr << "listen()error in NGSaseServer::createServer(): " << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "NGSaseServer: Socket creation completed" << std::endl;

    return serverSocket;


}

void NGSaseServer::funcAssign() {

    this->workFunctionMap->insert(std::pair<std::string, workerFunction>("MainReactor", mainReactorWorkFunc));
    this->workFunctionMap->insert(std::pair<std::string, workerFunction>("SubReactor", subReactorWorkFunc));
    readFunc = readSocket;
    writeFunc = writeSocket;
    acceptFunc = acceptSocket;
}


void NGSaseServer::start() {
    do {
        //Assign function to ReactorPanel including worker function and event call back function
        funcAssign();
        if (!mainReactorPanel->assign(this->mainReactorMinWorkerNum, this->mainReactorMaxWorkerNum,
                                      this->subReactorMinWorkerNum, this->subReactorMaxWorkerNum, this->readFunc,
                                      this->writeFunc, this->acceptFunc, this->workFunctionMap)) {
            std::cerr << "mainReactor assignment failed in ReactorPanel::assign()" << std::endl;
            break;
        } else
            std::cout << "mainReactor assignment succeed" << std::endl;
        //Assign necessary file
        if (!filePathAssign()) {
            std::cerr << "file dependency check failed in NGSase::filePathAssign()" << std::endl;
            break;
        } else
            std::cout << "file dependency check succeed" << std::endl;

        //initialize NGSase database including database creation, connection examination
        if (!ngsaseDatabaseInitialize()) {
            std::cerr << "NGSase database initialize failed in NGSaseServer::ngsaseDatabaseInitialize()" << std::endl;
            break;
        } else
            std::cout << "NGSase database initialize succeed" << std::endl;
        //create a server socket
        int serverfd = createServer();
        if (serverfd < 0) {
            std::cerr << "server fd creation failed in NGSaseServer::createServer()" << std::endl;
            break;
        } else
            std::cout << "server fd creation succeed" << std::endl;

        if (!mainReactorPanel->reactor.reactorInitialize(serverfd, 5)) {
            std::cerr << "reactor initialize failed in NGSaseServer::start()" << std::endl;
            break;
        } else
            std::cout << "reactor initialize succeed" << std::endl;
        //if ngsaseDatabase is initialized,assign confirmed information to mainReactorPanel
        mainReactorPanel->databaseInfo.assign(this->ngsaseDBInfo.host, this->ngsaseDBInfo.username,
                                              this->ngsaseDBInfo.password, this->ngsaseDBInfo.port,
                                              this->ngsaseDBInfo.unixSocket, this->ngsaseDBInfo.setting);
        if (!mainReactorPanel->readFunc)
            std::cout << "null function" << std::endl;
        //Register server fd to reactor in mainReactorPanel
        Event *serverEvent = new Event(mainReactorPanel->reactor.serverfd, mainReactorPanel->reactor.kqueuefd,
                                       this->serverAddress,
                                       TriggerMode::LEVEL,
                                       EVFILT_READ, EV_ADD, mainReactorPanel->readFunc, mainReactorPanel->writeFunc,
                                       mainReactorPanel->acceptFunc);

        if (!mainReactorPanel->reactor.reactorEventRegister(serverEvent)) {
            std::cerr << "register server fd failed in Reactor::reactorEventRegister()" << std::endl;
            delete serverEvent;
            break;
        } else
            std::cout << "register server fd succeed in reactor" << std::endl;


        mainReactorPanel->threadPool.createThreadPool(this->mainReactorMinWorkerNum, this->mainReactorMaxWorkerNum,
                                                      (*this->workFunctionMap)["MainReactor"], mainReactorPanel);

        //之後要補寫sigation操作shutdown;
        std::cout << "mainReactor thread id: " << std::this_thread::get_id() << std::endl;
        while (!mainReactorPanel->threadPool.threadPoolShutdown) {
            mainReactorPanel->reactor.startEventMonitor(10, 0);
            mainReactorPanel->threadPool.threadManage((*this->workFunctionMap)["MainReactor"], mainReactorPanel, 15);
        }


    } while (0);


}

bool eventRegister(Event *registerEvent) {
    struct kevent ev;
    EV_SET(&ev, registerEvent->listenfd, registerEvent->eventType, registerEvent->action, 0, 0, registerEvent);
    if (kevent(registerEvent->kqueuefd, &ev, 1, NULL, 0, NULL) == -1)
        return false;
    else
        return true;

}

bool eventCancel(Event *previousEvent) {
    struct kevent ev;
    EV_SET(&ev, previousEvent->listenfd, previousEvent->eventType, EV_DELETE, 0, 0, NULL);
    if (kevent(previousEvent->kqueuefd, &ev, 1, NULL, 0, NULL) == -1) {
        std::cerr << "event delete error: " << strerror(errno) << std::endl;
        return false;

    } else {
        delete previousEvent;
        previousEvent = nullptr;
        return true;

    }
}

void eventLoop(Reactor *r, timespec *timeLimit) {


    int eventCount = kevent(r->kqueuefd, nullptr, 0, r->events, r->maxEvent, timeLimit);
    for (int i = 0; i < eventCount; i++) {
        int mask = 0;
        struct kevent *e = &r->events[i];
        if (e->filter == EVFILT_READ)
            mask |= EVFILT_READ;
        if (e->filter == EVFILT_WRITE)
            mask |= EVFILT_WRITE;
        if (e->flags & (EV_ERROR | EV_EOF))
            mask |= EVFILT_READ | EVFILT_WRITE;
        if (mask & EVFILT_READ) {
            Event *event = (Event *) e->udata;
            //分配任務 subreactor的serverfd會為0，所以serverfd!=task->listenfd
            if (r->serverfd == event->listenfd) {
                if (event->acceptFunc)
                    event->acceptFunc(event, r);
            } else {
                if (event->readFunc)
                    event->readFunc(event, r);
            }

        }
    }

}

bool setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "setting failed on" << fd << " with non-blocking mode(" << strerror(errno) << ")" << std::endl;
        return false;
    } else
        return true;

}

bool setSocketNOSIGPIPE(int fd) {
    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) < 0) {
        std::cerr << "setSocketNOSIGPIPE() failed on " << fd << std::endl;
        return false;
    } else
        return true;
}

void mainReactorWorkFunc(void *threadPool, void *reactorPanel) {
    /* The main tasks performed by this SubReactorWorkerFunc are as follows:
      * (1) Retrieve tasks from the main reactor's task queue and register the corresponding event listeners.
      * (2) Implement simple load balancing, where each subreactor monitors up to 10 file descriptors (fd).
      * (3) Manage the status of the request-process thread pool.
      * */

    // Remember to write a function for the taskManager that iterates through and clears any nullptrs after deleting a task.
    std::cout << "subreactor thread id: " << std::this_thread::get_id() << std::endl;
    ReactorPanel<Event *> *mainReactor = (ReactorPanel<Event *> *) reactorPanel;
    ReactorPanel<Task *> *subReactor = new ReactorPanel<Task *>;
    subReactor->assign(mainReactor->minHeirThreadNum, mainReactor->maxHeirThreadNum, 1, 3, mainReactor->readFunc,
                       mainReactor->writeFunc, mainReactor->acceptFunc, mainReactor->workFuncMap);
    subReactor->databaseInfo.assign(mainReactor->databaseInfo.host, mainReactor->databaseInfo.username,
                                    mainReactor->databaseInfo.password, mainReactor->databaseInfo.port,
                                    mainReactor->databaseInfo.unixSocket, mainReactor->databaseInfo.setting);

// If one file descriptor (fd) is registered for both read and write, it counts as 2 fds.
// Considering some fds may not be registered for both read and write,
// the fdCapacity is set to maxEvent/2 to provide some leeway.
    int maxEvent = 30;
    int fdReserve = 0;
    int maxfdCapacity = maxEvent / 2;
    bool threadTermination = false;
    bool busyThreadSetted = false;
    std::future<Event *> asyncEvent;
    //workerFunction subreactorFunc=(*subReactor->workFuncMap)["SubReactor"];
    //若發現reactorInitialized沒辦法
    if (!subReactor->reactor.reactorInitialize(mainReactor->reactor.serverfd, 30))
        threadTermination = true;
    else
        subReactor->threadPool.createThreadPool(subReactor->minSiblingThreadNum, subReactor->maxSiblingThreadNum,
                                                (*subReactor->workFuncMap)["SubReactor"], subReactor);


    while (!threadTermination) {

        do {
            /*簡單的load-balance，若監聽的fd少於10則增加監聽fd。*/
            if (fdReserve <= maxfdCapacity) {
                Event *eventFetched = nullptr;

                /* First, check if the taskManager's count is zero.
                 * If it is, block until a task is obtained from the main reactor's task queue,
                 * so that the event loop can start monitoring events.
                 * Second, if the taskManager's count is greater than zero,
                 * use std::async to fetch another event from the main reactor's task queue without causing this thread to sleep.
                 * Also, set the timeout for the event loop, allowing it to acquire new fds for monitoring without pausing to fetch events from the main reactor's task queue.
                 * */
                if (subReactor->reactor.eventManger.empty()) {

                    eventFetched = (Event *) mainReactor->threadPool.fetchTask(threadTermination, busyThreadSetted);
                    if (eventFetched == nullptr &&
                        threadTermination == true) {
                        break;
                    }
                    eventFetched->kqueuefd = subReactor->reactor.kqueuefd;

                    if (!subReactor->reactor.reactorEventRegister(eventFetched)) {
                        std::cerr << "something wrong in your kqueue in subReactor" << std::endl;
                        threadTermination = true;
                        break;
                    }
                    fdReserve = subReactor->reactor.eventManger.size();


                } else {
                    asyncEvent = std::async(std::launch::async,
                                            [mainReactor](bool &Termination, bool &busyThreadSetted) {
                                                return (Event *) mainReactor->threadPool.fetchTask(Termination,
                                                                                                   busyThreadSetted);
                                            }, std::ref(threadTermination), std::ref(busyThreadSetted));
                }

                /* To avoid keeping the ayncEvent continuously blocking,
                 * when more than 100ms have elapsed,
                 * the program should enter the event loop to monitor events.
                 * Conversely, if a task is obtained from the main reactor's task queue,
                 * exit the while loop.
                 * */
                // Remember to pay attention to registering Write events. There may be various issues to handle when writing,
                // and it's not yet fully understood all the possible error scenarios that could occur during writing.
                // However, for now, it's best to prioritize not registering Write events.
                while (!eventFetched && threadTermination == false) {
                    int waitTimes = 0;
                    if (asyncEvent.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
                        subReactor->reactor.startEventMonitor(0, 10000000);
                        // If no Event is obtained for five consecutive times and the eventManager is empty,
                        // let this thread die, indicating that there may not be as many connections.
                        waitTimes++;
                        if (waitTimes > 5 && subReactor->reactor.eventManger.empty()) {
                            threadTermination = true;
                            break;
                        }
                    } else {
                        //Obtain event from main reactor's task queue, register it.
                        eventFetched = asyncEvent.get();
                        if (eventFetched && threadTermination == false)
                            eventFetched->kqueuefd = subReactor->reactor.kqueuefd;
                        subReactor->reactor.reactorEventRegister(eventFetched);
                        fdReserve = subReactor->reactor.eventManger.size();
                        break;
                    }

                }

            }

        } while (0);


        if (threadTermination)
            break;

        subReactor->reactor.startEventMonitor(0, 10000000);
        subReactor->threadPool.threadManage((*subReactor->workFuncMap)["SubReactor"], subReactor, 1);


    }
    if (busyThreadSetted) {
        std::lock_guard<std::mutex> busyThreadLock(subReactor->threadPool.busyThreadMutex);
        subReactor->threadPool.busyThreadNum--;
    }

    subReactor->threadPool.shutdownThreadPool();
    delete subReactor;

}

void subReactorWorkFunc(void *threadPool, void *reactorPanel) {
    ReactorPanel<Task *> *subReactor = (ReactorPanel<Task *> *) reactorPanel;
    bool threadTermination = false;
    bool busyThreadSetted = false;
    ngsase_database ngsaseDB(subReactor->databaseInfo.host, subReactor->databaseInfo.username,
                             subReactor->databaseInfo.password, subReactor->databaseInfo.port,
                             subReactor->databaseInfo.unixSocket, subReactor->databaseInfo.setting);
    while (!threadTermination) {
        Task *fetchedTask = subReactor->threadPool.fetchTask(threadTermination, busyThreadSetted);
        if (!fetchedTask && threadTermination)
            break;
        ProcResult procResult;
        switch (fetchedTask->actionType) {
            case Action::LOGIN: {
                LoginReqProc loginReqProc;
                procResult = loginReqProc.taskProcess(fetchedTask, ngsaseDB);
                break;
            }
            case Action::REGISTER: {
                RegisterReqProc registerReqProc;
                procResult = registerReqProc.taskProcess(fetchedTask, ngsaseDB);
                break;
            }
            case Action::PIPELINE: {
                PipelineReqProc pipelineReqProc;
                procResult = pipelineReqProc.taskProcess(fetchedTask, ngsaseDB);
                break;
            }
            case Action::PROJECT: {
                ProjectReqProc projectReqProc;
                procResult = projectReqProc.taskProcess(fetchedTask, ngsaseDB);
                break;
            }
            default: {
                std::cout << "There is no compatible Action for NGSase server" << std::endl;
            }
        }
        HTTPResponse response("HTTP/1.1", "200 OK", NONE, "", "");
        std::string responseToClientMSG;
        switch (procResult.responseType) {
            case JSON: {
                response.responseType = JSON;
                response.body = procResult.responseBody;
                responseToClientMSG = response.responseToString(false, false, true);
                writeHTTPResp(fetchedTask->filefd, responseToClientMSG, 0, 0);
                break;
            }
            case BINARY: {
                response.responseType = BINARY;
                std::string header = "Content-Disposition : attachment; filename=";
                header += ("\"" + procResult.fileName + "\"");
                std::vector<std::string> expandHeader{header};
                response.filefd = procResult.filefd;
                responseToClientMSG = response.responseToString(true, true, true, expandHeader);
                writeHTTPResp(fetchedTask->filefd, responseToClientMSG, 0, 0);
                writeHTTPResp(fetchedTask->filefd, responseToClientMSG, response.filefd, 1);
                close(response.filefd);
                break;
            }
        }
        delete fetchedTask;

    }
    if (busyThreadSetted) {
        std::lock_guard<std::mutex> busyThreadLock(subReactor->threadPool.busyThreadMutex);
        subReactor->threadPool.busyThreadNum--;
    }
}

void writeSocket(void *event, void *reactor) {

};

//accept socket for further data transferring
void acceptSocket(void *event, void *reactor) {
    Event *fetchedEvent = (Event *) event;
    ReactorPanel<Event *> *mainReactor = (ReactorPanel<Event *> *) reactor;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_sz = sizeof(client_addr);
    int client_socket = accept(fetchedEvent->listenfd, (struct sockaddr *) &client_addr, &client_addr_sz);
    if (client_socket < 0) {
        printf("accept error:%s\n", strerror(errno));
        return;
    } else
        std::cout << "client_socket acceptation succeed" << std::endl;

    bool setSocket;
    setSocket = setNonBlock(client_socket);
    setSocket = setSocketNOSIGPIPE(client_socket);
    if (!setSocket) {
        std::cerr << "set client socket " << client_socket << " failed" << std::endl;
        return;
    } else
        std::cout << "set client socket " << client_socket << " succeed" << std::endl;
    char clientIP[INET_ADDRSTRLEN];
    std::string clientDDN;
    if ((inet_ntop(AF_INET, &(client_addr.sin_addr), clientIP, INET_ADDRSTRLEN) != NULL)) {
        clientDDN = std::string(clientIP);
        std::cout << "Dot-Decimal Notation conversion succeed" << std::endl;
    } else {
        clientDDN = "Dot-Decimal Notation conversion failed";
    }

    Event *subReactorEvent = new Event(client_socket, 0, clientDDN, TriggerMode::EDGE, EVFILT_READ, EV_ADD,
                                       fetchedEvent->readFunc, fetchedEvent->writeFunc, fetchedEvent->acceptFunc);
    std::cout << "mainReactor add event to taskqueue with ThreadPool::addTask()" << std::endl;
    mainReactor->threadPool.addTask(subReactorEvent);


}

/* For non-blocking sockets,
 * it's necessary to consider cases where "resource temporarily unavailable" may occur,
 * especially during large file transfers. */
void writeHTTPResp(int fd, std::string &totalString, int filefd, int sendOption) {
    switch (sendOption) {
        case 0: {
            int msgSendResult = send(fd, totalString.c_str(), totalString.size(), 0);
            if (msgSendResult < 0)
                std::cout << "Message sending failed" << std::endl;
            else
                std::cout << "Messsage sending succeed" << std::endl;
            break;
        }
        case 1: {
            if (filefd > 0) {
                struct stat st;
                fstat(filefd, &st);
                off_t offset = 0;
                off_t fileSize = st.st_size;
                off_t len = 4096;
                off_t fileSendResult;
                /* Using select() to detect the availability of the write buffer also generates another issue:
                 * the write buffer can become blocked when two threads attempt to write to the same socket simultaneously.
                 * Therefore, the timing for writing to the buffer needs to be interleaved by using a random factor as a multiplier.
                 * */


                //   fd_set writeSet, tempWriteSet;
                //   FD_ZERO(&writeSet);
                // FD_SET(fd, &writeSet);
                // struct timeval timeoutTemp;
                int timeoutBase = 1;
                int timeoutTemp = 0;
                int randomNum = 1;
                std::random_device randomDevice;
                std::mt19937 randomGenerator(randomDevice());
                std::uniform_int_distribution<> dis(1, 10);
                std::cout << "sending socket: " << fd << std::endl;
                std::cout << "start sending file(" << fileSize << " bytes)" << std::endl;

                while (offset < fileSize) {
                    fileSendResult = sendfile(filefd, fd, offset, &len, nullptr, 0);
                    if (fileSendResult == 0) {
                        std::cout << "File sending completed" << std::endl;
                        std::cout << "File sending size: " << len << std::endl;
                        offset += len;
                        std::cout << "File already sent size:" << offset << std::endl;
                        len = 4096;

                    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        std::cerr << "File sending error" << std::strerror(errno) << std::endl;
                        randomNum = dis(randomGenerator);
                        timeoutTemp = timeoutBase * randomNum;
                        std::cout << "start sleep for " << timeoutTemp << " msec..." << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutTemp));
                        //usleep(timeoutTemp);

                        continue;

                    } else {
                        std::cerr << "File sending error" << std::strerror(errno) << std::endl;
                        break;
                    }
                }
                /*  if (fileSendResult > 0)
                      std::cout << "File sending succeed" << std::endl;*/
                /* off_t offset = 0;
                 off_t len = st.st_size;
                 off_t fileSendResult;
                 while ((fileSendResult = sendfile(filefd, fd, offset, &len, nullptr, 0)) == -1) {
                     if (errno == EAGAIN || errno == EWOULDBLOCK) {
                         std::cout << "blocking" << std::endl;
                         // The operation would block, retry after a short delay
                         usleep(10000); // Sleep for 10 milliseconds
                         continue;
                     } else {
                         // Another error occurred
                         std::cerr << "File sending failed: " << std::strerror(errno) << std::endl;
                         break;
                     }
                 }
                 if (fileSendResult >= 0) {
                     std::cout << "this file sending..." << std::endl;
                     std::cout << "File sending succeed" << std::endl;
                 }*/

            }
            break;
        }
        default:
            std::cout << "The setOption isn't valid, 0 for totalString 1 for filefd" << std::endl;
    }

}

void writeHTTPResp(int fd, int filefd, int fileStartPos) {
    if (filefd > 0) {
        struct stat st;
        fstat(filefd, &st);
        off_t offset = fileStartPos;
        off_t fileSize = st.st_size;
        off_t len = 4096;
        off_t fileSendResult;
        int timeoutBase = 10000;
        int timeoutTemp = 0;
        int randomNum = 1;
        std::random_device randomDevice;
        std::mt19937 randomGenerator(randomDevice());
        std::uniform_int_distribution<> dis(1, 10);
        std::cout << "sending socket: " << fd << std::endl;
        std::cout << "start sending file(" << fileSize << " bytes)" << std::endl;

        while (offset < fileSize) {
            fileSendResult = sendfile(filefd, fd, offset, &len, nullptr, 0);
            if (fileSendResult == 0) {
                std::cout << "File sending completed" << std::endl;
                std::cout << "File sending size: " << len << std::endl;
                offset += len;
                std::cout << "File already sent size:" << offset << std::endl;
                len = 4096;

            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "File sending error" << std::strerror(errno) << std::endl;
                randomNum = dis(randomGenerator);
                timeoutTemp = timeoutBase * randomNum;
                std::cout << "start sleep for " << timeoutTemp << " msec..." << std::endl;
                usleep(timeoutTemp);
                continue;

            } else {
                std::cerr << "File sending error" << std::strerror(errno) << std::endl;
                break;
            }
        }
    }
}

void readSocket(void *event, void *reactor) {
    ReactorPanel<Task *> *subReactor = (ReactorPanel<Task *> *) reactor;
    Event *fetchedEvent = (Event *) event;
    HTTPRequest *request = new HTTPRequest;
    CheckState checkstate = CHECK_STATE_REQUESTLINE;
    Multipart mulitpartCheck;
    int fd = fetchedEvent->listenfd;


    while (1) {
        int data_read = 0;
        int read_index = 0;
        int check_index = 0;
        int start_line = 0;
        int bufferSize = 4096;
        char buffer[bufferSize];
        memset(buffer, 0, 4096);
        data_read = recv(fd, buffer + read_index, bufferSize - read_index, 0);

        if (data_read == -1) {
            if (errno == EAGAIN) {
                if (request->contentType == MULTIPART) {
                } else
                    break;
            } else {
                printf("read error():%s\n", strerror(errno));
                std::cout << "error fd:" << fd << std::endl;
                subReactor->reactor.reactorEventCancel(fetchedEvent);
                close(fd);
                return;
            }
        } else if (data_read > 0) {
            read_index += data_read;
            HTTPCode result = parse_content(buffer, check_index, checkstate, read_index, start_line, *request,
                                            mulitpartCheck);
            if (result == NO_REQUEST)
                continue;
            else if (result == GET_HEADER) {
                continue;
            } else if (result == GET_REQUEST) {
                break;
            } else {
                printf("something error\n");
                break;
            }
        } else if (data_read == 0) {
            std::cout << "the fd " << fd << " is disconnected" << std::endl;
            subReactor->reactor.reactorEventCancel(fetchedEvent);
            close(fd);
        }
    }
    if (request->outputFile.is_open())
        request->outputFile.close();
    if (request->URL.empty()) {
        std::cout << "BAD request" << std::endl;
        HTTPResponse responseToClient("HTTP/1.1", "400 Bad Request", HTML, "404 Not Found", "close");
        std::string response = responseToClient.responseToString(false, false, true);
        writeHTTPResp(fd, response, 0, 0);
        delete request;
        return;
    }
    Task *task = new Task(fetchedEvent->kqueuefd, fd, request);
    switch (request->contentType) {
        case NONE: {
            std::string loginEmail;
            std::string requestFileName = request->URL.substr(request->URL.rfind("/") + 1);
            std::string filePath = NGSaseServer::getFilePath(requestFileName);

            if (request->URL.find("@") != std::string::npos &&
                request->URL.find("/analysis.html") != std::string::npos) {
                std::size_t emailPosStart = 0;

                int slashNum = 0;
                while ((emailPosStart = request->URL.find("/", emailPosStart)) != std::string::npos &&
                       slashNum < 2) {
                    slashNum++;
                    if (emailPosStart == 0)
                        emailPosStart += 1;
                }
                emailPosStart += 1;
                int lastSlashPos = request->URL.rfind("/");
                loginEmail = request->URL.substr(emailPosStart, lastSlashPos - emailPosStart);
            }


            if (filePath == "KeyError") {
                HTTPResponse responseToClient("HTTP/1.1", "400 Bad Request", HTML, "404 Not Found", "close");
                std::string response = responseToClient.responseToString(false, false, true);
                writeHTTPResp(fd, response, 0, 0);
            } else if (!loginEmail.empty()) {
                std::string line;
                char htmlC;
                std::string data;
                std::string composeString = "Info_Loading_Faild";
                std::cout << "composeString: " << composeString << std::endl;
                std::ifstream analysisHTML(filePath);
                int analysisHTMLFD = -1;
                HTTPResponse responseToClient("HTTP/1.1", "200 OK", HTML, "", "");
                bool fileCheck = false;
                int originalReplacePos = 0;
                do {
                    if (analysisHTML.is_open()) {
                        while (analysisHTML.get(htmlC)) {

                            data += htmlC;
//                            if (htmlC == '\n')
//                                std::cout << "breakline n" << std::endl;
//                            else if (htmlC == '\r')
//                                std::cout << "breakline r" << std::endl;
                            ssize_t composeStringPos = data.find(composeString);
                            if (composeStringPos != std::string::npos) {
                                originalReplacePos = data.size();
                                data.replace(composeStringPos, composeString.size(), loginEmail);
//                                std::cout << std::endl;
//                                std::cout << "data: " << data << std::endl;
                                break;
                            }
                        }
                    } else {
                        std::cerr << "open analysis.html failed" << std::endl;
                        break;
                    }
                    analysisHTMLFD = open(filePath.c_str(), O_RDONLY);

                    if (analysisHTMLFD < 0) {
                        std::cerr << "open analysis.html failed" << std::endl;
                        break;
                    } else
                        fileCheck = true;

                } while (0);
                if (fileCheck) {
                    responseToClient.filefd = analysisHTMLFD;
                    struct stat stat_buf;
                    fstat(analysisHTMLFD, &stat_buf);
                    off_t len = stat_buf.st_size;
                    int reviseContentLen = static_cast<int>(len) - composeString.size() + loginEmail.size();
                    responseToClient.contentLength = reviseContentLen;
                    //responseToClient.contentLength
                    std::string response = responseToClient.responseToString(true, false, false);
                    response += data;
                    std::cout << "analysis html Response: " << response << std::endl;
                    writeHTTPResp(fd, response, 0, 0);
                    writeHTTPResp(fd, analysisHTMLFD, originalReplacePos);
                    if (analysisHTML.is_open())
                        analysisHTML.close();
                    close(analysisHTMLFD);

                } else {
                    responseToClient.status = "400 Bad Request";
                    responseToClient.body = "404 Not Found";
                    responseToClient.connection = "close";
                    std::string response = responseToClient.responseToString(false, false, true);
                    writeHTTPResp(fd, response, 0, 0);

                }


            } else {

                int fileFD = open(filePath.c_str(), O_RDONLY);
                if (fileFD > 0) {
                    HTTPResponse responseToClient("HTTP/1.1", "200 OK", HTML, "", "");
                    responseToClient.filefd = fileFD;
                    responseToClient.responseType = NGSaseServer::getFileContentType(requestFileName);
                    std::string response = responseToClient.responseToString(true, false, true);
                    writeHTTPResp(fd, response, 0, 0);
                    writeHTTPResp(fd, response, responseToClient.filefd, 1);
                    close(fileFD);
                } else {
                    HTTPResponse responseToClient("HTTP/1.1", "400 Bad Request", HTML, "404 Not Found", "close");
                    std::string response = responseToClient.responseToString(false, false, true);
                    writeHTTPResp(fd, response, 0, 0);
                }
            }


            delete request;
            request = nullptr;
            break;
        }
        case JSON: {

            if (request->body.empty()) {
                HTTPResponse responseToClient("HTTP/1.1", "400 Bad Request", HTML,
                                              "Empty body, Please check your connection", "close");
                std::string response = responseToClient.responseToString(false, false, true);
                writeHTTPResp(fd, response, 0, 0);
            }

            if (request->URL.find("login") != std::string::npos)
                task->actionType = Action::LOGIN;
            else if (request->URL.find("register") != std::string::npos)
                task->actionType = Action::REGISTER;
            else if (request->URL.find("pipeline") != std::string::npos)
                task->actionType = Action::PIPELINE;
            else if (request->URL.find("project") != std::string::npos) {
                task->actionType = Action::PROJECT;
                task->executePath = NGSaseServer::getFilePath("ngsase_execute");
            } else
                task->actionType = Action::NO_ACTION;
            break;
        }
        case BINARY: {
            if (request->URL.find("pipeline") != std::string::npos)
                task->actionType = Action::PIPELINE;
            else if (request->URL.find("project") != std::string::npos)
                task->actionType = Action::PROJECT;
            else
                task->actionType = Action::NO_ACTION;

            break;
        }
        default:
            std::cout << "Your contentType haven't been supported" << std::endl;
    }
    if (request != nullptr && task->actionType != Action::NO_ACTION) {
        std::cout << request->body << std::endl;
        subReactor->threadPool.addTask(task);

    } else
        delete task;


}


void NGSaseServer::checkFileExisted(const std::filesystem::path &directory) {
    for (const auto &entry: std::filesystem::directory_iterator(directory)) {
        if (std::filesystem::is_regular_file(entry)) {
            std::string fileName = entry.path().filename().string();
            if (fileNameCollection.find(fileName) != fileNameCollection.end()) {
                filePathCollection.addFilePath(fileName, entry.path().string());
                fileNameCollection[fileName] = true;
            }

        } else if (std::filesystem::is_directory(entry)) {
            checkFileExisted(entry.path());
        }
    }
}