
#ifndef MILKCREATURETASK_H_
#define MILKCREATURETASK_H_

#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "engine/engine.h"

class MilkCreatureTask : public Task {

private:
	enum Phase { INITIAL, ONESUCCESS, ONEFAILURE, FINAL} currentPhase;
	ManagedReference<Creature*> creature;
	ManagedReference<CreatureObject*> player;

public:
	MilkCreatureTask(Creature* cre, CreatureObject* playo) : Task() {
		currentPhase = INITIAL;
		creature = cre;
		player = playo;
	}

	void run() {
		Locker locker(creature);

		Locker _clocker(player, creature);

		if (!creature->isInRange(player, 5.f) || creature->isDead()) {
			updateMilkState(CreatureManager::NOTMILKED);
			player->sendSystemMessage("@skl_use:milk_too_far"); // The creature has moved too far away to continue milking it.
			return;
		}

		if (!player->hasState(CreatureState::MASKSCENT) || player->isInCombat() || creature->isInCombat()) {
			updateMilkState(CreatureManager::NOTMILKED);
			player->sendSystemMessage("@skl_use:milk_not_hidden"); // The creature is spooked and won't let you milk it.
			return;
		}

		float failureChance = 5 + (5 * creature->getFerocity());
		float skill = 100;
		bool success = true;

		if (player->hasBuff(STRING_HASHCODE("skill_buff_mask_scent_self"))) {
			skill += player->getSkillMod("mask_scent");
		} else if (player->hasBuff(STRING_HASHCODE("skill_buff_mask_scent"))) {
			skill +=  player->getSkillMod("private_conceal");
		}

		failureChance /= (skill / 100);
		float bonusChance = 0;

		// Check Scout / Ranger skill trees
		if (player->hasSkill("outdoors_scout_master")){
			bonusChance += 2;
			if (player->hasSkill("outdoors_ranger_novice")){
				bonusChance += 1;
				if (player->hasSkill("outdoors_ranger_master")){
					bonusChance += 2;
				}
			}
		}
		// Check CH skill trees
		if (player->hasSkill("outdoors_creaturehandler_novice")){
			bonusChance += 2;
			if (player->hasSkill("outdoors_creaturehandler_master")){
				bonusChance += 3;
			}
		}
		// Check GCW skill trees
		if (player->hasSkill("gcw_ranger_02")){
			bonusChance += 1;
			if (player->hasSkill("gcw_ranger_06")){
				bonusChance += 2;
			}
		}
		
		// add FS Luck
		bonusChance += player->getSkillMod("force_luck");

		// min failure chance 5%
		if ((bonusChance + 5) < failureChance){
			failureChance -= bonusChance;
		}else{
			failureChance = 5;	
		}
		
		if (System::random(100) < failureChance)
			success = false;

		switch (currentPhase) {
		case INITIAL:
			player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
			if (success) {
				currentPhase = ONESUCCESS;
			} else {
				currentPhase = ONEFAILURE;
			}
			this->reschedule(10000);
			break;
		case ONESUCCESS:
			if (success) {
					player->sendSystemMessage("@skl_use:milk_success"); // You have successfully gathered milk from the creature!
					giveMilkToPlayer();
			} else {
					player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
					currentPhase = FINAL;
					this->reschedule(10000);
			}
			break;
		case ONEFAILURE:
			if (success) {
				player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
				currentPhase = FINAL;
				this->reschedule(10000);
			} else {
				updateMilkState(CreatureManager::NOTMILKED);
				_clocker.release();
				CombatManager::instance()->startCombat(creature, player, true);
			}
			break;
		case FINAL:
			if (success) {
				player->sendSystemMessage("@skl_use:milk_success"); // You have successfully gathered milk from the creature!
				giveMilkToPlayer();
			} else {
				updateMilkState(CreatureManager::NOTMILKED);
				_clocker.release();
				CombatManager::instance()->startCombat(creature, player, true);
			}
			break;
		}

		return;
	}

	void giveMilkToPlayer() {
		ManagedReference<ResourceManager*> resourceManager = player->getZoneServer()->getResourceManager();

		String restype = creature->getMilkType();
		int quantity = creature->getMilk();

		int quantityExtracted = Math::max(quantity, 3);

		ManagedReference<ResourceSpawn*> resourceSpawn = resourceManager->getCurrentSpawn(restype, player->getZone()->getZoneName());

		if (resourceSpawn == NULL) {
			player->sendSystemMessage("Error: Server cannot locate a current spawn of " + restype);
			return;
		}

		float density = resourceSpawn->getDensityAt(player->getZone()->getZoneName(), player->getPositionX(), player->getPositionY());

		float bonusDensity = 0;
		int bonusQuantity = 0;
		float bonusMod = 0;

		// Check Scout / Ranger skill trees
		if (player->hasSkill("outdoors_scout_master")){
			bonusDensity += 2;
			bonusMod += .05;
			bonusQuantity += 3;
			if (player->hasSkill("outdoors_ranger_novice")){
				bonusMod += .05;
				bonusQuantity += 2;
				if (player->hasSkill("outdoors_ranger_master")){
					bonusMod += .1;
					bonusQuantity += 3;
				}
			}
		}
		// Check CH skill trees
		if (player->hasSkill("outdoors_creaturehandler_novice")){
			bonusDensity += 2;
			bonusMod += .1;
			bonusQuantity += 2;
			if (player->hasSkill("outdoors_creaturehandler_master")){
				bonusDensity += 3;
				bonusMod += .2;
				bonusQuantity += 5;
			}
		}
		// Check GCW skill trees
		if (player->hasSkill("gcw_ranger_01")){
			bonusDensity += 1;
			if (player->hasSkill("gcw_ranger_03")){
				bonusMod += .05;
				if (player->hasSkill("gcw_ranger_04")){
					bonusQuantity += 2;
					if (player->hasSkill("gcw_ranger_05")){
						bonusQuantity += 3;
						if (player->hasSkill("gcw_ranger_06")){
							bonusQuantity += 5;
							if (player->hasSkill("gcw_ranger_07")){
								bonusMod += .05;
								if (player->hasSkill("gcw_ranger_master")){
									bonusDensity += 3;
								}
							}
						}
					}
				}
			}
		}
		
		// add FS Luck to density and quantity
		bonusDensity += player->getSkillMod("force_luck");
		quantityExtracted += System::random(player->getSkillMod("force_luck")*2);

		density += bonusDensity;
	
		if (density > 0.80f) {
			quantityExtracted = int(quantityExtracted * 1.25f);
		} else if (density > 0.60f) {
			quantityExtracted = int(quantityExtracted * 1.00f);
		} else if (density > 0.40f) {
			quantityExtracted = int(quantityExtracted * 0.75f);
		} else {
			quantityExtracted = int(quantityExtracted * 0.50f);
		}

		// add bonus multiplier from skill trees
		quantityExtracted += int(quantityExtracted * bonusMod);
		// add in bonus quantity from skill trees
		quantityExtracted += bonusQuantity;

		resourceManager->harvestResourceToPlayer(player, resourceSpawn, quantityExtracted);
		updateMilkState(CreatureManager::ALREADYMILKED);
	}

	void updateMilkState(const short milkState) {
		Locker clocker(creature);
		creature->setMilkState(milkState);
	}
};

#endif /* MILKCREATURETASK_H_ */
