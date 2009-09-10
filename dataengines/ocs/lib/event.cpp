/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "event.h"


using namespace Attica;

class Event::Private : public QSharedData {
    public:
        QString m_id;
        QString m_name;
        QString m_description;
        QString m_user;
        QDate m_startDate;
        QDate m_endDate;
        qreal m_latitude;
        qreal m_longitude;
        QString m_homepage;
        QString m_country;
        QString m_city;
        QMap<QString, QString> m_extendedAttributes;

        Private()
            : m_latitude(0),
              m_longitude(0)
        {
        }
};


Event::Event()
    : d(new Private)
{
}

Event::Event(const Event& other)
    : d(other.d)
{
}

Event& Event::operator=(const Event& other)
{
    d = other.d;
    return *this;
}

Event::~Event()
{
}


void Event::setId(const QString& id)
{
    d->m_id = id;
}

QString Event::id() const
{
    return d->m_id;
}


void Event::setName(const QString& name)
{
    d->m_name = name;
}

QString Event::name() const
{
    return d->m_name;
}


void Event::setDescription(const QString& text)
{
    d->m_description = text;
}

QString Event::description() const
{
    return d->m_description;
}


void Event::setUser(const QString& id)
{
    d->m_user = id;
}

QString Event::user() const
{
    return d->m_user;
}


void Event::setStartDate(const QDate& date)
{
    d->m_startDate = date;
}

QDate Event::startDate() const
{
    return d->m_startDate;
}


void Event::setEndDate(const QDate& date)
{
    d->m_endDate = date;
}

QDate Event::endDate() const
{
    return d->m_endDate;
}


void Event::setLatitude(qreal lat)
{
    d->m_latitude = lat;
}

qreal Event::latitude() const
{
    return d->m_latitude;
}


void Event::setLongitude(qreal lon)
{
    d->m_longitude = lon;
}

qreal Event::longitude() const
{
    return d->m_longitude;
}


void Event::setHomepage(const QString& url)
{
    d->m_homepage = url;
}

QString Event::homepage() const
{
    return d->m_homepage;
}


void Event::setCountry(const QString& country)
{
    d->m_country = country;
}

QString Event::country() const
{
    return d->m_country;
}


void Event::setCity(const QString& city)
{
    d->m_city = city;
}

QString Event::city() const
{
    return d->m_city;
}


void Event::addExtendedAttribute(const QString& key, const QString& value)
{
    d->m_extendedAttributes.insert(key, value);
}

QString Event::extendedAttribute(const QString& key) const
{
    return d->m_extendedAttributes.value(key);
}

QMap<QString, QString> Event::extendedAttributes() const
{
    return d->m_extendedAttributes;
}
