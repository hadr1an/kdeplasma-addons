/*
 *   Copyright 2007,2008 by Alex Merry <alex.merry@kdemail.net>
 *
 *   Some code (text size calculation) taken from clock applet:
 *   Copyright 2007 by Sebastian Kuegler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#include "nowplaying.h"
#include "controls.h"
#include "infopanel.h"

#include <Plasma/Slider>

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>


K_EXPORT_PLASMA_APPLET(nowplaying, NowPlaying)


NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_controller(0),
      m_state(NoPlayer),
      m_currentLayout(NoLayout),
      m_volume(0),
      m_length(0),
      m_textPanel(new InfoPanel),
      m_buttonPanel(new Controls),
      m_volumeSlider(new Plasma::Slider(this)),
      m_positionSlider(new Plasma::Slider(this))
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(300, 200);
    setMinimumSize(300, 200);

    connect(m_buttonPanel, SIGNAL(play()), this, SLOT(play()));
    connect(m_buttonPanel, SIGNAL(pause()), this, SLOT(pause()));
    connect(m_buttonPanel, SIGNAL(stop()), this, SLOT(stop()));
    connect(m_buttonPanel, SIGNAL(previous()), this, SLOT(prev()));
    connect(m_buttonPanel, SIGNAL(next()), this, SLOT(next()));
    connect(this, SIGNAL(stateChanged(State)),
            m_buttonPanel, SLOT(stateChanged(State)));
    connect(this, SIGNAL(controllerChanged(Plasma::Service*)),
            m_buttonPanel, SLOT(setController(Plasma::Service*)));

    connect(this, SIGNAL(metadataChanged(QMap<QString,QString>)),
            m_textPanel, SLOT(updateMetadata(QMap<QString,QString>)));

    m_volumeSlider->setOrientation(Qt::Vertical);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(0);
    connect(this, SIGNAL(volumeChanged(int)),
            m_volumeSlider, SLOT(setValue(int)));
    connect(m_volumeSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(setVolume(int)));
    m_volumeSlider->setEnabled(false);

    m_positionSlider->setOrientation(Qt::Horizontal);
    m_positionSlider->setMinimum(0);
    m_positionSlider->setMaximum(0);
    m_positionSlider->setValue(0);
    connect(this, SIGNAL(positionChanged(int)),
            m_positionSlider, SLOT(setValue(int)));
    connect(this, SIGNAL(lengthChanged(int)),
            m_positionSlider, SLOT(setMaximum(int)));
    connect(m_positionSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(setPosition(int)));
    m_positionSlider->setEnabled(false);
}

NowPlaying::~NowPlaying()
{
}

void NowPlaying::init()
{
    switch (formFactor())
    {
        case Plasma::Horizontal:
            layoutHorizontal();
            break;
        case Plasma::Vertical:
            layoutHorizontal(); // FIXME
            break;
        default:
            layoutPlanar();
            break;
    }
    Plasma::DataEngine* nowPlayingEngine = dataEngine("nowplaying");
    if ( nowPlayingEngine )
    {
        connect(nowPlayingEngine, SIGNAL(sourceAdded(QString)),
                SLOT(playerAdded(QString)));
        connect(nowPlayingEngine, SIGNAL(sourceRemoved(QString)),
                SLOT(playerRemoved(QString)));
    }
    else
        kDebug()<<" We can not get now playing engine";
    findPlayer();
}

void NowPlaying::layoutPlanar()
{
    if (m_currentLayout != PlanarLayout)
    {
        setMinimumSize(300, 200);
        setAspectRatioMode(Plasma::IgnoreAspectRatio);

        QGraphicsGridLayout* layout = new QGraphicsGridLayout();
        m_textPanel->show();
        layout->addItem(m_textPanel, 0, 0);
        m_buttonPanel->show();
        m_buttonPanel->setDisplayedButtons(Controls::AllButtons);
        layout->addItem(m_buttonPanel, 1, 0);
        m_positionSlider->show();
        layout->addItem(m_positionSlider, 2, 0);
        m_volumeSlider->show();
        layout->addItem(m_volumeSlider, 0, 1, 3, 1); // rowspan, colspan

        QGraphicsLayout* oldLayout = this->layout();
        setLayout(layout);
        delete oldLayout;

        m_currentLayout = PlanarLayout;
    }
}

void NowPlaying::layoutHorizontal()
{
    if (m_currentLayout != HorizontalLayout)
    {
        setMinimumSize(20, 10);
        resize(preferredHeight() * 2, preferredHeight());
        kDebug() << "preferredHeight():" << preferredHeight();
        kDebug() << "preferredSize():" << preferredSize();
        setAspectRatioMode(Plasma::KeepAspectRatio);

        m_textPanel->hide();
        m_positionSlider->hide();
        m_volumeSlider->hide();

        QGraphicsLinearLayout* layout = new QGraphicsLinearLayout();
        m_buttonPanel->show();
        kDebug() << "m_buttonPanel->preferredSize():" << m_buttonPanel->preferredSize();
        m_buttonPanel->setDisplayedButtons(Controls::PlayPauseButton | Controls::NextButton);
        layout->addItem(m_buttonPanel);

        QGraphicsLayout* oldLayout = this->layout();
        setLayout(layout);
        delete oldLayout;

        m_currentLayout = HorizontalLayout;
    }
}

QSizeF NowPlaying::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    kDebug() << "Asked for size hint" << which << "with constraint" << constraint;
    return Applet::sizeHint(which, constraint);
}

void NowPlaying::constraintsUpdated(Plasma::Constraints constraints)
{
    kDebug() << "Constraints:" << constraints;
    kDebug() << "Maximum size:" << maximumSize();
    kDebug() << "Preferred size:" << preferredSize();
    if (constraints & Plasma::FormFactorConstraint)
    {
        switch (formFactor())
        {
            case Plasma::Horizontal:
                layoutHorizontal();
                break;
            case Plasma::Vertical:
                layoutHorizontal(); // FIXME
                break;
            default:
                layoutPlanar();
                break;
        }
    }
    if (constraints & Plasma::SizeConstraint && formFactor() == Plasma::Horizontal)
    {
        resize(preferredHeight() * 2, preferredHeight());
        setAspectRatioMode(Plasma::KeepAspectRatio);
    }
}

void NowPlaying::dataUpdated(const QString &name,
                             const Plasma::DataEngine::Data &data)
{
                    //i18n("No media player found")
                    //i18nc("The state of a music player", "Stopped")
    if (name != m_watchingPlayer) {
        kDebug() << "Wasn't expecting an update from" << name;
        return;
    }
    if (data.isEmpty()) {
        kDebug() << "Got no data";
        findPlayer();
        return;
    }

    State newstate;
    if (data["State"].toString() == "playing") {
        newstate = Playing;
    } else if (data["State"].toString() == "paused") {
        newstate = Paused;
    } else {
        newstate = Stopped;
    }
    if (newstate != m_state) {
        emit stateChanged(newstate);
        m_state = newstate;
    }

    QString timeText;
    int length = data["Length"].toInt();
    if (length != m_length) {
        m_length = length;
        if (length == 0) {
            emit positionChanged(0);
        }
        emit lengthChanged(m_length);
    }
    if (length != 0) {
        int pos = data["Position"].toInt();
        timeText = QString::number(pos / 60) + ':' +
                   QString::number(pos % 60).rightJustified(2, '0') + " / " +
                   QString::number(length / 60) + ':' +
                   QString::number(length % 60).rightJustified(2, '0');
        // we assume it's changed
        emit positionChanged(pos);
    }

    QMap<QString,QString> metadata;
    metadata["Artist"] = data["Artist"].toString();
    metadata["Album"] = data["Album"].toString();
    metadata["Title"] = data["Title"].toString();
    metadata["Time"] = timeText;
    metadata["Track number"] = QString::number(data["Track number"].toInt());
    metadata["Comment"] = data["Comment"].toString();
    metadata["Genre"] = data["Genre"].toString();

    // the time should usually have changed
    emit metadataChanged(metadata);

    // TODO: we should set a tooltip with the timeText on the position slider

    if (data["Volume"].toDouble() != m_volume) {
        m_volume = data["Volume"].toDouble();
        emit volumeChanged(m_volume * 100);
    }

    // used for seeing when the track has changed
    QString track = metadata["Artist"] + " - " + metadata["Title"];

    // assume the artwork didn't change unless the track did
    if (track != m_track) {
        m_track = track;

        m_artwork = data["Artwork"].value<QPixmap>();
        emit coverChanged(m_artwork);
    }

    update();
}

void NowPlaying::playerAdded(const QString &name)
{
    kDebug() << "Player" << name << "added";
    if (m_watchingPlayer.isEmpty()) {
        kDebug() << "Installing" << name << "as watched player";
        m_watchingPlayer = name;
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 500);
    }
}

void NowPlaying::playerRemoved(const QString &name)
{
    kDebug() << "Player" << name << "removed";
    if (m_watchingPlayer == name) {
        findPlayer();
    }
}

void NowPlaying::findPlayer()
{
    QStringList players = dataEngine("nowplaying")->sources();
    kDebug() << "Looking for players.  Possibilities:" << players;
    if (players.isEmpty()) {
        m_state = NoPlayer;
        m_watchingPlayer.clear();
        m_controller = 0;

        emit stateChanged(m_state);
        emit controllerChanged(0);
        m_positionSlider->setEnabled(false);
        m_volumeSlider->setEnabled(false);
        update();
    } else {
        m_watchingPlayer = players.first();

        m_controller = dataEngine("nowplaying")->serviceForSource(m_watchingPlayer);
        m_controller->associateWidget(m_positionSlider, "seek");
        m_controller->associateWidget(m_volumeSlider, "volume");
        emit controllerChanged(m_controller);

        kDebug() << "Installing" << m_watchingPlayer << "as watched player";
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 999);
    }
}

void NowPlaying::play()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("play"));
    }
}

void NowPlaying::pause()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("pause"));
    }
}

void NowPlaying::stop()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("stop"));
    }
}

void NowPlaying::prev()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("previous"));
    }
}

void NowPlaying::next()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("next"));
    }
}

void NowPlaying::setVolume(int volumePercent)
{
    qreal volume = ((qreal)qBound(0, volumePercent, 100)) / 100;
    if (m_controller) {
        KConfigGroup op = m_controller->operationDescription("volume");
        op.writeEntry("level", volume);
        m_controller->startOperationCall(op);
    }
}

void NowPlaying::setPosition(int position)
{
    if (m_controller) {
        KConfigGroup op = m_controller->operationDescription("seek");
        op.writeEntry("seconds", position);
        m_controller->startOperationCall(op);
    }
}

#include "nowplaying.moc"
