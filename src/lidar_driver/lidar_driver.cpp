#include "lidar_driver/lidar_driver.hpp"

LidarDriver::LidarDriver(std::unique_ptr<interfaces::Server> server, std::unique_ptr<LidarBase> lidar)
    : server(std::move(server)), lidar(std::move(lidar)) {
    this->lidar->set_callback(std::bind(&LidarDriver::on_scan_callback, this, std::placeholders::_1));
}

LidarDriver::~LidarDriver() { lidar->set_callback(nullptr); }

/**< TODO */
void LidarDriver::spin(std::unique_ptr<interfaces::Notification> notif) {

    while (notif->is_ready()) {

        std::weak_ptr<interfaces::Connection> new_connection;

        //accept a client connection
        if (!server->accept(new_connection)) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(connection_mtx);
            connection = new_connection;
        }

        //after accepting, keep running until shutdown
        //the callback thread will handle sending scans

        while (notif->is_ready()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        }

    }
}

/**< TODO */
void LidarDriver::on_scan_callback(const rix::msg::sensor::LaserScan &scan) {
    std::lock_guard<std::mutex> lock(connection_mtx);

    auto shared_conn = connection.lock();
    if (!shared_conn) {
        return;
    }

    size_t msg_size = scan.size();

    //allocate buffer
    std::vector<uint8_t> buffer(msg_size);

    //serialize msg
    size_t offset = 0;
    scan.serialize(buffer.data(), offset);

    //prefix with 4 byte size 
    rix::msg::standard::UInt32 size_prefix;
    size_prefix.data = static_cast<uint32_t>(msg_size);

    //send size prefix 
    if (shared_conn->write(reinterpret_cast<uint8_t*>(&size_prefix), sizeof(size_prefix)) <= 0) {
        return;
    }

    //send payload
    shared_conn->write(buffer.data(), msg_size);
    


}
