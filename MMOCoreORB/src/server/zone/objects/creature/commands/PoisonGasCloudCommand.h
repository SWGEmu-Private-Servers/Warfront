/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef POISONGASCLOUDCOMMAND_H_
#define POISONGASCLOUDCOMMAND_H_

#include "CombatQueueCommand.h"

class PoisonGasCloudCommand : public CombatQueueCommand {
public:

	PoisonGasCloudCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<SceneObject*> targetObject = creature->getZoneServer()->getObject(target);

		CreatureObject* targetCreature = cast<CreatureObject*>(targetObject.get());

		if (targetCreature == NULL)
			return INVALIDTARGET;

		if (creature != targetCreature && !CollisionManager::checkLineOfSight(creature, targetCreature)) {
			creature->sendSystemMessage("You do not have a clear line of sight to the target.");
			return INVALIDTARGET;
		}


		if (!targetCreature->isAttackableBy(creature))
			return INVALIDTARGET;

		CreatureObject* player = cast<CreatureObject*>(creature);
		Locker clocker(targetCreature, creature);

		if (creature->getDistanceTo(targetCreature) > 20.f){
			creature->sendSystemMessage("You are out of range.");
			return GENERALERROR;}

		if (!creature->checkCooldownRecovery("gas")) {
   			StringIdChatParameter stringId;

   			Time* cdTime = creature->getCooldownTime("gas");

   			int timeLeft = floor((float)cdTime->miliDifference() / 1000) *-1;

   			stringId.setStringId("@innate:equil_wait"); // You are still recovering from your last Command available in %DI seconds.
   			stringId.setDI(timeLeft);
   			creature->sendSystemMessage(stringId);
   			        return GENERALERROR;
   		       }

 		player->addCooldown("gas", 10 * 1000); // 10 second cooldown
		targetCreature->playEffect("clienteffect/cbt_explode_asteroid_gas_large.cef", "");

		return doCombatAction(creature, target);
	}

};

#endif //POISONGASCLOUDCOMMAND_H_
