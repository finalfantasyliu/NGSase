#ifndef NGSASE_JSON_H
#define NGSASE_JSON_H
#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>
using json = nlohmann::json;

enum class AccountBehaviour {
    LOGIN,
    REGISTER
};

enum class PipelineBehaviour {
    EXSIT_CHECK,
    GTF_INSERT,
    INSERT,
    OPTIONS,
    PIPELINE_OPTION,
    GTF_OPTION,
    GENOMEINDEX_OPTION,

};
enum class ProjectBehaviour {
    GET_PREVIOUS,
    INSERT,
    FILE_DOWNLOAD,
    PIPELINE_QUERY

};

class LoginRegister {
   public:
    bool assignFromBody(std::string &body);
    std::string getQueryString(AccountBehaviour behaviour);

   public:
    std::string email;
    std::string password;
    bool dataIntegrity = false;
};

class Pipeline {
   public:
    Pipeline() = default;
    Pipeline(Pipeline &&other) noexcept {
        pipelineName = std::move(other.pipelineName);
        tool = std::move(other.tool);
        setting = std::move(other.setting);
        step = std::move(other.step);
        email = std::move(other.email);
        dataIntegrity = other.dataIntegrity;
    }

   public:
    bool assignFromBody(std::string &body, PipelineBehaviour behaviour);
    std::string getQueryString(PipelineBehaviour behaviour);

   public:
    std::string pipelineName;
    std::string tool;
    std::string type;
    std::string setting;
    std::string step;
    std::string email;
    std::string gtfName;
    std::string gtfDirectory;
    bool dataIntegrity = false;
};

class Project {
   public:
    bool assignFromBody(std::string &body, ProjectBehaviour behaviour);
    std::string getQueryString(ProjectBehaviour behaviour);

   public:
    std::string uuid;
    std::string projectName;
    std::string pipelineName;
    std::string pairedEnd;
    std::string inputDirectory;
    std::string outputDirectory;
    std::string projectDate;
    std::string finished;
    std::string email;

    bool dataIntegrity = false;
};
#endif