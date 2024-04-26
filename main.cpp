#include "reactor.h"

int main(int argc, char *argv[]) {

    //Assign server parameter to NGSaseServer constructor
    NGSaseParameter ngsaseParameter;
    if (!ngsaseParameter.assignUserInput(argc, argv))
        return 0;
    std::cout << "argument check passed" << std::endl;

    NGSaseServer ngsaseServer(ngsaseParameter.userAddress,
                              ngsaseParameter.userPort,
                              ngsaseParameter.userBacklog,
                              ngsaseParameter.userMainReactorMaxWorkerNum,
                              ngsaseParameter.userMainReactorMinWorkerNum,
                              ngsaseParameter.userSubReactorMaxWorkerNum,
                              ngsaseParameter.userSubReactorMinWorkerNum,
                              ngsaseParameter.userMySQLHost,
                              ngsaseParameter.userMySQLAccount,
                              ngsaseParameter.userMySQLPassword);

    ngsaseServer.start();




    return 0;
}
