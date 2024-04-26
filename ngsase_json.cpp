#include "ngsase_json.h"

bool LoginRegister::assignFromBody(std::string &body) {
    try {
        auto json = json::parse(body);
        if (json.contains("email"))
            email = json.value("email", "");
        if (json.contains("password"))
            password = json.value("password", "");
        if (!email.empty() && !password.empty()) {
            dataIntegrity = true;
            return true;
        } else {
            std::cout << "The property email && password doesn't exist in JSON object" << std::endl;
            return false;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
};

std::string LoginRegister::getQueryString(AccountBehaviour behaviour) {
    if (dataIntegrity) {
        std::string temp;
        switch (behaviour) {
            case AccountBehaviour::LOGIN: {
                temp += "SELECT * FROM users WHERE email=";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += " AND password=";
                temp += "\"";
                temp += password;
                temp += "\"";
                temp += ";";
                break;
            }
            case AccountBehaviour::REGISTER: {
                temp += "INSERT INTO users(email,password) VALUES(";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ",";
                temp += "\"";
                temp += password;
                temp += "\"";
                temp += ");";
                break;
            }
        }

        return temp;
    } else
        return "";
}

bool Pipeline::assignFromBody(std::string &body, PipelineBehaviour behaviour) {
    try {
        auto json = json::parse(body);
        if (json.contains("pipeline_name"))
            pipelineName = json.value("pipeline_name", "");
        if (json.contains("tool"))
            tool = json.value("tool", "");
        if (json.contains("type"))
            type = json.value("type", "DEFAULT");
        if (json.contains("setting"))
            setting = json["setting"].dump();
        std::cout << setting << std::endl;
        if (json.contains("step")) {
            if (json["step"].is_number_integer()) {
                step = std::to_string((json["step"].get<int>()));
            }
        }
        if (json.contains("email"))
            email = json.value("email", "");
        switch (behaviour) {
            case PipelineBehaviour::GTF_INSERT: {
                std::cout << "Can't assign from body" << std::endl;
                return false;
                break;
            }
            case PipelineBehaviour::OPTIONS: {
                if (!email.empty() && !tool.empty() && type != "DEFAULT") {
                    dataIntegrity = true;
                    return true;
                } else {
                    std::cout << "The property pipeline_name && email doesn't exist in JSON object" << std::endl;
                    return false;
                }
                break;
            }
            case PipelineBehaviour::EXSIT_CHECK: {
                if (!email.empty() && !pipelineName.empty()) {
                    dataIntegrity = true;
                    return true;
                } else {
                    std::cout << "The property pipeline_name && email doesn't exist in JSON object" << std::endl;
                    return false;
                }
                break;
            }
            case PipelineBehaviour::INSERT: {
                if (pipelineName.empty() || tool.empty() || setting.empty() || step.empty() || email.empty()) {
                    std::cout << "The property doesn't exist in JSON object" << std::endl;
                    return false;
                } else {
                    dataIntegrity = true;
                    return true;
                }
                break;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}

std::string Pipeline::getQueryString(PipelineBehaviour behaviour) {
    if (dataIntegrity) {
        std::string temp;
        switch (behaviour) {
            case PipelineBehaviour::PIPELINE_OPTION: {
                temp += "SELECT DISTINCT pipeline_name FROM pipelines WHERE email=";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ";";
                break;
            }
            case PipelineBehaviour::GTF_OPTION: {
                temp += "SELECT gtf_name FROM gtfs WHERE email=";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ";";
                break;
            }
            case PipelineBehaviour::GENOMEINDEX_OPTION: {
                temp += "SELECT index_name FROM genomeIndexs WHERE tool=";
                temp += "\"";
                temp += tool;
                temp += "\" AND email=";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ";";
                break;
            }
            case PipelineBehaviour::GTF_INSERT: {
                temp += "INSERT INTO gtfs(gtf_name,gtf_directory,email)VALUES(";
                temp += "\"";
                temp += gtfName;
                temp += "\"";
                temp += ", ";
                temp += "\"";
                temp += gtfDirectory;
                temp += "\"";
                temp += ", ";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ");";
                break;
            }
            case PipelineBehaviour::EXSIT_CHECK: {
                temp += "SELECT EXISTS (";
                temp += "SELECT 1 FROM pipelines WHERE pipeline_name=";
                temp += "\"";
                temp += pipelineName;
                temp += "\"";
                temp += " AND email=";
                temp += "\"";
                temp += email;
                temp += "\"";
                temp += ")";
                temp += ";";
                break;
            }
            case PipelineBehaviour::INSERT: {
                temp += "INSERT INTO pipelines(pipeline_name,tool,setting,step,email) VALUES('";
                temp += pipelineName;
                temp += "','";
                temp += tool;
                temp += "','";
                temp += setting;
                temp += "','";
                temp += step;
                temp += "','";
                temp += email;
                temp += "');";
                break;
            }
        }
        return temp;
    } else
        return "";
}

bool Project::assignFromBody(std::string &body, ProjectBehaviour behaviour) {
    try {
        auto json = json::parse(body);
        if (json.contains("uuid"))
            uuid = json.value("uuid", "");
        if (json.contains("project_name"))
            projectName = json.value("project_name", "");
        if (json.contains("pipeline_name"))
            pipelineName = json.value("pipeline_name", "");
        if (json.contains("paired_end"))
            pairedEnd = json.value("paired_end", "");
        if (json.contains("input_directory"))
            inputDirectory = json.value("input_directory", "");
        if (json.contains("output_directory"))
            outputDirectory = json.value("output_directory", "");
        if (json.contains("finished"))
            finished = json.value("finished", "");
        if (json.contains("email"))
            email = json.value("email", "");
        if (json.contains("pipeline_name"))
            pipelineName = json.value("pipeline_name", "");

        switch (behaviour) {
            case ProjectBehaviour::GET_PREVIOUS: {
                if (!email.empty()) {
                    dataIntegrity = true;
                    return true;
                } else {
                    std::cout << "The property email doesn't exist in JSON object" << std::endl;
                    return false;
                }
                break;
            }
            case ProjectBehaviour::INSERT: {
                if (uuid.empty() || projectName.empty() || pipelineName.empty() || pairedEnd.empty() ||
                    inputDirectory.empty() || outputDirectory.empty() || finished.empty() || email.empty()) {
                    std::cout << "The property doesn't exist in JSON object" << std::endl;
                    return false;
                } else {
                    dataIntegrity = true;
                    return true;
                }
                break;
            }
            case ProjectBehaviour::FILE_DOWNLOAD: {
                if (uuid.empty() || projectName.empty() || email.empty()) {
                    std::cout << "The property doesn't exist in JSON object" << std::endl;
                    return false;
                } else {
                    dataIntegrity = true;
                    return true;
                }
                break;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}

std::string Project::getQueryString(ProjectBehaviour behaviour) {
    std::string temp;
    if (!dataIntegrity)
        return temp;
    switch (behaviour) {
        case ProjectBehaviour::GET_PREVIOUS: {
            temp =
                    "SELECT "
                    "    uuid, "
                    "    project_name, "
                    "    pipeline_name, "
                    "    paired_end, "
                    "    input_directory, "
                    "    output_directory, "
                    "    DATE_FORMAT(project_date, '%Y-%m-%d') AS project_date, "
                    "    finished, "
                    "    email "
                    "FROM "
                    "    projects "
                    "WHERE "
                    "    project_date >= DATE_SUB(NOW(), INTERVAL 7 DAY) "
                    "AND email=";
            temp += "\"";
            temp += email;
            temp += "\"";
            temp += ";";
            return temp;
            break;
        }
        case ProjectBehaviour::INSERT: {
            std::filesystem::path currentPath = std::filesystem::current_path();
            inputDirectory = (currentPath / inputDirectory).string();
            outputDirectory = (currentPath / outputDirectory).string();

            temp += "INSERT INTO projects (uuid,project_name,pipeline_name,paired_end,input_directory,output_directory,finished,email) VALUES (";
            temp += "\"";
            temp += uuid;
            temp += "\", ";
            temp += "\"";
            temp += projectName;
            temp += "\", ";
            temp += "\"";
            temp += pipelineName;
            temp += "\", ";
            temp += pairedEnd;
            temp += ", ";
            temp += "\"";
            temp += inputDirectory;
            temp += "\", ";
            temp += "\"";
            temp += outputDirectory;
            temp += "\", ";
            temp += finished;
            temp += ", ";
            temp += "\"";
            temp += email;
            temp += "\")";
            temp += ";";


            return temp;
            break;
        }
        case ProjectBehaviour::FILE_DOWNLOAD: {
            temp = "SELECT output_directory FROM projects WHERE uuid=";
            temp += "\"";
            temp += uuid;
            temp += "\"";
            temp += " AND project_name=";
            temp += "\"";
            temp += projectName;
            temp += "\"";
            temp += " AND email=";
            temp += "\"";
            temp += email;
            temp += "\"";
            temp += ";";

            return temp;
            break;
        }
        case ProjectBehaviour::PIPELINE_QUERY: {
            temp = R"(
            SELECT JSON_ARRAYAGG(
                    JSON_OBJECT(
                            'tool', tool,
                            'setting', setting,
                            'step', step
                    )
            ) AS result FROM pipelines WHERE email =)";
            temp += "\"";
            temp += email;
            temp += "\" AND pipeline_name=\"";
            temp += pipelineName;
            temp += "\"  ORDER BY step;";
            return temp;
            break;
        }
    }
}
