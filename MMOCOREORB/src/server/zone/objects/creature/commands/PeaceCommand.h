/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef PEACECOMMAND_H_
#define PEACECOMMAND_H_

#include "server/zone/managers/combat/CombatManager.h"

class PeaceCommand : public QueueCommand {
public:

	PeaceCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if ((creature->isPlayerCreature()) and !creature->checkCooldownRecovery("peace")) {
		StringIdChatParameter stringId;

		Time* cdTime = creature->getCooldownTime("peace");

		// Returns -time. Multiple by -1 to return positive.
		int timeLeft = floor((float)cdTime->miliDifference() / 1000) *-1;

		stringId.setStringId("@innate:equil_wait"); // You are still recovering from your last equilization. Command available in %DI seconds.
		stringId.setDI(timeLeft);
		creature->sendSystemMessage(stringId);
		return GENERALERROR;
	}
		if (creature->isPlayerCreature()){
			creature->clearQueueActions();
			creature->addCooldown("peace", 10 * 1000);
		}

		CombatManager::instance()->attemptPeace(creature);

		return SUCCESS;
	}

};

#endif //PEACECOMMAND_H_
