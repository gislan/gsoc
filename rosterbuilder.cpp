#include <QIcon>
#include <QDebug>

#include "rosterbuilder.h"
#include "group.h"
#include "contact.h"
#include "xmpprosteritem.h"
#include "xmppresource.h"
#include "manager.h"
#include "account.h"
#include "rosterdataservice.h"
#include "resource.h"
#include "roster.h"
#include "metacontact.h"
#include "expanddataservice.h"

namespace Roster {

	RosterBuilder::RosterBuilder(Roster* root, Manager* manager, ExpandDataService* joinedExpandService) : 
																		joinedExpandService_(joinedExpandService), 
																		root_(root), 
																		manager_(manager), 
																		joinedAccounts_(true), 
																		joinByName_(true) {
	}

	void RosterBuilder::rebuild() {
		if ( joinedAccounts_ ) {
			buildJoinedAccounts();
		} else {
			buildAllAccounts();
		}
	}

	void RosterBuilder::setJoinedAccounts(bool joinedAccounts) {
		if ( joinedAccounts == joinedAccounts_ ) {
			return;
		}

		joinedAccounts_ = joinedAccounts;
		rebuild();
	}

	void RosterBuilder::addItem(const XMPPRosterItem* xitem, const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];

		foreach(QString xgroup, xitem->getGroups()) {
			Contact* contact = new Contact(xitem->getName(), xitem->getJid());
			contact->setAccountName(acname);
			contact->setAvatar(srv->getAvatar(contact->getJid()));
			contact->setExpanded(expandServices_[acname]->isContactExpanded(xitem->getJid(), xgroup));

			Group* group = findGroup(xgroup, acname);

			addContact(contact, group);

			foreach(XMPPResource* xresource, xitem->getResources()) {
				Resource* resource = new Resource(xresource->getName(), xresource->getPriority(), xresource->getStatus());
				resource->setAccountName(acname);
				// FIXME: move it to some better place
				if ( xresource->getShow() == STATUS_ONLINE ) {
					resource->setIcon(QIcon("icons/online.png"));
				} else {
					resource->setIcon(QIcon("icons/offline.png"));
				}

				manager_->addResource(resource, contact);
			}
		}
	}

	void RosterBuilder::buildRoster(const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];
		foreach(XMPPRosterItem* xitem, srv->getRosterItems()) { 
			addItem(xitem, acname);
		}
	}

	void RosterBuilder::addContact(Contact* contact, Group* group) {
		if ( ! joinByName_ ) {
			manager_->addContact(contact, group);
		} else {
			if ( Metacontact* metacontact = group->findMetacontact(contact->getName()) ) {
				manager_->addToMetacontact(contact, metacontact);
			} else if ( Contact* similar = group->findContact(contact->getName()) ) {
				Metacontact* metacontact = new Metacontact(contact->getName());
				if ( ! joinedAccounts_ ) {
					metacontact->setAccountName(contact->getAccountName());
				}

				manager_->addMetacontact(metacontact, group);
				manager_->moveContact(similar, metacontact);
				manager_->addToMetacontact(contact, metacontact);
			} else {
				manager_->addContact(contact, group);
			}
		}
	}

	void RosterBuilder::buildAllAccounts() {
		clear(root_);
		foreach(QString name, rosterServices_.keys()) {
			Account* account = new Account(name);
			account->setAccountName(name);
			manager_->addAccount(account, root_);

			buildRoster(name);
		}
	}

	void RosterBuilder::buildJoinedAccounts() {
		clear(root_);
		foreach(QString acname, rosterServices_.keys()) {
			buildRoster(acname);
		}
	}

	void RosterBuilder::clear(Item* item) {
		if ( GroupItem* g = dynamic_cast<GroupItem*>(item) ) {
			foreach(Item* subitem, g->getItems()) {
				clear(subitem);
				manager_->removeItem(subitem);
			}
		}
	}

	Group* RosterBuilder::addGroup(const QString& groupName, const QString& acname, GroupItem* parent) {
		Group* group = new Group(groupName);
		if ( ! joinedAccounts_ ) {
			group->setAccountName(acname);
		}

		ExpandDataService* srv;
		if ( group->getAccountName().isEmpty() ) {
			srv = joinedExpandService_;
		} else {
			srv = expandServices_[acname];
		}

		QString fullPath = parent->getGroupPath();
		if ( ! fullPath.isEmpty() ) {
			fullPath += SEPARATOR;
		}
		fullPath += group->getName();

		group->setExpanded(srv->isGroupExpanded(fullPath));

		manager_->addGroup(group, parent);
		return group;
	}

	/* create (if it doesn't exist yet) and return group with given name on given account */
	Group* RosterBuilder::findGroup(const QString& groupName, const QString& acname, bool create) {
		QList<QString> groupNames = groupName.split(SEPARATOR);

		GroupItem* up = root_->findAccount(acname);
		if ( ! up ) {
			up = root_;
		}

		foreach(QString name, groupNames) {
			Group* next = up->findGroup(name);

			if ( ! next ) {
				if ( create ) {
					next = addGroup(name, acname, up);
				} else {
					return 0;
				}
			}


			up = next;
		}

		return static_cast<Group*>(up);
	}

	void RosterBuilder::registerAccount(const QString& acname, RosterDataService* rosterService, ExpandDataService* expService) {
		rosterServices_.insert(acname, rosterService);
		expandServices_.insert(acname, expService);
	}

	void RosterBuilder::setJoinByName(bool joinByName) {
		if ( joinByName == joinByName_ ) {
			return;
		}

		joinByName_ = joinByName;
		rebuild();
	}

	void RosterBuilder::itemAdded(const XMPPRosterItem* xitem, const QString& acname) {
		addItem(xitem, acname);
	}

	void RosterBuilder::itemRemoved(const XMPPRosterItem* xitem, const QString& acname) {
		QList<Contact*> contacts = findContacts(xitem, acname);

		foreach(Contact* contact, contacts) {
			manager_->removeContact(contact);
		}
	}

	void RosterBuilder::itemChanged(const XMPPRosterItem* xitem, const QString& acname) {
	}

	QList<Contact*> RosterBuilder::findContacts(const XMPPRosterItem* xitem, const QString& acname) {
		QList<Contact*> contacts;

		foreach(QString xgroup, xitem->getGroups()) {
			Group* group = findGroup(xgroup, acname, false);

			if ( ! group ) {
				continue;
			}

			if ( joinByName_ ) {
			   	if ( Metacontact* metacontact = group->findMetacontact(xitem->getName()) ) {
					if ( Contact* contact = metacontact->findContact(xitem->getName()) ) {
						contacts.append(contact);
					}
				}
			}

			if ( Contact* contact = group->findContact(xitem->getName()) ) {
				contacts.append(contact);
			}
		}

		return contacts;
	}
}

