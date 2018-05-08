//
// Created by Iwan Paolucci on 08/05/2018.
//

#pragma once


#include <igtl/igtlServerSocket.h>
#include <thread>
#include <igtl/igtlTrackingDataMessage.h>

class ConnectionListener {
public:
    ConnectionListener();

    virtual ~ConnectionListener();

    void init();
    
	void listenForConnections();
    void send(igtl::TrackingDataMessage::Pointer message);
private:
	bool run;
    std::thread connectionListener;
    igtl::ServerSocket::Pointer serverSocket;
    std::vector<igtl::Socket::Pointer> connections;
};


