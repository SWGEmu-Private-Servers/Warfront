/*
 * Orbital Strike
 *
 *  Created on: 11/22/2015
 *
 */

#ifndef ORBITALSTRIKECOMMAND_H_
#define ORBITALSTRIKECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "CombatQueueCommand.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/Zone.h"

class OrbitalStrikeCommand : public CombatQueueCommand {
public:

	OrbitalStrikeCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {

	}
	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		CreatureObject* player = cast<CreatureObject*>(creature);

		int nSkill = 0;
		if (creature->hasSkill("force_title_jedi_rank_02"))
			nSkill += 1;

		ZoneServer* zserv = creature->getZoneServer();
		PlayerObject* jedi = creature->getPlayerObject();

		if (nSkill > 0){
			creature->sendSystemMessage("You are trying to use a top level skill while being invested in jedi skills, "
					"you must devote yourself to science and skill, not mystic arts before using this command.");
			return GENERALERROR;
		}

		// Fail if target is not a player...
		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		if (object == NULL || !object->isCreatureObject())
			return INVALIDTARGET;

		CreatureObject* creatureTarget = cast<CreatureObject*>( object.get());
		PlayerManager* playerManager = server->getPlayerManager();

		if (creature != creatureTarget && !CollisionManager::checkLineOfSight(creature, creatureTarget)) {
			creature->sendSystemMessage("You do not have a clear line of sight to the target.");
			return INVALIDTARGET;
		}

		if (!creature->checkCooldownRecovery("strike")) {
			StringIdChatParameter stringId;

			Time* cdTime = creature->getCooldownTime("strike");

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

		if (creature->getDistanceTo(creatureTarget) > 42.f){
			creature->sendSystemMessage("You are out of range.");
			return GENERALERROR;}

		if (object->isCreatureObject() && creatureTarget->isAttackableBy(creature)) {
    		creatureTarget->setDizziedState(6);
		creature->addDefender(creatureTarget);
		    creatureTarget->setPosture(CreaturePosture::CROUCHED);
			creatureTarget->playEffect("clienteffect/combat_pt_orbitalstrike.cef", "");
			creatureTarget->sendSystemMessage("A beam of energy from an orbiting ship hits your position");
			creatureTarget->inflictDamage(creatureTarget, CreatureAttribute::HEALTH, 1000 + System::random(500), true);
		creature->addCooldown("strike", 60 * 1000);

		}

		return SUCCESS;
	}

};


#endif //ORBITALSTRIKECOMMAND_H_
