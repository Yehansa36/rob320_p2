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

    while (!notif->is_ready()) {
        std::weak_ptr<interfaces::Connection> new_connection;

        //accept a cleint connection
        if (!server->accept(new_connection)) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(connection_mtx);
            connection = new_connection;

        }

        auto shared_conn = new_connection.lock();
        if (!shared_conn) continue;

        Endpoint remote = shared_conn->remote_endpoint();

        //connect clinet to same addres but port 8300
        Endpoint goal_server(remote.address, 8300);

        if (!client->connect(goal_server)) {
            client->reset();
            continue;
        }

        //read goal pose2dstamped msgs
        while (!notif->is_ready()) {
            //first read 4 byte size 

            UInt32 size_msg;
            ssize_t n = client->read(reinterpret_cast<uint8_t*>(&size_msg), sizeof(UInt32));

            if (n <= 0) {
                client->reset();
                break;
            }

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

/**< TODO */
void MBotDriver::on_pose_callback(const Pose2DStamped &pose) {
    std::lock_guard<std::mutex> lock(connection_mtx);

    auto shared_conn = connection.lock();
    if (!shared_conn) return;

    size_t msg_size = pose.size();

    //allocate buffer
    std::vector<uint8_t> buffer(msg_size);

    //serialize into buffer
    size_t offset = 0;
    pose.serialize(buffer.data(), offset);

    //prefix with 4 byts size
    UInt32 size_prefix;
    size_prefix.data = static_cast<uint32_t>(msg_size);

    //send size prefix
    if (shared_conn->write(reinterpret_cast<uint8_t*>(&size_prefix), sizeof(UInt32)) <= 0) {
        return;
    }

    shared_conn->write(buffer.data(), msg_size);

}