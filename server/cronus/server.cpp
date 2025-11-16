
#include "cronus/server.h"

namespace cronus
{

Server::Server()=default;
Server::~Server()=default;

void Server::run()
{
    m_cronus.run();
}

void Server::stop()
{
    m_cronus.stop();
}

} // namespace cronus
