/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SABERPOLEARMHIT3COMMAND_H_
#define SABERPOLEARMHIT3COMMAND_H_

#include "JediCombatQueueCommand.h"

class SaberPolearmHit3Command : public JediCombatQueueCommand {
public:

	SaberPolearmHit3Command(const String& name, ZoneProcessServer* server)
		: JediCombatQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (creature->isInvisible()) {
			return GENERALERROR;
		}

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		ManagedReference<WeaponObject*> weapon = creature->getWeapon();


		if (!weapon->isJediPolearmWeapon()) {
			return INVALIDWEAPON;
		}

		return doCombatAction(creature, target);
	}

};

#endif //SABERPOLEARMHIT3COMMAND_H_
