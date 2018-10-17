/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef UPDATEMODIFIEDOBJECTSMESSAGE_H_
#define UPDATEMODIFIEDOBJECTSMESSAGE_H_

#include "system/lang.h"

#include "engine/orb/messages/DOBMessage.h"

#include "ObjectBrokerAgent.h"

namespace engine {
  namespace ORB {

	class ControlMessage : public DOBMessage {
		int command;

	public:	
		ControlMessage(int command) : DOBMessage(CONTROLMESSAGE, 20) {
			insertInt(command);
		}
	
		ControlMessage(Packet* message) : DOBMessage(message) {
			command = message->parseInt();
		}

		void execute() {
			ObjectBrokerAgent* agent = ObjectBrokerAgent::instance();

			agent->doCommand((ObjectBrokerDirector::Command) command);
		}
	};

  } // namespace ORB
} // namespace engine

using namespace engine::ORB;

#endif /*UPDATEMODIFIEDOBJECTSMESSAGE_H_*/
