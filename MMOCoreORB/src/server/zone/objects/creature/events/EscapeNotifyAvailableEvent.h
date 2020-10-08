/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ESCAPENOTIFYAVAILABLEEVENT_H_
#define ESCAPENOTIFYAVAILABLEEVENT_H_

#include "server/zone/objects/creature/CreatureObject.h"

class EscapeNotifyAvailableEvent : public Task {
	ManagedWeakReference<CreatureObject*> creo;

public:
	EscapeNotifyAvailableEvent(CreatureObject* cr) : Task() {
		creo = cr;
	}

	void run() {
		ManagedReference<CreatureObject*> creature = creo.get();

		if (creature == NULL)
			return;

		Locker locker(creature);

		creature->removePendingTask("escape_notify");
		creature->sendSystemMessage("You've now recovered from your last narrow escape"); //"You are no longer tired.";
	}

};

#endif /*ESCAPENOTIFYAVAILABLEEVENT_H_*/
