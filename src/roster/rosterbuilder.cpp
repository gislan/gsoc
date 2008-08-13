#include <QIcon>
#include <QDebug>

#include "rosterbuilder.h"
#include "group.h"
#include "contact.h"
#include "manager.h"
#include "account.h"
#include "rosterdataservice.h"
#include "resource.h"
#include "roster.h"
#include "metacontact.h"
#include "viewstatemanager.h"
#include "transport.h"
#include "self.h"
#include "userlist.h"

namespace Roster {

	RosterBuilder::RosterBuilder(Roster* root, Manager* manager, ViewStateManager* vsm) : 
																		vsm_(vsm),
																		root_(root), 
																		manager_(manager), 
																		joinedAccounts_(false), 
																		joinByName_(false),
																		itemFilter_(FILTER_OFFLINE)	{
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

	void RosterBuilder::setShowOffline(bool show) {
		if ( show ) {
			setFilter(getFilter() & (~FILTER_OFFLINE));
		} else {
			setFilter(getFilter() | FILTER_OFFLINE);
		}
	}

	unsigned int RosterBuilder::getFilter() {
		return itemFilter_;
	}

	void RosterBuilder::setFilter(unsigned int itemFilter) {
		itemFilter_ = itemFilter;
		rebuild();
	}

	void RosterBuilder::updateResources(const UserResourceList list, GroupItem* groupItem) {
		QSet<QString> names; // for deleting later
		foreach(UserResource xresource, list) {
			Resource* resource = groupItem->findResource(xresource.name());

			if ( resource ) {
				if ( resource->getStatus() != xresource.status().type() ) {
					manager_->setStatus(resource, xresource.status().type());
				}
				if ( resource->getStatusMessage() != xresource.status().status() ) {
					manager_->setStatusMessage(resource, xresource.status().status());
				}
			} else {
				Resource* resource = new Resource(xresource.name(), xresource.status().priority(), 
						xresource.status().type(), xresource.status().status());
				resource->setAccountName(groupItem->getAccountName());
				manager_->addResource(resource, groupItem);
			}

			names.insert(xresource.name());
		}

		// delete obsolete resources
		foreach(Item* item, groupItem->getItems()) {
			Resource* resource = static_cast<Resource*>(item);
			if ( ! names.contains(resource->getName()) ) {
				manager_->removeResource(resource);
				delete resource;
			}
		}
	}

	void RosterBuilder::updateContact(const UserListItem* xitem, const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];

		QList<QString> groups = xitem->groups();
		if ( groups.isEmpty() ) {
			groups.append("General");
		}

		foreach(QString xgroup, groups) {
			GroupItem* parent = findGroup(xgroup, acname);

			if ( joinByName_ and parent ) {
				Metacontact* metacontact = parent->findMetacontact(xitem->name());
				if ( metacontact ) {
					parent = metacontact;
				}
			}

			Contact* contact = parent ? parent->findContact(xitem->jid(), acname) : 0;

			if ( isContactVisible(xitem) ) {
				if ( ! contact ) {
					QString name = xitem->name().isEmpty() ? xitem->jid().full() : xitem->name();
					contact = new Contact(name, xitem->jid());
					contact->setAccountName(acname);
					
					if ( ! parent ) {
						parent = findGroup(xgroup, acname, true);
					}

					if ( ! joinByName_ or dynamic_cast<Metacontact*>(parent) ) {
						parent = findGroup(xgroup, acname, true);
						manager_->addContact(contact, parent);
					} else {
						if ( Contact* similar = parent->findContact(xitem->name()) ) {
							Metacontact* metacontact = addMetacontact(contact->getName(), parent->getAccountName(), parent);

							manager_->removeContact(similar);
							manager_->addContact(similar, metacontact);
							manager_->addContact(contact, metacontact);
						} else {
							parent = findGroup(xgroup, acname, true);
							manager_->addContact(contact, parent);
						}
					}
				}

				// update contact data
				manager_->setAvatar(contact, srv->getAvatar(contact->getJid()));
				if ( contact->isExpanded() != vsm_->isContactExpanded(contact) ) {
					manager_->updateState(contact, vsm_->isContactExpanded(contact));
				}

				updateResources(xitem->userResourceList(), contact);
			} else {
				if ( contact ) {
					manager_->removeContact(contact);
					delete contact;

					if ( Metacontact* metacontact = dynamic_cast<Metacontact*>(parent) ) {
						if ( metacontact->getNbItems() == 1 ) {
							GroupItem* grandparent = metacontact->getParent();

							Contact* lastContact = dynamic_cast<Contact*>(metacontact->getItems().at(0));
							manager_->removeContact(lastContact);
							manager_->removeMetacontact(metacontact);
							delete metacontact;

							manager_->addContact(lastContact, grandparent);
						}
					}
				}
			}
		}
	}

