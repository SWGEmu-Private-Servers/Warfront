/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DIRTYTRICKCOMMAND_H_
#define DIRTYTRICKCOMMAND_H_

#include "CombatQueueCommand.h"

class DirtyTrickCommand : public CombatQueueCommand {
public:

	DirtyTrickCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<WeaponObject*> weapon = creature->getWeapon();

		if (!weapon->isPistolWeapon())
			return INVALIDWEAPON;

		return doCombatAction(creature, target);
	}

};

#endif //DIRTYTRICKCOMMAND_H_
