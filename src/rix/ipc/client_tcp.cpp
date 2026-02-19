#include "rix/ipc/client_tcp.hpp"

namespace rix {
namespace ipc {

/**< TODO */
ClientTCP::ClientTCP() : socket(AF_INET, SOCK_STREAM) {}

ClientTCP::ClientTCP(const ClientTCP &other) : socket(other.socket) {}

ClientTCP &ClientTCP::operator=(const ClientTCP &other) {
    if (this != &other) {
        socket = other.socket;
    }
    return *this;
}

ClientTCP::~ClientTCP() {}

/**< TODO */
bool ClientTCP::connect(const Endpoint &endpoint) { 
    
    return socket.connect(endpoint); 
}

/**< TODO */
ssize_t ClientTCP::write(const uint8_t *buffer, size_t len) const { 
    return socket.write(buffer, len); 
}

/**< TODO */
ssize_t ClientTCP::read(uint8_t *buffer, size_t len) const { 
    return socket.read(buffer, len); 
}

/**< TODO */
Endpoint ClientTCP::remote_endpoint() const { 
    Endpoint ep;
    socket.getpeername(ep);
    return ep; 
}

/**< TODO */
Endpoint ClientTCP::local_endpoint() const {
    Endpoint ep;
    socket.getsockname(ep);

    return ep; 
}

/**< TODO */
bool ClientTCP::ok() const { 
    return socket.ok(); 
}

/**< TODO */
bool ClientTCP::wait_for_connect(const rix::util::Duration &duration) const { 
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(socket.fd(), &wfds);

    timeval tv;
    tv.tv_sec = duration.to_seconds();
    tv.tv_usec = duration.to_microseconds() % 1000000;

    int ret = select(socket.fd() + 1, nullptr, &wfds, nullptr, &tv);

    return ret > 0 && FD_ISSET(socket.fd(), &wfds); 
}

/**< TODO */
bool ClientTCP::wait_for_writable(const rix::util::Duration &duration) const { 
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(socket.fd(), &wfds);

    timeval tv;
    tv.tv_sec = duration.to_seconds();
    tv.tv_usec = duration.to_microseconds() % 1000000;

    int ret = select(socket.fd() + 1, nullptr, &wfds, nullptr, &tv);

    return ret > 0 && FD_ISSET(socket.fd(), &wfds); 
}

/**< TODO */
bool ClientTCP::wait_for_readable(const rix::util::Duration &duration) const { 
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
void ClientTCP::set_nonblocking(bool status) {
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return;

    if (status)
        fcntl(socket.fd(), F_SETFL, flags | O_NONBLOCK);
    else
        fcntl(socket.fd(), F_SETFL, flags & ~O_NONBLOCK);
}

/**< TODO */
bool ClientTCP::is_nonblocking() const { 
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return false;
    return (flags & O_NONBLOCK) != 0; 
}

void ClientTCP::reset() { socket = Socket(AF_INET, SOCK_STREAM); }

}  // namespace ipc
}  // namespace rix