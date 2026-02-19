#include "lidar_driver/lidar_driver.hpp"

LidarDriver::LidarDriver(std::unique_ptr<interfaces::Server> server, std::unique_ptr<LidarBase> lidar)
    : server(std::move(server)), lidar(std::move(lidar)) {
    this->lidar->set_callback(std::bind(&LidarDriver::on_scan_callback, this, std::placeholders::_1));
}

LidarDriver::~LidarDriver() { lidar->set_callback(nullptr); }

/**< TODO */
void LidarDriver::spin(std::unique_ptr<interfaces::Notification> notif) {

  lidar->spin();

    const rix::util::Duration poll(0.01);  // 10ms poll interval

    bool done = false;
    while (!done) {
        // Wait up to 10ms for an incoming connection, then re-check notif
        if (!server->wait_for_accept(poll)) {
            done = notif->wait(rix::util::Duration(0));
            continue;
        }

        std::weak_ptr<interfaces::Connection> new_connection;
        if (!server->accept(new_connection)) {
            done = notif->wait(rix::util::Duration(0));
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(connection_mtx);
            connection = new_connection;
        }

        // After accepting, keep running until shutdown.
        // The callback thread handles sending scans.
        while (!(done = notif->wait(rix::util::Duration(0)))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void LidarDriver::on_scan_callback(const rix::msg::sensor::LaserScan &scan) {
    std::lock_guard<std::mutex> lock(connection_mtx);

    auto shared_conn = connection.lock();
    if (!shared_conn) {
        return;
    }

    size_t msg_size = scan.size();

    // Build a single buffer: serialized size prefix + serialized scan
    rix::msg::standard::UInt32 size_prefix;
    size_prefix.data = static_cast<uint32_t>(msg_size);

    std::vector<uint8_t> buffer(size_prefix.size() + msg_size);
    size_t offset = 0;
    size_prefix.serialize(buffer.data(), offset);  // serialize size prefix properly
    scan.serialize(buffer.data(), offset);          // serialize scan after it

    shared_conn->write(buffer.data(), buffer.size());
}
