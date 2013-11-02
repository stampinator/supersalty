/*
 * Server.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: stamptd
 */

#include "Server.h"
#include <iostream>
#include <boost/thread.hpp>
#include <string>


#define PORT "1337"
#define BACKLOG 10

using namespace std;
using namespace boost::asio;

static string WELCOMEMESSAGE = "You are now connected to the salty databot server. Enjoy your stay\n";

Server::Server() :
        _io_service(),
        _endpoint(ip::tcp::v4(),1337),
        _acceptor(_io_service,_endpoint),
        _sockets()
{

}

Server::~Server() {
//    Could cause a concurrent modification error, but this only happens when the program is shutting down anyway.
    for(std::vector<ConnectionWrapper*>::iterator it; it != _sockets.end(); ++it){
        delete *it;
    }
}

void Server::publishNames(std::string n1, std::string n2) {
    connectionVector::iterator it;
    _name1Buffer = n1 + "\n";
    _name2Buffer = n2 + "\n";
//    cout << "Publishing names " << n1 << " " << n2 << endl;
    for(it = _sockets.begin(); it != _sockets.end() - 1; ++it){
//        cout << "Writing to socket" << *it << endl;
        async_write((*it)->socket,buffer(_name1Buffer),
                boost::bind(&Server::writehandler, this, placeholders::error, placeholders::bytes_transferred));
        async_write((*it)->socket,buffer(_name2Buffer),
                boost::bind(&Server::writehandler, this, placeholders::error, placeholders::bytes_transferred));
    }
}

void Server::startAccept(){
    ConnectionWrapper *s = new ConnectionWrapper(_io_service);
    _sockets.push_back(s);
    _acceptor.async_accept(s->socket,
            boost::bind(&Server::acceptHandler,this, s,
                    boost::asio::placeholders::error));
}

void Server::acceptHandler(ConnectionWrapper *s, const boost::system::error_code& ec) {
    if(!ec)
    {
        cout << "New connection " << s << endl;
        async_write(s->socket,buffer(WELCOMEMESSAGE),
                boost::bind(&Server::writehandler, this, placeholders::error, placeholders::bytes_transferred));
        s->socket.async_receive(buffer(s->readBuffer,READBUFFERSIZE),0,
                boost::bind(&Server::readHandler,this,s,placeholders::error, placeholders::bytes_transferred));
        startAccept();
    }
}

void Server::readHandler(ConnectionWrapper *w, const boost::system::error_code& error, std::size_t bytes_transferred){
    if(bytes_transferred == 0){
        _sockets.erase(remove(_sockets.begin(),_sockets.end(),w),_sockets.end());
        cout << "Client disconnected. " << _sockets.size() - 1 << " clients remaining." << endl;
    } else {
        w->socket.async_receive(buffer(w->readBuffer,READBUFFERSIZE),0,
                boost::bind(&Server::readHandler,this,w,placeholders::error, placeholders::bytes_transferred));
    }
}

void Server::asyncRun(){
    _acceptor.listen();
    _thread = boost::thread(&Server::listen,this);
}

void Server::writehandler(const boost::system::error_code& ec,
        std::size_t bytes_transferred) {
}

void Server::listen(){
    cout << "Starting listen server on port " PORT << endl;
    startAccept();
    _io_service.run();
    cout << "Listen server terminated";
}




