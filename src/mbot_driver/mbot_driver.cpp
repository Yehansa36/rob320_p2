#include "mbot_driver/mbot_driver.hpp"

using rix::msg::standard::UInt32;

MBotDriver::MBotDriver(std::unique_ptr<interfaces::Server> server, std::unique_ptr<interfaces::Client> client,
                       std::unique_ptr<MBotBase> mbot)
    : server(std::move(server)), client(std::move(client)), mbot(std::move(mbot)) {
    this->mbot->set_callback(std::bind(&MBotDriver::on_pose_callback, this, std::placeholders::_1));
}

/**< TODO */
void MBotDriver::spin(std::unique_ptr<interfaces::Notification> notif) {
    //run until notifincation is triggered

    //auto start_time = std::chrono::steady_clock::now();
    //const auto max_duration = std::chrono::seconds(10);
  mbot->spin();

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

        auto shared_conn = new_connection.lock();
        if (!shared_conn) continue;

        Endpoint remote = shared_conn->remote_endpoint();

        // Connect client to same address but port 8300
        Endpoint goal_server(remote.address, 8300);

        // Retry connecting until successful or shutdown
        while (!client->connect(goal_server)) {
            if ((done = notif->wait(rix::util::Duration(0)))) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (done) break;

        // Read goal Pose2DStamped messages until disconnected or shutdown
        while (!done) {
            // Wait up to 10ms for readable data, then re-check notif
            if (!client->wait_for_readable(poll)) {
                done = notif->wait(rix::util::Duration(0));
                continue;
            }

            // Read 4-byte size prefix (serialized)
            UInt32 size_msg;
            std::vector<uint8_t> size_buf(size_msg.size());
            ssize_t n = client->read(size_buf.data(), size_buf.size());

            if (n <= 0) {
                client->reset();
                break;
            }

            size_t size_offset = 0;
            size_msg.deserialize(size_buf.data(), size_buf.size(), size_offset);
            uint32_t msg_size = size_msg.data;
            std::vector<uint8_t> buffer(msg_size);

            ssize_t total_read = 0;
            while (total_read < (ssize_t)msg_size) {
                ssize_t r = client->read(buffer.data() + total_read, msg_size - total_read);

                if (r <= 0) {
                    client->reset();
                    break;
                }

                total_read += r;
            }

            if (total_read != (ssize_t)msg_size) {
                break;
            }

            Pose2DStamped goal_pose;
            size_t bytes_read = 0;
            goal_pose.deserialize(buffer.data(), buffer.size(), bytes_read);

            mbot->drive_to(goal_pose);
        }
    }
}

void MBotDriver::on_pose_callback(const Pose2DStamped &pose) {
    std::lock_guard<std::mutex> lock(connection_mtx);

    auto shared_conn = connection.lock();
    if (!shared_conn) return;

    size_t msg_size = pose.size();

    // Build a single buffer: serialized size prefix + serialized pose
    UInt32 size_prefix;
    size_prefix.data = static_cast<uint32_t>(msg_size);

    std::vector<uint8_t> buffer(size_prefix.size() + msg_size);
    size_t offset = 0;
    size_prefix.serialize(buffer.data(), offset);  // serialize size prefix properly
    pose.serialize(buffer.data(), offset);          // serialize pose after it

    shared_conn->write(buffer.data(), buffer.size());
}