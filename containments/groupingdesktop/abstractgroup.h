/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ABSTRACTGROUP_H
#define ABSTRACTGROUP_H

#include <Plasma/Applet>

class KConfigGroup;

class AbstractGroupPrivate;

class PLASMA_EXPORT AbstractGroup : public Plasma::Applet
{
    Q_OBJECT
    public:
        /**
        * Constructor of the abstract class.
        **/
        AbstractGroup(QObject *parent = 0, const QVariantList &args = QVariantList());
        /**
        * Default destructor
        **/
        ~AbstractGroup();

        /**
        * Reimplemented from Plasma::Applet
        **/
        void init();

        /**
        * Assignes an Applet to this Group
        * @param applet the applet to be managed by this
        * @param layoutApplets if true calls layoutApplet(applet)
        **/
        void assignApplet(Plasma::Applet *applet, bool layoutApplets = true);

        /**
        * Saves the group's specific configurations for an applet.
        * This function must be reimplemented by a child class.
        * @param applet the applet which will be saved
        * @param group the config group for the configuration
        **/
        virtual void saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) const = 0;

        /**
        * Restores the group's specific configurations for an applet.
        * This function must be reimplemented by a child class.
        * @param applet the applet which will be restored
        * @param group the config group for the configuration
        **/
        virtual void restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group) const = 0;

        /**
        * Lay outs an applet inside the group
        * This function must be reimplemented by a child class.
        * @param applet the applet to be layed out
        **/
        virtual void layoutApplet(Plasma::Applet *applet) = 0;

        /**
        * Destroyed this groups and its applet, deleting the configurations too
        **/
        void destroy();

        /**
        * Reimplemented from Plasma::Applet
        **/
        KConfigGroup config();

        /**
        * Reimplemented from Plasma::Applet
        **/
        void save(KConfigGroup &group) const;

        /**
        * Used to have a list of the applets managed by this group
        * @return the list of the applets
        **/
        Plasma::Applet::List assignedApplets() const;

    protected:
        /**
        * Reimplemented from QGraphicsItem
        **/
        void dropEvent(QGraphicsSceneDragDropEvent *event);

    private slots:
        /**
        * @internal slot called when an applet is removed
        **/
        void onAppletRemoved(Plasma::Applet *applet);

    private:
        AbstractGroupPrivate *const d;
};

#endif // ABSTRACTGROUP_H
