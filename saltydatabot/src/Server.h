/*
 * Server.h
 *
 *  Created on: Oct 26, 2013
 *      Author: stamptd
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <stdlib.h>

using namespace boost::asio;
#define READBUFFERSIZE 8

typedef struct ConnectionWrapper {
    ip::tcp::socket socket;
    char readBuffer[READBUFFERSIZE];
    ConnectionWrapper(io_service &io) : socket(io){ memset(readBuffer,0,READBUFFERSIZE); }
} ConnectionWrapper;

typedef std::vector<ConnectionWrapper*> connectionVector;

class Server {
public:
    Server();
    virtual ~Server();
    void asyncRun();
    void publishNames(std::string n1, std::string n2);
private:
    void acceptHandler(ConnectionWrapper *s, const boost::system::error_code &ec);
    void readHandler(ConnectionWrapper *w, const boost::system::error_code& error, std::size_t bytes_transferred);
    void writehandler(const boost::system::error_code &ec, std::size_t bytes_transferred);
    void listen();
    void startAccept();

    io_service _io_service;
    std::string _name1Buffer;
    std::string _name2Buffer;

    ip::tcp::endpoint _endpoint;
    ip::tcp::acceptor _acceptor;
    connectionVector _sockets;
    boost::thread _thread;
};

#endif /* SERVER_H_ */
