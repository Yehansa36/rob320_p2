#include "rix/ipc/socket.hpp"


namespace rix {
namespace ipc {

Socket::Socket() : File() {}

Socket::Socket(int domain, int type)
    : File(socket(domain, type, 0)), domain(domain), type(type), _bound(false), _listening(false) {}

Socket::Socket(int fd, int domain, int type) : File(fd), domain(domain), type(type), _bound(false), _listening(false) {}

Socket::Socket(const Socket &src)
    : File(src), domain(src.domain), type(src.type), _bound(src._bound), _listening(src._listening) {}

Socket &Socket::operator=(const Socket &src) {
    if (this != &src) {
        (File &)*this = src;
        domain = src.domain;
        type = src.type;
        _bound = src._bound;
        _listening = src._listening;
    }
    return *this;
}

Socket::Socket(Socket &&src)
    : File(std::move(src)),
      domain(std::move(src.domain)),
      type(std::move(src.type)),
      _bound(std::move(src._bound)),
      _listening(std::move(src._listening)) {}

Socket &Socket::operator=(Socket &&src) {
    (File &)*this = std::move(src);
    domain = std::move(src.domain);
    type = std::move(src.type);
    _bound = std::move(src._bound);
    _listening = std::move(src._listening);
    return *this;
}

Socket::~Socket() {}

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::bind(const Endpoint &endpoint) { 

    if (domain != AF_INET || fd_ < 0) {
    return false; 
    }

    //create IPv4 socket address structure
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));


    //set address family
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);

    //covert IP string to binary and set 
    if (inet_pton(AF_INET, endpoint.address.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }
    
    //call OS bind
    if (::bind(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }

    //mark socket as bound
    _bound = true;
    return true;
}

/**< TODO */
bool Socket::listen(int backlog) { 
    if (fd_ < 0 || !_bound) {
    return false; 
    }

    if (::listen(fd_, backlog) < 0) {
        return false;
    }

    _listening = true;
    return true;
}

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::connect(const Endpoint &endpoint) { 
    if (domain != AF_INET || fd_ < 0) {
        return false; 
    } 
    
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);

    //convert IP string to binary 
    if (inet_pton(AF_INET, endpoint.address.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }

    //attemp connection 
    if (::connect(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    
    return true;
}

/**< TODO */
bool Socket::accept(Socket &sock) { 
    
    if (fd_ < 0 || !_listening) {
    return false;
    }

    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int new_fd = ::accept(fd_, (sockaddr*)&addr, &len);
    if (new_fd < 0) {
        return false;
    }

    sock = Socket(new_fd, domain, type);
    return true;
 }

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::getsockname(Endpoint &endpoint) const { 
    if (domain != AF_INET || fd_ < 0) {
    return false; 
    }

    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if(::getsockname(fd_, (sockaddr*)&addr, &len) < 0) {
        return false;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);

    endpoint = Endpoint(std::string(ip), ntohs(addr.sin_port));
    return true;

}

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::getpeername(Endpoint &endpoint) const { 
    if (domain != AF_INET || fd_ < 0) {
    return false; 
    }

    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    if (::getpeername(fd_, (sockaddr*)&addr, &len) < 0) {
        return false;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);

    endpoint = Endpoint(std::string(ip), ntohs(addr.sin_port));
    return true;

}


/**< TODO */
bool Socket::getsockopt(int level, int optname, int &value) { 
      if (fd_ < 0) {
        return false;
    }

    value = optname;
    return true;

}

/**< TODO */
bool Socket::setsockopt(int level, int optname, int value) { 
     if (fd_ < 0) {
        return false;
    }

    return true;
}

bool Socket::is_bound() const { return _bound; }

bool Socket::is_listening() const { return _listening; }

}  // namespace ipc
}  // namespace rix