#include <iostream>
#include "api.h"

int main(int argc, char* argv[])
{
    char hostname[] = "localhost";
    server_rec *s = modsecInit();
    s->server_hostname = hostname;

    modsecStartConfig();
    modsecFinalizeConfig();
    modsecInitProcess();

    directory_config *config = modsecGetDefaultConfig();
    const char *err = modsecProcessConfig(config, argv[1], nullptr);
    if (err) {
        std::cout << err << std::endl;
        return 1;
    }

    std::cout << "ModSecurity configuration successfully loaded." << std::endl;
    return 0;
}
