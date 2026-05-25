#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
        : socket_(io_context), endpoints_(endpoints) {}

    void start() {
        do_connect();
    }

    void send_message(const std::string& message);

private:
    void do_connect();
    void do_read();

    tcp::socket socket_;
    tcp::resolver::results_type endpoints_;
    std::string outbound_message_;
    enum { max_length = 1024 };
    char data_[max_length];
};

void Client::do_connect() {
    auto self = shared_from_this();
    boost::asio::async_connect(
        socket_,
        endpoints_,
        [this, self](boost::system::error_code ec, const tcp::endpoint&) {
            if (!ec) {
                std::cout << "Connected to server.\n";
            } else {
                std::cout << "Connection error: " << ec.message() << "\n";
            }
        });
}

void Client::send_message(const std::string& message) {
    auto self = shared_from_this();
    outbound_message_ = message;
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(outbound_message_),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::cout << "Message sent.\n";
                do_read();
            } else {
                std::cout << "Send error: " << ec.message() << "\n";
            }
        });
}

void Client::do_read() {
    auto self = shared_from_this();
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::cout << "Received from server: "
                          << std::string(data_, length) << "\n";
            } else {
                std::cout << "Receive error: " << ec.message() << "\n";
            }
        });
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        auto client = std::make_shared<Client>(io_context, endpoints);
        client->start();

        std::thread t([&io_context]() { io_context.run(); });

        std::string message;
        while (true) {
            std::cout << "Enter coordinates (e.g. 55.75,37.61): ";
            std::getline(std::cin, message);
            if (message.empty()) {
                break;
            }
            client->send_message(message);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        io_context.stop();
        t.join();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
