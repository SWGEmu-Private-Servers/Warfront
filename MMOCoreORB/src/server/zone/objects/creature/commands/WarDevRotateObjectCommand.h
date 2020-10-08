/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef WARDEVROTATEOBJECTCOMMAND_H_
#define WARDEVROTATEOBJECTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class WarDevRotateObjectCommand : public QueueCommand {
public:

	WarDevRotateObjectCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == NULL)
			return GENERALERROR;

		int adminLevelCheck = ghost->getAdminLevel();

		if (adminLevelCheck != 15){
		creature->sendSystemMessage("Sorry, the /devrotate command requires administrator privileges.");
		return GENERALERROR;
		}

		String dir;
		int degrees = 0;
		bool enableAllRotation = true; // Default is false, for SWG 14.1 style rotation.

		if (enableAllRotation == false){
			// SWG version 14.1 rotation style
			try {
			UnicodeTokenizer tokenizer(arguments.toString());
			tokenizer.getStringToken(dir);
			degrees = tokenizer.getIntToken();

			dir = dir.toLowerCase();

			if (dir != "left" && dir != "right")
				throw Exception();

		} catch (Exception& e) {
			creature->sendSystemMessage("@player_structure:formet_rotratefurniture_degrees"); //Format: /rotateFurniture <LEFT/RIGHT> <degrees>
			return INVALIDPARAMETERS;
		}

		if (degrees < 1 || degrees > 180) {
			creature->sendSystemMessage("@player_structure:rotate_params"); //The amount to rotate must be between 1 and 180.
			return INVALIDPARAMETERS;
		}

		ZoneServer* zoneServer = creature->getZoneServer();
		ManagedReference<SceneObject*> obj = zoneServer->getObject(target);

		if (obj == NULL || !obj->isTangibleObject() || obj->isPlayerCreature() || obj->isPet()) {
			creature->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
			return GENERALERROR;
		}

		if (dir == "right")
			obj->rotate(-degrees);
		else
			obj->rotate(degrees);

		obj->incrementMovementCounter();

		if (obj->getParent() != NULL)
			obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY(), obj->getParent().get()->getObjectID());
		else
			obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY());

		return SUCCESS;
	}
		else{


			ManagedReference<SceneObject*> rootParent = creature->getRootParent();

			try {
				UnicodeTokenizer tokenizer(arguments.toString());
				tokenizer.getStringToken(dir);
				degrees = tokenizer.getIntToken();

				dir = dir.toLowerCase();

				if  (dir != "left" && dir != "right" && dir != "yaw" && dir != "roll" && dir != "pitch" && dir != "reset")
					throw Exception();

			} catch (Exception& e) {
				creature->sendSystemMessage("Standard Format: /rotateFurniture <LEFT/RIGHT> <degrees>. Degrees can be 1 to 180 when using this format.");
				creature->sendSystemMessage("Enhanced Format: /rotateFurniture <YAW/PITCH/ROLL> <degrees>. Degrees can be -360 to 360 when using this format.");
				creature->sendSystemMessage("Reset Rotation to Defaults: /rotateFurniture reset 1");
				return INVALIDPARAMETERS;
			}

			if ((dir == "left" || dir == "right") && (degrees < 1 || degrees > 180)) {
				creature->sendSystemMessage("Using Standard Format: The amount to rotate must be between 1 and 180.");
				return INVALIDPARAMETERS;
			}

			if ((dir == "roll" || dir == "pitch" || dir == "yaw" ) && (degrees < -360 || degrees > 360)) {
				creature->sendSystemMessage("Using Enhanced Format: The amount to rotate must be between -360 and 360.");
				return INVALIDPARAMETERS;
			}

			ZoneServer* zoneServer = creature->getZoneServer();
			ManagedReference<SceneObject*> obj = zoneServer->getObject(target);

			if (obj == NULL || !obj->isTangibleObject() || obj->isPlayerCreature() || obj->isPet()) {
				creature->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
				return GENERALERROR;
			}

			if (obj->isVendor() && !obj->checkContainerPermission(creature, ContainerPermissions::MOVEVENDOR)) {
				return GENERALERROR;
			}

			if (dir == "right"){
				obj->rotate(-degrees);
			}
			else if (dir == "left"){
				obj->rotate(degrees);
			}
			else if (dir == "yaw"){
			obj->rotate(degrees);
			}
			else if (dir == "pitch"){
				obj->rotateYaxis(degrees);
			}
			else if (dir == "roll"){
			obj->rotateXaxis(degrees);
			}
			else if (dir == "reset"){
				obj->setDirection(1, 0, 0, 0);
			}

			obj->incrementMovementCounter();

			if (obj->getParent() != NULL)
				obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY(), obj->getParent().get()->getObjectID());
			else
				obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY());

			return SUCCESS;
		}

	}
};

#endif //WARDEVROTATEOBJECTCOMMAND_H_
