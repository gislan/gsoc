#include "viewactionsservice.h"
#include "psiaccount.h"
#include "rosteractionsservice.h"
#include "contact.h"
#include "account.h"

/* 
 * This file is silly. 
 * Repeating yourself 100 times is not what I like
 * FIXME: do something about it
 */

namespace Roster {

	ViewActionsService::ViewActionsService() {
	}

	ViewActionsService::~ViewActionsService() {
	}

	void ViewActionsService::registerAccount(const QString& acname, PsiAccount* acc) {
		RosterActionsService* manager = new RosterActionsService(acc);
		managers_.insert(acname, manager);
	}

	void ViewActionsService::unregisterAccount(const QString& acname) {
		managers_.remove(acname);
	}

	void ViewActionsService::sendMessage(Contact* contact) {
		managers_[contact->getAccountName()]->sendMessage(contact->getJid());
	}

	void ViewActionsService::openChat(Contact* contact) {
		managers_[contact->getAccountName()]->openChat(contact->getJid());
	}

	void ViewActionsService::executeCommand(Contact* contact) {
		managers_[contact->getAccountName()]->executeCommand(contact->getJid());
	}

	void ViewActionsService::sendFile(Contact* contact) {
		managers_[contact->getAccountName()]->sendFile(contact->getJid());
	}

	void ViewActionsService::removeContact(Contact* contact) {
		managers_[contact->getAccountName()]->removeContact(contact->getJid());
	}

	void ViewActionsService::showHistory(Contact* contact) {
		managers_[contact->getAccountName()]->showHistory(contact->getJid());
	}

	void ViewActionsService::userInfo(Contact* contact) {
		managers_[contact->getAccountName()]->userInfo(contact->getJid());
	}

	void ViewActionsService::openWhiteboard(Contact* contact) {
		managers_[contact->getAccountName()]->openWhiteboard(contact->getJid());
	}

	void ViewActionsService::resendAuthTo(Contact* contact) {
		managers_[contact->getAccountName()]->resendAuthTo(contact->getJid());
	}

	void ViewActionsService::rerequestAuthFrom(Contact* contact) {
		managers_[contact->getAccountName()]->rerequestAuthFrom(contact->getJid());
	}

	void ViewActionsService::removeAuthFrom(Contact* contact) {
		managers_[contact->getAccountName()]->removeAuthFrom(contact->getJid());
	}

	void ViewActionsService::assignAvatar(Contact* contact, const QString& file) {
		managers_[contact->getAccountName()]->assignAvatar(contact->getJid(), file);
	}

	void ViewActionsService::clearAvatar(Contact* contact) {
		managers_[contact->getAccountName()]->clearAvatar(contact->getJid());
	}

	void ViewActionsService::changeStatus(Account* account, const StatusType status) {
		managers_[account->getAccountName()]->changeStatus(status);
	}

	void ViewActionsService::newBlankMessage(Account* account) {
		XMPP::Jid jid;
		managers_[account->getAccountName()]->sendMessage(jid);
	}

	void ViewActionsService::serviceDiscovery(Account* account) {
		managers_[account->getAccountName()]->serviceDiscovery(account->getJid(), "");
	}

	void ViewActionsService::mood(Account* account) {
		managers_[account->getAccountName()]->mood();
	}

	void ViewActionsService::setAvatar(Account* account) {
		managers_[account->getAccountName()]->setAvatar();
	}

	void ViewActionsService::unsetAvatar(Account* account) {
		managers_[account->getAccountName()]->unsetAvatar();
	}

	void ViewActionsService::modifyAccount(Account* account) {
		managers_[account->getAccountName()]->modifyAccount();
	}

	void ViewActionsService::addContact(Account* account) {
		managers_[account->getAccountName()]->addContact();
	}

	void ViewActionsService::xmlConsole(Account* account) {
		managers_[account->getAccountName()]->xmlConsole();
	}

}
