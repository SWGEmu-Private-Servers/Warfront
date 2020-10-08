/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef REMOTEDETONATORCOMMAND_H_
#define REMOTEDETONATORCOMMAND_H_

#include "CombatQueueCommand.h"

class RemoteDetonatorCommand : public CombatQueueCommand {
public:

	RemoteDetonatorCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;
		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		if (object == NULL || !object->isCreatureObject())
			return INVALIDTARGET;

		CreatureObject* creatureTarget = cast<CreatureObject*>( object.get());
		PlayerManager* playerManager = server->getPlayerManager();

		if (creature != creatureTarget && !CollisionManager::checkLineOfSight(creature, creatureTarget)) {
			creature->sendSystemMessage("You do not have a clear line of sight to the target.");
			return INVALIDTARGET;
		}

		if (!creature->checkCooldownRecovery("explosion")) {
			StringIdChatParameter stringId;

			Time* cdTime = creature->getCooldownTime("explosion");

			// Returns -time. Multiple by -1 to return positive.
			int timeLeft = floor((float)cdTime->miliDifference() / 1000) *-1;

			stringId.setStringId("@innate:equil_wait"); // You are still recovering from your last equilization. Command available in %DI seconds.
			stringId.setDI(timeLeft);
			creature->sendSystemMessage(stringId);
			return GENERALERROR;
		}

		if (creatureTarget == NULL)
			return INVALIDTARGET;

		Locker clocker(creatureTarget, creature);

		ManagedReference<PlayerObject*> targetGhost = creatureTarget->getPlayerObject();
		ManagedReference<PlayerObject*> playerObject = creature->getPlayerObject();

		if (targetGhost == NULL || playerObject == NULL)
			return GENERALERROR;

		if (creature->getDistanceTo(creatureTarget) > 20.f){
			creature->sendSystemMessage("You are out of range.");
			return GENERALERROR;}

		if (creatureTarget->isRidingMount()) {
			creature->sendSystemMessage("you cannot knockdown a player while they are mounted");
			return GENERALERROR;
		}

		if (object->isCreatureObject() && creatureTarget->isAttackableBy(creature)) {
			creatureTarget->setSnaredState(5);
			//creatureTarget->playEffect("clienteffect/carbine_snare.cef", "");
			creatureTarget->playEffect("clienteffect/restuss_event_big_explosion.cef", "");
			creatureTarget->inflictDamage(creatureTarget, CreatureAttribute::HEALTH, 500 + System::random(500), true);
			creatureTarget->sendSystemMessage("You have been snared");
			creature->addCooldown("explosion", 60 * 1000);

		}

		
		uint32 buffCRC = BuffCRC::FORCE_RANK_SUFFERING; //DURATION
		int duration = 10;
		int loopCount = ((duration/9)-1);
		ManagedReference<SingleUseBuff*> buff = new SingleUseBuff(creatureTarget, buffCRC, duration, BuffType::JEDI, getNameCRC());
		
		 if (!creature->hasBuff(buffCRC)) {
			Locker locker(buff);
			buff->setSpeedMultiplierMod(0.01f);
			creatureTarget->addBuff(buff);
			}

		return doCombatAction(creature, target);
	}
		
};

#endif //REMOTEDETONATORCOMMAND_H_
