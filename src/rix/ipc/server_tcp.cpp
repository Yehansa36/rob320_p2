#include "rix/ipc/server_tcp.hpp"
#include "rix/ipc/interfaces/server.hpp"
#include <memory>
#include <unordered_set>



namespace rix {
namespace ipc {

/**< TODO */
ServerTCP::ServerTCP(const Endpoint &ep, size_t backlog)
    : socket(AF_INET, SOCK_STREAM), connections() {

    if (!socket.bind(ep)) return;

    if (!socket.listen(backlog)) return;
    }


ServerTCP::ServerTCP() : socket(), connections() {}

ServerTCP::ServerTCP(const ServerTCP &other) : socket(other.socket), connections(other.connections) {}

ServerTCP &ServerTCP::operator=(const ServerTCP &other) {
    if (this != &other) {
        socket = other.socket;
        connections = other.connections;
    }
    return *this;
}

ServerTCP::~ServerTCP() {}

/**< TODO */
void ServerTCP::close(const std::weak_ptr<interfaces::Connection> &connection) {
    auto shared = connection.lock();
    if (shared) return;

    connections.erase(shared);
}

/**< TODO */
bool ServerTCP::ok() const { 
    
    return socket.ok(); 
}

/**< TODO */
Endpoint ServerTCP::local_endpoint() const { 
    Endpoint ep;
    socket.getsockname(ep);
    return ep;
    
}

/**< TODO */
bool ServerTCP::accept(std::weak_ptr<interfaces::Connection> &connection) { 

  // Socket client_socket;

    //if (!socket.accept(client_socket)) {
       // connection.reset();
       // return false;
   // }

    // Cannot create ConnectionTCP here; leave weak_ptr empty
    //connection.reset();  

    //return true;

       Socket client_socket;

    if (!socket.accept(client_socket)) {
        return false;
    }

    // Direct new — NOT make_shared
    std::shared_ptr<ConnectionTCP> conn(
        new ConnectionTCP(client_socket)
    );

    connections.insert(conn);

    connection = conn;

    return true;
}

/**< TODO */
bool ServerTCP::wait_for_accept(rix::util::Duration duration) const { 
    
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket.fd(), &rfds);

    timeval tv;
    tv.tv_sec = duration.to_seconds();
    tv.tv_usec = duration.to_microseconds() % 1000000;

    int ret = select(socket.fd() + 1, &rfds, nullptr, nullptr, &tv);

    return ret > 0 && FD_ISSET(socket.fd(), &rfds);
    
}

/**< TODO */
void ServerTCP::set_nonblocking(bool status) {
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return;

    if (status)
        fcntl(socket.fd(), F_SETFL, flags | O_NONBLOCK);
    else
        fcntl(socket.fd(), F_SETFL, flags & ~O_NONBLOCK);
}

/**< TODO */
bool ServerTCP::is_nonblocking() const { 
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return false;

    return (flags & O_NONBLOCK) != 0; 
}

}  // namespace ipc
}  // namespace rix