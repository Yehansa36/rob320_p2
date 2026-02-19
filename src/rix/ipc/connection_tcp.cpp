#include "rix/ipc/connection_tcp.hpp"

namespace rix {
namespace ipc {



ConnectionTCP::ConnectionTCP(const Socket &socket) : socket(socket) {}

ConnectionTCP::ConnectionTCP() : socket() {}

ConnectionTCP::ConnectionTCP(const ConnectionTCP &other) : socket(other.socket) {}

ConnectionTCP &ConnectionTCP::operator=(const ConnectionTCP &other) {
    if (this != &other) {
        socket = other.socket;
    }
    return *this;
}

ConnectionTCP::~ConnectionTCP() {}

/**< TODO */
ssize_t ConnectionTCP::write(const uint8_t *buffer, size_t len) const { 
    
    return socket.write(buffer, len); 
}

/**< TODO */
ssize_t ConnectionTCP::read(uint8_t *buffer, size_t len) const { 
    
    return socket.read(buffer, len); 
}

/**< TODO */
Endpoint ConnectionTCP::remote_endpoint() const { 
    Endpoint ep;
    socket.getpeername(ep);
    return ep; 
}

/**< TODO */
Endpoint ConnectionTCP::local_endpoint() const { 
    Endpoint ep;
    socket.getsockname(ep);
    return ep; 
}

/**< TODO */
bool ConnectionTCP::ok() const { 

    return socket.ok(); 
}

/**< TODO */
bool ConnectionTCP::wait_for_writable(const rix::util::Duration &duration) const { 
    
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(socket.fd(), &wfds);

    timeval tv;
    tv.tv_sec = duration.to_seconds();
    tv.tv_usec = duration.to_microseconds();

    int ret = select(socket.fd() + 1, nullptr, &wfds, nullptr, &tv);

    return ret > 0 && FD_ISSET(socket.fd(), &wfds); 
}

/**< TODO */
bool ConnectionTCP::wait_for_readable(const rix::util::Duration &duration) const { 
    
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket.fd(), &rfds);

    timeval tv;
    tv.tv_sec = duration.to_seconds();
    tv.tv_usec = duration.to_microseconds();

    int ret = select(socket.fd() + 1, &rfds, nullptr, nullptr, &tv);


    return ret > 0 && FD_ISSET(socket.fd(), &rfds); 
}

/**< TODO */
void ConnectionTCP::set_nonblocking(bool status) {
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return;

    if (status)
        fcntl(socket.fd(), F_SETFL, flags | O_NONBLOCK);

    else 
        fcntl(socket.fd(), F_SETFL, flags & ~O_NONBLOCK);

}

/**< TODO */
bool ConnectionTCP::is_nonblocking() const { 
    int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags < 0) return false; 

    return (flags & O_NONBLOCK) != 0;
}

}  // namespace ipc
}  // namespace rix