	void RosterBuilder::updateTransport(const UserListItem* xitem, const QString& acname) {
		Group* group = findGroup("Agents/Transports", acname, false);
		Transport* transport = group ? group->findTransport(xitem->jid(), acname) : 0;
		if ( ! isTransportVisible(xitem) ) {
			if ( transport ) {
				manager_->removeTransport(transport);
				delete transport;
			}
		} else {
			if ( ! transport ) {
				Group* group = findGroup("Agents/Transports", acname, true);
				QString name = xitem->name().isEmpty() ? xitem->jid().full() : xitem->name();
				transport = new Transport(name, xitem->jid());
				transport->setAccountName(acname);
				manager_->addTransport(transport, group);
			}

			if ( transport->isExpanded() != vsm_->isTransportExpanded(transport) ) {
				manager_->updateState(transport, vsm_->isTransportExpanded(transport));
			}

			updateResources(xitem->userResourceList(), transport);
		}
	}

	void RosterBuilder::buildRoster(const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];

		foreach(UserListItem* xitem, srv->getRosterItems()) { // FIXME: isn't this calling getRosterItems many times?
			if ( srv->isTransport(xitem->jid()) ) {
				updateTransport(xitem, acname);
			} else {
				updateContact(xitem, acname);
			}
		}

		updateSelf(srv->getSelf(), acname);
	}

	Metacontact* RosterBuilder::addMetacontact(const QString& name, const QString& acname, GroupItem* parent) {
		Metacontact* metacontact = new Metacontact(name);
		if ( ! joinedAccounts_ ) {
			metacontact->setAccountName(acname);
		}

		manager_->addMetacontact(metacontact, parent);
		manager_->updateState(metacontact, vsm_->isMetacontactExpanded(metacontact));
		return metacontact;
	}

	void RosterBuilder::buildAllAccounts() {
		clear(root_);
		foreach(QString name, rosterServices_.keys()) {
			if ( rosterServices_[name]->isEnabled() ) {
				updateAccount(name);
				buildRoster(name);
			}
		}
	}

	void RosterBuilder::updateAccount(const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];
		if ( srv->isEnabled() and ! joinedAccounts_ ) {
			Account* account = root_->findAccount(acname);
			
			if ( ! account ) {
				account = new Account(acname, srv->getJid());
				account->setAccountName(acname);
				manager_->addAccount(account, root_);
			}

			manager_->updateState(account, vsm_->isAccountExpanded(account));
			if ( account->getStatus() != srv->getStatus() ) {
				manager_->setStatus(account, srv->getStatus());
			}
		}
	}

	void RosterBuilder::buildJoinedAccounts() {
		clear(root_);
		foreach(QString acname, rosterServices_.keys()) {
			if ( rosterServices_[acname]->isEnabled() ) {
				buildRoster(acname);
			}
		}
	}

	void RosterBuilder::clear(Item* item) {
		if ( GroupItem* g = dynamic_cast<GroupItem*>(item) ) {
			foreach(Item* subitem, g->getItems()) {
				clear(subitem);
				manager_->removeItem(subitem);
				delete subitem;
			}
		}
	}

	Group* RosterBuilder::addGroup(const QString& groupName, const QString& acname, GroupItem* parent) {
		Group* group = new Group(groupName);
		if ( ! joinedAccounts_ ) {
			group->setAccountName(acname);
		}

		manager_->addGroup(group, parent);

		if ( group->isExpanded() != vsm_->isGroupExpanded(group) ) {
			manager_->updateState(group, vsm_->isGroupExpanded(group));
		}

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

	void RosterBuilder::registerAccount(const QString& acname, RosterDataService* rosterService) {
		rosterServices_.insert(acname, rosterService);

		connect(rosterService, SIGNAL(itemUpdated(const UserListItem*, const QString&)), 
				SLOT(itemChanged(const UserListItem*, const QString&)));
		connect(rosterService, SIGNAL(selfUpdated(const UserListItem*, const QString&)), 
				SLOT(selfChanged(const UserListItem*, const QString&)));
		connect(rosterService, SIGNAL(accountUpdated(const QString&)), SLOT(accountChanged(const QString&)));
	}

	void RosterBuilder::setJoinByName(bool joinByName) {
		if ( joinByName == joinByName_ ) {
			return;
		}

		joinByName_ = joinByName;
		rebuild();
	}

	void RosterBuilder::itemAdded(const UserListItem* xitem, const QString& acname) {
		Q_UNUSED(xitem); Q_UNUSED(acname);
		// FIXME: no real stuff here since this is currently not used
	}

	void RosterBuilder::itemRemoved(const UserListItem* xitem, const QString& acname) {
		Q_UNUSED(xitem); Q_UNUSED(acname);
		// FIXME: no real stuff here since this is currently not used
	}

	const bool RosterBuilder::isContactVisible(const UserListItem* xitem) const {
		if ( ! searchText_.isEmpty() ) {
			if ( xitem->name().toLower().contains(searchText_.toLower()) ) {
				return true;
			} else {
				return false;
			}
		}

		foreach(UserResource xres, xitem->userResourceList()) {
			if ( xres.status().type() == STATUS_ONLINE or xres.status().type() == STATUS_CHAT ) {
				return true;
			} else if ( xres.status().type() == STATUS_DND and !(itemFilter_ & FILTER_DND) ) {
				return true;
			} else if ( xres.status().type() == STATUS_AWAY and !(itemFilter_ & FILTER_AWAY) ) {
				return true;
			} else if ( xres.status().type() == STATUS_XA and !(itemFilter_ & FILTER_XA) ) {
				return true;
			}
		}

		return ! (itemFilter_ & FILTER_OFFLINE);
	}

	const bool RosterBuilder::isTransportVisible(const UserListItem* xitem) const {
		if ( ! searchText_.isEmpty() ) {
			if ( xitem->name().toLower().contains(searchText_.toLower()) ) {
				return true;
			} else {
				return false;
			}
		}
		return ! (itemFilter_ & FILTER_TRANSPORTS);
	}

	void RosterBuilder::itemChanged(const UserListItem* xitem, const QString& acname) {
		RosterDataService* srv = rosterServices_[acname];

		if ( ! srv->isEnabled() ) {
			return;
		}

		if ( srv->isTransport(xitem->jid()) ) {
			updateTransport(xitem, acname);
		} else {
			updateContact(xitem, acname);
		}
	}

	void RosterBuilder::updateSelf(const UserListItem* xitem, const QString& acname) {
		GroupItem* acc = root_->findAccount(acname);
		if ( ! acc ) {
			acc = root_;
		}

		Self* self = acc->findSelf(acname);

		if ( itemFilter_ & FILTER_SELF ) {
			if ( self ) {
				manager_->removeSelf(self);
				delete self;
			}
		} else {
			if ( ! self ) {
				self = new Self(xitem->name(), xitem->jid());
				self->setAccountName(acname);

				manager_->addSelf(self, acc);
			}

			if ( self->isExpanded() != vsm_->isSelfExpanded(self) ) {
				manager_->updateState(self, vsm_->isSelfExpanded(self));
			}

			updateResources(xitem->userResourceList(), self);	
		}
	}

	void RosterBuilder::accountChanged(const QString& acname) {
		updateAccount(acname);
	}

	void RosterBuilder::selfChanged(const UserListItem* xitem, const QString& acname) {
		if ( rosterServices_[acname]->isEnabled() ) {
			updateSelf(xitem, acname);
		}
	}

	void RosterBuilder::setSearch(const QString& searchText) {
		searchText_ = searchText;
		rebuild();
	}

	void RosterBuilder::setShowSelf(bool show) {
		if ( show ) {
			itemFilter_ = itemFilter_ & (~FILTER_SELF);
		} else {
			itemFilter_ = itemFilter_ | FILTER_SELF;
		}
	
		foreach(QString acname, rosterServices_.keys()) {
			updateSelf(rosterServices_[acname]->getSelf(), acname);
		}
	}

	void RosterBuilder::unregisterAccount(const QString& acname) {
		rosterServices_[acname]->disconnect();
		delete rosterServices_[acname];
		rosterServices_.remove(acname);
	}

}

