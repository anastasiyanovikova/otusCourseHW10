#pragma once

#include "boost/asio/buffer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/address_v4.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/system/error_code.hpp"
#include "boost/asio.hpp"
#include "boost/bind/bind.hpp"
#include <iostream>
#include <memory>
#include <cinttypes>

namespace asio = boost::asio;

void* connect(std::size_t N);
void receive(const char * data, std::size_t dataSize, void* context);
void disconnect(void* context);

class bulkServerHelper
{
public:
    bulkServerHelper(std::size_t bulk_N): m_bulkN(bulk_N)
    {
        currentContext = connect(bulk_N);
    }
    ~bulkServerHelper()
    {
        disconnect(currentContext);
    }
    void addCommand(std::string msg)
    {
        receive(msg.c_str(), msg.size(), currentContext);
    }
private:
    void* currentContext = nullptr;
    std::size_t m_bulkN = 3;
};

struct Session : std::enable_shared_from_this<Session> {
    Session(asio::io_context &context, asio::ip::tcp::socket socket, bulkServerHelper& bulkHelper, std::size_t bulkN):
    m_context{context}, m_socket{std::move(socket)}, currentHelper(bulkHelper), tmpHelper(nullptr), bulkNumber(bulkN) {}

    void getLine()
    {
        auto self(shared_from_this());
        boost::asio::async_read_until(m_socket, bf, '\n', [this, self](boost::system::error_code ec, size_t recv_n)
        {
            bf.commit(recv_n);
            std::istream isStrm(&bf);
            std::string line;
            isStrm >> line;
            std::string newline = std::string(line.c_str(), recv_n);
            auto cycle = std::all_of(newline.begin(), newline.end(), [](char c){return c == '\0';});
            if(!ec)
            {
                if(!cycle)
                {
                    if(self->tmpHelper)
                    {
                        if(newline.find("{") != std::string::npos)
                            beginBr++;
                        else if(newline.find("}") != std::string::npos)
                            endBr++;
                        
                        self->tmpHelper->addCommand(newline);
                        if(endBr == beginBr)
                        {
                            endBr = 0;
                            beginBr = 0;
                            self->tmpHelper = nullptr;
                        }
                    }
                    else
                    {
                        if(newline.find("{") != std::string::npos)
                        {
                            beginBr++;
                            self->tmpHelper = new bulkServerHelper(bulkNumber);
                            self->tmpHelper->addCommand(newline);
                        }
                        else
                            currentHelper.addCommand(newline);   
                    }
                }
                getLine();
            }
            else
            {
                return;
            }
        });
    }

    void run(){
        getLine();
    }

    asio::io_context & m_context;
    asio::ip::tcp::socket m_socket;
    size_t rec_vNumbers;
    boost::asio::streambuf bf;
    bulkServerHelper& currentHelper;
    bulkServerHelper* tmpHelper = nullptr;
    std::size_t bulkNumber = 3;

    int beginBr = 0;
    int endBr = 0;
};

struct Server {
    Server(asio::io_context &context, unsigned short port, std::size_t bulkN): m_context{context}, m_port_number(port),
    acceptor{context, asio::ip::tcp::endpoint{asio::ip::make_address_v4("127.0.0.1"), port},},
    bulk_N(bulkN), m_helper(bulkServerHelper(bulkN))
    {
    }

    void accept(){
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_context);
        acceptor.async_accept(*socket, [this, socket](boost::system::error_code ec) {
            assert(!ec);
            std::cout << "connected a client\n";
            auto session = std::make_shared<Session>(m_context, std::move(*socket), m_helper, bulk_N);
            session->run();
            accept();
        });
    }
    asio::io_context & m_context;
    unsigned short m_port_number = 9000;
    asio::ip::tcp::acceptor acceptor;
    std::size_t bulk_N;
    bulkServerHelper m_helper;
};