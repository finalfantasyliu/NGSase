#include "ngsase_requestProc.h"

ProcResult LoginReqProc::taskProcess(Task *fetchTask, ngsase_database &ngsaseDB) {

    ProcResult procResult;
    LoginRegister loginInfo;
    std::string queryString;
    json jsonObject;
    std::string responseMSG;

    bool loginCheck = loginInfo.assignFromBody(fetchTask->request->body);
    if (loginCheck) {
        queryString = loginInfo.getQueryString(AccountBehaviour::LOGIN);
        if (queryString.empty()) {
            std::cout << "The data is not completed, Please use Json deserialization first" << std::endl;
            jsonObject = {"message", "Server Error: json deserialization fault in login"};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
        }
    }
    mysql_result *result = ngsaseDB.query(queryString);
    if (result->errorMSG.empty() && result->affectRow > 0) {
        responseMSG = "/";
        responseMSG += "analysis/" + loginInfo.email + "/analysis.html";
        jsonObject = {
                {"newURL", responseMSG}};
        procResult.responseType = JSON;
        procResult.responseBody = jsonObject.dump();
    } else if (result->affectRow == 0) {
        jsonObject = {
                {"message", "Login Failed"}};
        procResult.responseType = JSON;
        procResult.responseBody = jsonObject.dump();
    } else {
        responseMSG = "Error Occurred";
        jsonObject = {
                {"message", responseMSG}
        };
        procResult.responseType = JSON;
        procResult.responseBody = jsonObject.dump();
    }
    delete result;
    return procResult;
}

