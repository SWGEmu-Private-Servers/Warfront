
#ifndef LAGCOMMAND_H_
#define LAGCOMMAND_H_

#include "engine/engine.h"
#include "server/login/account/AccountManager.h"
#include "server/login/account/Account.h"

class LagCommand : public QueueCommand {
public:

	LagCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!creature->isPlayerCreature())
			return 1;

		String account;
		String password;
		StringTokenizer tokenizer(arguments.toString());
		if (!tokenizer.hasMoreTokens()) {
			sendSyntax(creature);
			return 1;
		}
		tokenizer.getStringToken(account);
		account = account.toLowerCase();

		if (!tokenizer.hasMoreTokens()) {
			sendSyntax(creature);
			return 1;
		}

		tokenizer.getStringToken(password);

		LoginServer* loginServer = DistributedObjectBroker::instance()->lookUp("LoginServer").castTo<LoginServer*>().get();

		if (loginServer == NULL) {
			return 1;
		}


		AccountManager* am = loginServer->getAccountManager();

		if (am == NULL) {
			return 1;
		}

		String storedPassword;

		ManagedReference<Account*> a = NULL;
		a = am->getAccount(account, true);
		if (a != NULL) {
			creature->sendSystemMessage("Account already exists: Failed to create account!");
			return 1;
		}

		a = am->createAccount(account, password, storedPassword);
		if (a == NULL) {
			creature->sendSystemMessage("Failed to create account!");
		} else {
			creature->sendSystemMessage("Account " + account + " created!");
		}


		return 0;
	}

	static void sendSyntax(CreatureObject* player) {
		if (player != NULL)
			player->sendSystemMessage("Syntax: /server registerAccount <accountname> <password>");
	}

};

#endif //LAGCOMMAND_H_
