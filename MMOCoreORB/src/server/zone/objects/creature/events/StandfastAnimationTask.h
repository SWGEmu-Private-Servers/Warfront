/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef StandfastANIMATIONTASK_H_
#define StandfastANIMATIONTASK_H_

#include "server/zone/objects/creature/CreatureObject.h"

namespace server {
namespace zone {
namespace objects {
namespace creature {
namespace events {

class StandfastAnimationTask : public Task {

	ManagedReference<CreatureObject*> player;
	int countDown;
	
public:
	StandfastAnimationTask(CreatureObject* player, int countDown) : Task() { 
		this->player = player;
		this->countDown = countDown;
	}

	void run() {

		if( player == NULL )
			return;

		Locker locker(player);

		player->removePendingTask("Standfast_animation");
		
		// If buff is still active, perform animation and reschedule
		if (countDown > 0) 
			{
			countDown = countDown -1;
			player->playEffect("clienteffect/commando_position_secured.cef", "");
			//reschedule( 9000 );
			Reference<Task*> standfastAnimationTask = new StandfastAnimationTask(player, countDown); //crc,
			player->addPendingTask("standfast_animation", standfastAnimationTask, 9000);
			}
	}
};

} // events
} // creature
} // objects
} // zone
} // server

using namespace server::zone::objects::creature::events;

#endif /*STANDFASTANIMATIONTASK_H_*/
