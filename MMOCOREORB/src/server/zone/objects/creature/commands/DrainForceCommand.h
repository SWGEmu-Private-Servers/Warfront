/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DRAINFORCECOMMAND_H_
#define DRAINFORCECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "CombatQueueCommand.h"

class DrainForceCommand : public CombatQueueCommand {
public:

	DrainForceCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (creature->hasAttackDelay() || !creature->checkPostureChangeDelay())
			return GENERALERROR;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}
	
		uint32 avoidIncapacitationCRC = BuffCRC::JEDI_AVOID_INCAPACITATION;
		// Stop channeling while AI 
		if (creature->hasBuff(avoidIncapacitationCRC)){
			creature->sendSystemMessage("Cannot Drain Force while AI");
			return GENERALERROR;
		}

		// Fail if target is not a player...

		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		if (object == NULL || !object->isPlayerCreature())
			return INVALIDTARGET;

		CreatureObject* targetCreature = cast<CreatureObject*>( object.get());

		if (targetCreature == NULL || targetCreature->isDead() || (targetCreature->isIncapacitated() && !targetCreature->isFeigningDeath()) || !targetCreature->isAttackableBy(creature))
			return INVALIDTARGET;

		if(!checkDistance(creature, targetCreature, range))
			return TOOFAR;

		if (!CollisionManager::checkLineOfSight(creature, targetCreature)) {
			creature->sendSystemMessage("@combat_effects:cansee_fail");//You cannot see your target.
			return GENERALERROR;
		}

		   	if (!creature->checkCooldownRecovery("drainforce")) {
                    creature->sendSystemMessage("You cannot drain force yet.");
                    return GENERALERROR;
                }

		Locker clocker(targetCreature, creature);

		ManagedReference<PlayerObject*> targetGhost = targetCreature->getPlayerObject();
		ManagedReference<PlayerObject*> playerGhost = creature->getPlayerObject();

		if (targetGhost == NULL || playerGhost == NULL)
			return GENERALERROR;

		CombatManager* manager = CombatManager::instance();
		int maxDrain = 0;
		if (manager->startCombat(creature, targetCreature, false)) { //lockDefender = false because already locked above.
			int forceSpace = playerGhost->getForcePowerMax() - playerGhost->getForcePower();
			if (forceSpace <= 0) //Cannot Force Drain if attacker can't hold any more Force.
				return GENERALERROR;

			maxDrain = minDamage; //Value set in command lua.

			int targetForce = targetGhost->getForcePower();
			if (targetForce <= 0) {
				creature->sendSystemMessage("@jedi_spam:target_no_force"); //That target does not have any Force Power.
				return GENERALERROR;
			}

			maxDrain += 30;
			maxDrain *= creature->getFrsMod("power"); //FRS drains more
			maxDrain /= targetCreature->getFrsMod("control"); //FRS gets drained less

			if (creature->hasSkill("force_discipline_enhancements_master"))
				maxDrain *= 1.5; //Master enhancer drains more

			if (targetCreature->hasSkill("force_discipline_enhancements_master"))
				maxDrain /= 1.5; //Master enhancer gets drained less

			int forceDrain = targetForce >= maxDrain ? maxDrain : targetForce; //Drain whatever Force the target has, up to max.

			if (forceDrain > forceSpace)
				forceDrain = forceSpace; //Drain only what attacker can hold in their own Force pool.

			playerGhost->setForcePower(playerGhost->getForcePower() + forceDrain);
			targetGhost->setForcePower(targetGhost->getForcePower() - forceDrain);

			uint32 animCRC = getAnimationString().hashCode();
			creature->doCombatAnimation(targetCreature, animCRC, 0x1, 0xFF);
			manager->broadcastCombatSpam(creature, targetCreature, NULL, forceDrain, "cbt_spam", combatSpam, 1);
			creature->updateCooldownTimer("drainforce", (5000/creature->getFrsMod("manipulation"))); // CD scales with frs

			return SUCCESS;
		}

		return GENERALERROR;

	}

	float getCommandDuration(CreatureObject* object, const UnicodeString& arguments) const {
		return defaultTime * 1.2;
	}

};

#endif //DRAINFORCECOMMAND_H_
