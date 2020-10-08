/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCERESISTSTATESCOMMAND_H_
#define FORCERESISTSTATESCOMMAND_H_

class ForceResistStatesCommand : public JediQueueCommand {
public:

	ForceResistStatesCommand(const String& name, ZoneProcessServer* server)
		: JediQueueCommand(name, server) {

		buffCRC = BuffCRC::JEDI_RESIST_STATES;

		skillMods.put("resistance_states", 60);

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}
		return doJediSelfBuffCommand(creature);
	}

};

#endif //FORCERESISTSTATESCOMMAND_H_
