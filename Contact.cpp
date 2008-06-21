#include <QIcon>

#include "Contact.h"

namespace Roster {

	Contact::Contact(const QString& name, const QString& jid) : name_(name), jid_(jid) {
	}

	Contact::~Contact() {
	}

	const QString& Contact::getName() const {
		return name_;
	}

	const QString& Contact::getJid() const {
		return jid_;
	}

	void Contact::setName(const QString& name) {
		name_ = name;
	}

	void Contact::setJid(const QString& jid) {
		jid_ = jid;
	}

	const QIcon& Contact::getIcon() const {
		return icon_;
	}

	void Contact::setIcon(const QIcon& icon) {
		icon_ = icon;
	}

	const QIcon& Contact::getAvatar() const {
		return avatar_;
	}

	void Contact::setAvatar(const QIcon& avatar) {
		avatar_ = avatar;
	}
}

