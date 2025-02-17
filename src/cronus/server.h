#ifndef _cronus_server_h_
#define _cronus_server_h_

#include "cronus/cronus.h"

namespace cronus
{

class Server
{
public:
    Server();
    ~Server();

    void run();
    void stop();
private:
    Cronus m_cronus;
};

}//namespace cronus

#endif// _cronus_server_h_