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
bool Socket::bind(const Endpoint &endpoint) { return false; }

/**< TODO */
bool Socket::listen(int backlog) { return false; }

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::connect(const Endpoint &endpoint) { return false; }

/**< TODO */
bool Socket::accept(Socket &sock) { return false; }

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::getsockname(Endpoint &endpoint) const { return false; }

/**< TODO
 * Hint: You only need to consider the case when domain is AF_INET (IPv4)
 */
bool Socket::getpeername(Endpoint &endpoint) const { return false; }

/**< TODO */
bool Socket::getsockopt(int level, int optname, int &value) { return false; }

/**< TODO */
bool Socket::setsockopt(int level, int optname, int value) { return false; }

bool Socket::is_bound() const { return _bound; }

bool Socket::is_listening() const { return _listening; }

}  // namespace ipc
}  // namespace rix