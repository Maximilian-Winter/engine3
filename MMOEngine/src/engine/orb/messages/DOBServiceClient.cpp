/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "engine/orb/DistributedObjectBroker.h"

#include "RemoteObjectBroker.h"

#include "DOBServiceHandler.h"

DOBServiceClient::DOBServiceClient(Socket* sock)
		: StreamServiceClient(sock), Logger("DistributedObjectBroker") {
	/*info("client \'" + host + "\' connected", true);*/

	rob = new RemoteObjectBroker(this);

	serviceHandler = nullptr;

	//setLogging(true);
	//setGlobalLogging(true);

	info("client connected");
}

DOBServiceClient::DOBServiceClient(const String& host, int port)
		: StreamServiceClient(host, port) , Logger("DistributedObjectBroker") {
	rob = nullptr;

	serviceHandler = new DOBServiceHandler();

	//setLogging(true);
	//setGlobalLogging(true);

	try {
		connect();
		info("connected to server \'" + host + "\'");
	} catch (SocketException& e) {
		error("unable to connect to \'" + host + "\'");
		error(e.getMessage());
	}
}

DOBServiceClient::~DOBServiceClient() {
	if (rob != nullptr) {
		delete rob;
		rob = nullptr;
	}
}

void DOBServiceClient::run() {
	receiveMessages();
}

void DOBServiceClient::receiveMessages() {
	Packet packet;

	assert(serviceHandler);

	int messageSize = 0;

	while (doRun) {
		try	{
			if (packet.size() > 0) {
				packet.setOffset(packet.size() - 1);
			} else {
				packet.reset();
			}

			if (receiveAppend(&packet)) {
				//printf("ran receive append with data:%s\n", packet.toStringData().toCharArray());

				if (packet.size() == 0)
					break;

				while (packet.size() > 0) {
					packet.reset();
					//printf("ran while loop with data:%s\n", packet.toStringData().toCharArray());

					if (messageSize && (messageSize <= packet.size())) {
						//printf("messageSize:%d packet.size() pre handle:%d\n", messageSize, packet.size());

						Packet incomingData;
						incomingData.writeStream(&packet, messageSize);
						incomingData.reset();

						serviceHandler->handleMessage(this, &incomingData);

						//printf("messageSize:%d packet.size() post handle:%d\n", messageSize, packet.size());
						packet.removeRange(0, messageSize);
						messageSize = 0;
						packet.reset();

						//printf("messageSize:%d packet.size() post remove range:%d\n", messageSize, packet.size());
						//printf("data post remove range:%s\n", packet.toStringData().toCharArray());
					} else {
						if (!messageSize) {
							if (packet.size() >= 4) {
								packet.reset();

								messageSize = packet.parseInt(0) - 4;

								packet.removeRange(0, 4);
							} else {
								break;
							}
						} else {
							break;
						}
					}
				}
			} else {
				doRun = false;
			}
		} catch (SocketException& e) {
			if (!serviceHandler->handleError(this, e)) {
				doRun = false;
				break;
			}
		}
	}

	disconnect();
}


bool DOBServiceClient::send(DOBMessage* message) {
	message->setSequence(sentMessageSequence.increment());
	message->setSize();

	info("SEND " + message->toStringData());
	StreamServiceClient::send(message);

	//delete message;
	return true;
}


bool DOBServiceClient::sendReply(DOBMessage* message) {
	message->setSize();

	info("SEND " + message->toStringData());

	StreamServiceClient::send(message);

	//delete message;
	return true;
}

bool DOBServiceClient::sendAndAcceptReply(DOBMessage* message) {
	sendWithReply(message);

	return waitForReply(message);
}

bool DOBServiceClient::sendWithReply(DOBMessage* message) {
	uint32 sequence = sentMessageSequence.increment();
	message->setSequence(sequence);
	message->setSize();

	sentMessageQueue.put(sequence, message);

	info("SEND " + message->toStringData());
	return StreamServiceClient::send(message);
}

bool DOBServiceClient::waitForReply(DOBMessage* message) {
	bool res = message->waitForReply();

	if (!res)
		error("timeout on message " + String::valueOf(message->getSequence()));

	return res;
}
