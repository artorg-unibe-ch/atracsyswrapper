//
// Created by Iwan Paolucci on 08/05/2018.
//

#include "connectionlistener.h"


ConnectionListener::ConnectionListener() {}

ConnectionListener::~ConnectionListener() {
	run = false;
	connectionListener.detach();
    serverSocket->CloseSocket();
}

void ConnectionListener::init() {
    serverSocket = igtl::ServerSocket::New();
    int r = serverSocket->CreateServer(22222);
    if(r == 0){
		run = true;
		connectionListener = std::thread(&ConnectionListener::listenForConnections, this);
    }
}

void ConnectionListener::listenForConnections() {
	while (run) {
		igtl::Socket::Pointer socket;
		socket = serverSocket->WaitForConnection(10000);
		if (!socket.IsNotNull()) {
			std::cout << "No connection" << std::endl;
		}
		else {
			std::string address;
			int port;
			socket->GetSocketAddressAndPort(address, port);
			std::cout << "connection from " << address << ":" << port << std::endl;
			connections.push_back(socket);
		}
	}
}

void ConnectionListener::send(igtl::TrackingDataMessage::Pointer message) {
    if(connections.size() > 0) {
		auto iter = connections.begin();
        while (iter != connections.end()) {
			auto socket = *iter;
			int r = socket->Send(message->GetPackPointer(), message->GetPackSize());
			if (r != 1) {
				iter = connections.erase(iter);
			}
			else {
				++iter;
			}
        }
    }
}