ProcResult RegisterReqProc::taskProcess(Task *fetchTask, ngsase_database &ngsaseDB) {
    LoginRegister registerInfo;
    ProcResult procResult;
    std::string queryString;
    std::string responseMSG;
    json jsonObject;
    bool registerCheck = registerInfo.assignFromBody(fetchTask->request->body);
    if (registerCheck) {
        queryString = registerInfo.getQueryString(AccountBehaviour::REGISTER);
        if (queryString.empty()) {
            std::cout << "The data is not completed, Please use Json deserialization first" << std::endl;
            jsonObject = {"message", "Server Error: json deserialization fault in register"};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
        }
    }
    mysql_result *result = ngsaseDB.query(queryString);
    if (result->errorMSG.empty()) {
        if (result->affectRow == 1) {
            jsonObject = {
                    {"message", "succeed, Re-Login"}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();

        }
    } else {

        if (result->errorMSG.find("Duplicate") != std::string::npos) {
            jsonObject = {
                    {"message", "Duplicated Account"}};
        } else
            jsonObject = {{"message", "Error Occurred"}};
        procResult.responseType = JSON;
        procResult.responseBody = jsonObject.dump();
    }

    delete result;
    return procResult;
}

PipelineTask PipelineReqProc::assignTask(std::string &URL) {
    size_t firstSlashPos = URL.find('_');
    if (firstSlashPos == std::string::npos)
        return PipelineTask::ERROR;

    std::string task = URL.substr(firstSlashPos + 1);
    if (task.find("sync") != std::string::npos)
        return PipelineTask::PIPELINE_SYNC;
    else if (task.find("fileUpload") != std::string::npos)
        return PipelineTask::PIPELINE_FILE_UPLOAD;
    else if (task.find("check") != std::string::npos)
        return PipelineTask::PIPELINE_CHECK;
    else if (task.find("insert") != std::string::npos)
        return PipelineTask::PIPELINE_INSERT;
    else
        return PipelineTask::ERROR;
}

ProcResult PipelineReqProc::taskProcess(Task *fetchTask, ngsase_database &ngsaseDB) {
    PipelineTask taskType = assignTask(fetchTask->request->URL);
    Pipeline pipeline;
    ProcResult procResult;
    json jsonObject;
    std::string queryString;
    switch (taskType) {
        case PipelineTask::PIPELINE_SYNC: {

            std::vector<std::string> options;
            bool assignCheck = pipeline.assignFromBody(fetchTask->request->body, PipelineBehaviour::OPTIONS);
            PipelineBehaviour optionType;
            if (pipeline.tool.find("pipeline") != std::string::npos)
                optionType = PipelineBehaviour::PIPELINE_OPTION;
            else if (pipeline.type.find("gtf") != std::string::npos)
                optionType = PipelineBehaviour::GTF_OPTION;
            else if (pipeline.type.find("genomeIndex") != std::string::npos)
                optionType = PipelineBehaviour::GENOMEINDEX_OPTION;
            if (assignCheck) {
                queryString = pipeline.getQueryString(optionType);
                mysql_result *result = ngsaseDB.query(queryString);
                MYSQL_ROW row;
                MYSQL_FIELD *fields = mysql_fetch_fields(result->result);

                if (!result->errorMSG.empty()) {
                    std::string serverError = "Server Error: ";
                    serverError += result->errorMSG;
                    jsonObject = {
                            {"message", serverError}};
                } else {

                    if (result->affectRow > 0) {
                        while ((row = mysql_fetch_row(result->result)) != NULL) {
                            for (int i = 0; i < mysql_num_fields(result->result); i++) {
                                options.push_back(row[i]);
                                std::cout<<"pipeline_name: "<<row[i]<<std::endl;
                            }
                        }

                    }

                    if (options.size() > 0) {
                        json optionArray = json::array();
                        for (int i = 0; i < options.size(); i++) {
                            optionArray.push_back(options[i]);
                        }
                        jsonObject = {{"optionArray", optionArray}};
                    } else {
                        jsonObject = {
                                {"message", "empty option in pipeline"}};

                    }

                }
                delete result;

            } else {
                jsonObject = {"message", "Server Error: json deserialization fault in pipeline"};

            }
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();

            return procResult;
            break;
        }
        case PipelineTask::PIPELINE_FILE_UPLOAD: {

            //若檔案有錯誤直接return
            if (fetchTask->request->body.find("Error") != std::string::npos ||
                fetchTask->request->fileLocation.empty()) {
                if (fetchTask->request->fileLocation.empty())
                    jsonObject = {{"message", "Server Error: no file existed in specified directory"}};
                else
                    jsonObject = {{"message", fetchTask->request->body}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
                return procResult;
            }

            //執行移轉gtf至gtf資料庫
            std::filesystem::path currentPath = std::filesystem::current_path();
            std::filesystem::path dataPath(fetchTask->request->fileLocation);
            std::filesystem::path newFilePath;
            std::string email;
            std::string fileName = dataPath.filename().string();
            std::string responseMSG;
            int restHeaderLength = fetchTask->request->restHeaders.size();

            // 確認header內是不是有特殊header，若含有email 取出，作為後續directory path
            for (int i = 0; i < restHeaderLength; i++) {
                if (fetchTask->request->restHeaders[i].find("email") != std::string::npos) {
                    email = fetchTask->request->restHeaders[i].substr(7);
                    break;
                }
            }
            //如果header中沒有email的參數則直接return
            if (email.empty()) {
                jsonObject = {{"message",
                               "Server Error: There is a missing 'email' header in the request header section"}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
                return procResult;
            }

            //添加email至路徑中
            newFilePath = currentPath / "data" / "ngsase_data" / "gtf" / email;

            if (!std::filesystem::exists(newFilePath)) {
                try {
                    std::filesystem::create_directories(newFilePath);
                }
                catch (const std::exception &e) {
                    responseMSG = "Error: " + std::string(e.what());
                }
            }

            //確認fileName是否有.gtf
            if ((fileName.find(".gtf") != std::string::npos && responseMSG.find("Error") == std::string::npos) ||
                (fileName.find(".gff") != std::string::npos && responseMSG.find("Error") == std::string::npos)) {
                newFilePath = newFilePath / fileName;
                size_t firstDotPos = fileName.find(".");
                if (firstDotPos >= 0)
                    fileName = fileName.substr(0, firstDotPos);
                pipeline.email = email;
                pipeline.gtfName = fileName;
                pipeline.gtfDirectory = newFilePath.string();
                pipeline.dataIntegrity = true;
                //插入gtf至資料庫
                queryString = pipeline.getQueryString(PipelineBehaviour::GTF_INSERT);
                mysql_result *result = ngsaseDB.query(queryString);
                if (!result->errorMSG.empty()) {
                    responseMSG = "Error: " + result->errorMSG;

                    // 出現error移除原本的下載檔案
                    try {
                        std::filesystem::remove(dataPath);
                    } catch (const std::exception &e) {
                        std::cerr << "remove " << fileName << " Error: " << e.what() << std::endl;
                    }

                } else {
                    try {
                        std::filesystem::rename(dataPath, newFilePath);
                        responseMSG = "Upload succeed";
                        fetchTask->request->fileLocation = newFilePath.string();
                    } catch (const std::exception &e) {
                        responseMSG = "Error: " + std::string(e.what());
                    }
                }
                delete result;
            } else
                responseMSG = "Upload succeed";

            jsonObject = {{"message",  responseMSG},
                          {"filePath", fetchTask->request->fileLocation}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
            break;
        }
        case PipelineTask::PIPELINE_CHECK: {
            std::string responseMSG;
            bool assignCheck = pipeline.assignFromBody(fetchTask->request->body, PipelineBehaviour::EXSIT_CHECK);


            if (assignCheck)
                queryString = pipeline.getQueryString(PipelineBehaviour::EXSIT_CHECK);
            else
                responseMSG = "Server Error: json deserialization fault in pipeline";


            if (queryString.empty())
                responseMSG = "Server Error: queryString unqualified";
            else {
                mysql_result *result = ngsaseDB.query(queryString);

                int check = ((mysql_fetch_row(result->result))[0])[0] - '0';
                if (check)
                    responseMSG = "Pipeline Existed";
                else
                    responseMSG = "Pipeline Non-Existed";

                delete result;
            }


            jsonObject = {{"message", responseMSG}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();

            return procResult;
            break;
        }
        case PipelineTask::PIPELINE_INSERT: {
            std::string responseMSG;
            std::vector<std::string> pipelineQueryStringVector;
            json pipelineArray;
            try {
                pipelineArray = json::parse(fetchTask->request->body);
                for (const auto &elem: pipelineArray) {
                    std::string pipelineString = elem.dump();

                    bool assignCheck = pipeline.assignFromBody(pipelineString, PipelineBehaviour::INSERT);

                    if (assignCheck)
                        queryString = pipeline.getQueryString(PipelineBehaviour::INSERT);
                    if (queryString.empty()) {
                        responseMSG = "Server Error: json deserialization fault in pipeline";
                        break;
                    } else {
                        pipelineQueryStringVector.push_back(queryString);
                    }
                }

                int pipelineQueryStringVectorSize = pipelineQueryStringVector.size();
                // 會有上傳不成功需要把pipeline_rawdata刪除的情形，但之後再補，不然會有多個copy
                ngsaseDB.startTransaction();
                for (int i = 0; i < pipelineQueryStringVectorSize; i++) {
                    std::cout << pipelineQueryStringVector[i] << std::endl;
                    mysql_result *result = ngsaseDB.query(pipelineQueryStringVector[i]);
                    if (!result->errorMSG.empty()) {
                        responseMSG = result->errorMSG;
                        delete result;
                        break;
                    } else {
                        responseMSG = "Insertion succeed";
                        delete result;
                    }
                }
                if (responseMSG == "Insertion succeed")
                    ngsaseDB.commitTransaction();

            } catch (const json::parse_error &e) {
                // If parsing fails, catch the parse_error exception and handle it
                responseMSG = "Server Error: json deserialization fault in pipeline";
            }

            jsonObject = {{"message", responseMSG}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
            break;
        }

        case PipelineTask::ERROR: {
            jsonObject = {{"message", "Server Error:The process of request haven't been supported."}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
            break;
        }
    }
};

ProjectTask ProjectReqProc::assignTask(std::string &URL) {
    size_t firstSlashPos = URL.find('_');
    if (firstSlashPos == std::string::npos)
        return ProjectTask::ERROR;

    std::string task = URL.substr(firstSlashPos + 1);
    if (task.find("previous") != std::string::npos)
        return ProjectTask::PROJECT_PREVIOUS;
    else if (task.find("fileDownload") != std::string::npos)
        return ProjectTask::PROJECT_FILE_DOWNLOAD;
    else if (task.find("insert") != std::string::npos)
        return ProjectTask::PROJECT_INSERT;
    else if (task.find("execute") != std::string::npos)
        return ProjectTask::PROJECT_EXECUTE;
    else if (task.find("fileUpload") != std::string::npos)
        return ProjectTask::PROJECT_FILE_UPLOAD;
    else
        return ProjectTask::ERROR;
}

ProcResult ProjectReqProc::taskProcess(Task *fetchTask, ngsase_database &ngsaseDB) {
    ProjectTask processType = assignTask(fetchTask->request->URL);
    Project project;
    ProcResult procResult;
    std::string queryString;
    std::string responseMSG;
    json jsonObject;
    switch (processType) {
        case ProjectTask::PROJECT_FILE_DOWNLOAD: {

            bool projectAssignCheck = project.assignFromBody(fetchTask->request->body, ProjectBehaviour::FILE_DOWNLOAD);
            if (!projectAssignCheck) {
                jsonObject = {{"message", "Server Error: json deserialization fault in project"}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
                return procResult;
                break;
            }
            queryString = project.getQueryString(ProjectBehaviour::FILE_DOWNLOAD);
            mysql_result *result = ngsaseDB.query(queryString);
            std::string outputData;

            //這之後要和加入project那邊配合，當project跑完時要去update output_directory
            //取出filePath
            if (result->errorMSG.empty()) {
                MYSQL_ROW row;
                MYSQL_FIELD *fields = mysql_fetch_fields(result->result);
                while ((row = mysql_fetch_row(result->result)) != NULL) {
                    for (int i = 0; i < mysql_num_fields(result->result); i++) {
                        outputData = row[i];
                    }
                }
                //確認查詢結果是否有data
                if (!outputData.empty()) {
                    std::filesystem::path outputPath(outputData);
                    //確認檔案是否存在
                    if (std::filesystem::exists(outputPath)) {
                        //確認開啟檔案是否成功

                        int outputDataFD = open(outputData.c_str(), O_RDONLY);
                        if (outputDataFD > 0) {
                            procResult.responseType = BINARY;
                            procResult.filefd = outputDataFD;
                            procResult.fileName = outputPath.filename().string();
                        } else {
                            responseMSG = "Server Error: can't open file(" + outputData + ")";
                            jsonObject = {{"message", responseMSG}};
                            procResult.responseType = JSON;
                            procResult.responseBody = jsonObject.dump();

                        }


                    } else//檔案不存在
                    {

                        responseMSG = "Server Error: the file doesn't existed(" + outputPath.filename().string() + ")";
                        jsonObject = {{"message", responseMSG}};
                        procResult.responseType = JSON;
                        procResult.responseBody = jsonObject.dump();
                    }
                } else //查詢結果沒有任何紀錄
                {
                    responseMSG = "Server Error: There is no record in database";
                    jsonObject = {{"message", responseMSG}};
                    procResult.responseType = JSON;
                    procResult.responseBody = jsonObject.dump();
                }
            } else//查詢有error產生
            {

                responseMSG = "Server Error: " + result->errorMSG;
                jsonObject = {{"message", responseMSG}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
            }


            delete result;
            return procResult;


            break;
        }
        case ProjectTask::PROJECT_PREVIOUS: {
            json previousProjectToClientArray = json::array();
            bool projectAssignCheck = project.assignFromBody(fetchTask->request->body, ProjectBehaviour::GET_PREVIOUS);
            if (!projectAssignCheck) {
                json jsonObject = {{"message", "Server Error: json deserialization fault in project"}};
                procResult.responseBody = jsonObject.dump();
                procResult.responseType = JSON;
                return procResult;
                break;
            }

            std::map<std::string, bool> projectMap;
            queryString = project.getQueryString(ProjectBehaviour::GET_PREVIOUS);
            mysql_result *result = ngsaseDB.query(queryString);
            MYSQL_ROW row;
            MYSQL_FIELD *fields = mysql_fetch_fields(result->result);


            if (result->errorMSG.empty()) {
                while ((row = mysql_fetch_row(result->result)) != NULL) {
                    Project previousProjectToClient;
                    // Process each row
                    for (int i = 0; i < mysql_num_fields(result->result); i++) {
                        if (!strcasecmp(fields[i].name, "uuid")) {
                            previousProjectToClient.uuid = row[i];
                        } else if (!strcasecmp(fields[i].name, "project_name")) {
                            previousProjectToClient.projectName = row[i];
                            projectMap.insert(std::pair<std::string, bool>(previousProjectToClient.projectName, true));
                        } else if (!strcasecmp(fields[i].name, "pipeline_name"))
                            previousProjectToClient.pipelineName = row[i];
                        else if (!strcasecmp(fields[i].name, "paired_end"))
                            previousProjectToClient.pairedEnd = ((strcasecmp(row[i], "0")) == 0) ? "false" : "true";
                        else if (!strcasecmp(fields[i].name, "input_directory"))
                            previousProjectToClient.inputDirectory = row[i];
                        else if (!strcasecmp(fields[i].name, "output_directory"))
                            previousProjectToClient.outputDirectory = row[i];
                        else if (!strcasecmp(fields[i].name, "project_date"))
                            previousProjectToClient.projectDate = row[i];
                        else if (!strcasecmp(fields[i].name, "finished"))
                            previousProjectToClient.finished = ((strcasecmp(row[i], "0")) == 0) ? "false" : "true";
                        else if (!strcasecmp(fields[i].name, "email"))
                            previousProjectToClient.email = row[i];
                    }
                    previousProjectToClientArray.push_back({{"uuid",             previousProjectToClient.uuid},
                                                            {"project_name",     previousProjectToClient.projectName},
                                                            {"paired_end",       previousProjectToClient.pairedEnd},
                                                            {"input_directory",  previousProjectToClient.inputDirectory},
                                                            {"output_directory", previousProjectToClient.outputDirectory},
                                                            {"project_date",     previousProjectToClient.projectDate},
                                                            {"finished",         previousProjectToClient.finished},
                                                            {"email",            previousProjectToClient.email}});
                };
                procResult.responseBody = previousProjectToClientArray.dump();
                procResult.responseType = JSON;

            } else {
                std::string serverError = "Server Error: " + result->errorMSG;
                json jsonObject = {{"message", serverError}};
                procResult.responseBody = jsonObject.dump();
                procResult.responseType = JSON;
                delete result;
                return procResult;
                break;
            }

            std::filesystem::path currentPath = std::filesystem::current_path().string();

            std::filesystem::path rawDataBasePath = (currentPath / "data" / "ngsase_data" / "project_rawdata" /
                                                     project.email);
            std::filesystem::path resultDataBasePath = (currentPath / "data" / "ngsase_data" / "project_result" /
                                                        project.email);
            std::error_code ec;
            if (std::filesystem::exists(rawDataBasePath) && std::filesystem::exists(resultDataBasePath)) {
                for (auto &entry: std::filesystem::directory_iterator(rawDataBasePath)) {
                    if (std::filesystem::is_directory(entry) &&
                        projectMap.find(entry.path().filename().string()) == projectMap.end() && !ec) {
                        std::filesystem::remove_all(entry.path(), ec);
                    }
                }

                for (auto &entry: std::filesystem::directory_iterator(resultDataBasePath)) {
                    if (std::filesystem::is_directory(entry) &&
                        projectMap.find(entry.path().filename().string()) == projectMap.end() && !ec) {
                        std::filesystem::remove_all(entry.path(), ec);
                    }
                    if (std::filesystem::is_regular_file(entry) &&
                        projectMap.find(entry.path().filename().string()) == projectMap.end() && !ec) {
                        std::filesystem::remove(entry.path(), ec);
                    }


                }

            }
            if (ec) {
                std::string serverError = "Server Error: " + ec.message();
                json jsonObject = {{"message", serverError}};
                procResult.responseBody = jsonObject.dump();
                procResult.responseType = JSON;
                delete result;
                return procResult;
                break;


            }


            delete result;
            return procResult;

            break;
        }
        case ProjectTask::PROJECT_INSERT: {
            bool assignCheck = project.assignFromBody(fetchTask->request->body, ProjectBehaviour::INSERT);
            if (!assignCheck) {
                jsonObject = {{"message", "Server Error: json deserialization fault in project"}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
                return procResult;
            }
            //如果有人篡改javascirpt則會Error
            std::string rawDataBasePath =
                    "/data/ngsase_data/project_rawdata/" + project.email + "/" + project.projectName;
            std::string resultDataBasePath =
                    "/data/ngsase_data/project_result/" + project.email + "/" + project.projectName;
            if (project.inputDirectory != rawDataBasePath || project.outputDirectory != resultDataBasePath) {
                jsonObject = {{"message", "Server Error: tamper with client javascript"}};
                procResult.responseType = JSON;
                procResult.responseBody = jsonObject.dump();
                return procResult;

            }
            queryString = project.getQueryString(ProjectBehaviour::INSERT);
            mysql_result *result = ngsaseDB.query(queryString);
            if (result->errorMSG.empty()) {
                jsonObject = {{"message", "project construction succeed"}};
            } else {
                responseMSG = "Server Error: " + result->errorMSG;
                jsonObject = {{"message", responseMSG}};
                std::filesystem::path inputDir(project.inputDirectory);
                bool fileDelCheck = removeProjectRawData(inputDir);
            }
            delete result;
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;


        }
        case ProjectTask::PROJECT_EXECUTE: {
            procResult.responseType = NONE;
            procResult.responseBody = "execution forwarding";
            json procForward;
            std::string procForwardString;
            bool projectAssignCheck = project.assignFromBody(fetchTask->request->body, ProjectBehaviour::INSERT);
            if (projectAssignCheck) {
                queryString = project.getQueryString(ProjectBehaviour::PIPELINE_QUERY);
                std::filesystem::path currentDir = std::filesystem::current_path();
                std::filesystem::path ngsaseBaseDir = currentDir / "data" / "ngsase_data";
                std::string outputDirectory = currentDir.string() + project.outputDirectory;
                std::string inputDirectory = currentDir.string() + project.inputDirectory;
                int pairEndCheck = 0;
                if (project.pairedEnd == "true")
                    pairEndCheck = 1;
                mysql_result *result = ngsaseDB.query(queryString);
                std::string socketString;
                if (ngsaseDB.unix_socket == nullptr)
                    socketString = "nullptr";
                else
                    socketString = std::string(ngsaseDB.unixSocket);
                if (result->errorMSG.empty()) {
                    MYSQL_ROW row = mysql_fetch_row(result->result);
                    std::string jsonString = row[0];




                    procForward = {
                            {"output_directory", outputDirectory},
                            {"input_directory",  inputDirectory},
                            {"project_name",     project.projectName},
                            {"email",            project.email},
                            {"uuid",             project.uuid},
                            {"pipeline_name",    project.pipelineName},
                            {"paired_end",       pairEndCheck},
                            {"settings",         jsonString},
                            {"kqueueFD",         fetchTask->kqueuefd},
                            {"clientFD",         fetchTask->filefd},
                            {"prevProcError",    ""},
                            {"ngsase_baseDir",   ngsaseBaseDir.string()},
                            {"mysql_host",       ngsaseDB.host},
                            {"mysql_username",   ngsaseDB.username},
                            {"mysql_password",   ngsaseDB.password},
                            {"mysql_socket",     socketString},
                            {"mysql_port",       ngsaseDB.port}
                    };


                } else {
                    procForward = {
                            {"output_directory", outputDirectory},
                            {"input_directory",  inputDirectory},
                            {"project_name",     project.projectName},
                            {"email",            project.email},
                            {"uuid",             project.uuid},
                            {"pipeline_name",    project.pipelineName},
                            {"paired_end",       pairEndCheck},
                            {"settings",         ""},
                            {"kqueueFD",         fetchTask->kqueuefd},
                            {"clientFD",         fetchTask->filefd},
                            {"prevProcError",    ("Error: " + result->errorMSG)},
                            {"ngsase_baseDir",   ngsaseBaseDir.string()},
                            {"mysql_host",       ngsaseDB.host},
                            {"mysql_username",   ngsaseDB.username},
                            {"mysql_password",   ngsaseDB.password},
                            {"mysql_socket",     socketString},
                            {"mysql_port",       ngsaseDB.port}
                    };

                }
                delete result;
                procForwardString = procForward.dump();
                std::cout << procForwardString << std::endl;
                std::cout << std::flush;
                int exitStatus;

                pid_t pid = fork();

                if (pid == 0) {
                    std::cout << "Child Process: " << getpid() << std::endl;

                    execl(fetchTask->executePath.c_str(),
                          procForwardString.c_str(),
                          nullptr);

                    std::cerr << "error to execl" << std::endl;

                } else if (pid < 0) {
                    std::cerr << "Error: fork failed" << std::endl;
                } else {
                    wait(&exitStatus);
                    if (WIFEXITED(exitStatus))
                        std::cout << "Child process exited with status: " << WEXITSTATUS(exitStatus) << std::endl;
                    else
                        std::cout << "Child process exited abnormally" << std::endl;
                }


            } else {
                //除非使用者亂改不然不會發生
                procResult.responseType = JSON;
                jsonObject = {{"message", "Server Error: json deserialization fault in project"}};
                procResult.responseBody = jsonObject.dump();
            }

            return procResult;
            break;
        }
        case ProjectTask::PROJECT_FILE_UPLOAD: {
            if (fetchTask->request->body.find("Error") != std::string::npos ||
                fetchTask->request->fileLocation.empty()) {
                jsonObject = {{"message", "Server Error: no file existed in specified directory"}};
                return procResult;
            } else {
                jsonObject = {{"message", "Upload succeed"}};
            }
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
            break;
        }
        case ProjectTask::ERROR: {
            jsonObject = {{"message", "Server Error:The process of request haven't been supported."}};
            procResult.responseType = JSON;
            procResult.responseBody = jsonObject.dump();
            return procResult;
            break;
        }
    }

}

bool ProjectReqProc::removeProjectRawData(std::filesystem::path inputDir) {
    if (std::filesystem::exists(inputDir)) {
        try {
            for (const auto &file: std::filesystem::directory_iterator(inputDir))
                std::filesystem::remove(file.path());
        }
        catch (const std::exception &e) {
            std::cerr << "Error removing file: " << e.what() << std::endl;
            return false;
        }

    }
    return true;
}
