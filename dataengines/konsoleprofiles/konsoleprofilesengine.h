/*****************************************************************************
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#ifndef KONSOLEPROFILESENGINE_H
#define KONSOLEPROFILESENGINE_H

#include <Plasma/DataEngine>

/**
 * This engine provides the current state of the keyboard modifiers
 * and mouse buttons, primarily useful for accessibility feature support.
 */
class KonsoleProfilesEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    KonsoleProfilesEngine(QObject *parent, const QVariantList &args);
    ~KonsoleProfilesEngine();

    void init();
    Plasma::Service *serviceForSource(const QString &source);

protected Q_SLOTS:

private:
};

#endif
