/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Sławomir Stępień (s.stepien@interia.pl)
 * Copyright 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QSettings>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "plugin/metadata/plugin-metadata.h"
#include "plugin/state/plugin-state-service.h"
#include "plugin/state/plugin-state.h"
#include "windows/main-configuration-window.h"

#include "plugins/mediaplayer/mediaplayer-plugin-object.h"
#include "plugins/mediaplayer/mediaplayer.h"

#include "mpris-player.h"
#include "mpris-player.moc"

const QString MPRISPlayer::UserPlayersListFile = "mprisplayer-players.data";
const QString MPRISPlayer::GlobalPlayersListFile =
    "plugins/data/mprisplayer_mediaplayer/" + MPRISPlayer::UserPlayersListFile;

MPRISPlayer::MPRISPlayer(QObject *parent) : MPRISMediaPlayer(parent)
{
}

MPRISPlayer::~MPRISPlayer()
{
}

void MPRISPlayer::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void MPRISPlayer::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void MPRISPlayer::setPluginStateService(PluginStateService *pluginStateService)
{
    m_pluginStateService = pluginStateService;
}

void MPRISPlayer::init()
{
    // The moving on goes last because replacePlugin() is one of the things that writes an address
    // down, and what it writes when it takes over from the old mpris_mediaplayer module is
    // whatever that module was told to use -- a version 1 address, since that is all there was.
    // Moving first left such a user speaking version 1 for the whole of the session, put right
    // only at the next start.
    prepareUserPlayersFile();
    replacePlugin();
    moveChosenPlayerToMpris2();
}

namespace
{
/**
 * @short The version 2 address of a player named by a version 1 one.
 *
 * Whoever chose a player before this was fixed has an address from version 1 of MPRIS written
 * down -- org.mpris.audacious, or one of the two that were never MPRIS at all -- and Kadu has been
 * speaking version 2 to it, which nothing answers. The names that can be known for certain are
 * moved on; anything else is handed back as it came, since it may well be something the user typed
 * in themselves.
 */
QString movedToMpris2(const QString &service)
{
    static const QMap<QString, QString> moved = {
        {QStringLiteral("org.kde.amarok"), QStringLiteral("org.mpris.MediaPlayer2.amarok")},
        {QStringLiteral("org.mpris.amarok"), QStringLiteral("org.mpris.MediaPlayer2.amarok")},
        {QStringLiteral("org.mpris.audacious"), QStringLiteral("org.mpris.MediaPlayer2.audacious")},
        {QStringLiteral("org.mpris.clementine"), QStringLiteral("org.mpris.MediaPlayer2.clementine")},
        {QStringLiteral("org.kde.dragon.player"), QStringLiteral("org.mpris.MediaPlayer2.dragonplayer")},
        {QStringLiteral("org.mpris.vlc"), QStringLiteral("org.mpris.MediaPlayer2.vlc")},
        {QStringLiteral("org.mpris.xmms2"), QStringLiteral("org.mpris.MediaPlayer2.xmms2")}};

    return moved.value(service, service);
}
}

void MPRISPlayer::moveChosenPlayerToMpris2()
{
    auto const chosen = m_configuration->deprecatedApi()->readEntry("MPRISPlayer", "Service");
    auto const moved = movedToMpris2(chosen);

    if (moved == chosen)
        return;

    m_configuration->deprecatedApi()->writeEntry("MPRISPlayer", "Service", moved);
}

void MPRISPlayer::prepareUserPlayersFile()
{
    if (QFile::exists(MPRISPlayer::userPlayersListFileName(m_pathsProvider)))
        return;

    QFile userFile(MPRISPlayer::userPlayersListFileName(m_pathsProvider));
    if (!userFile.open(QIODevice::ReadWrite))
        return;

    userFile.close();
}

void MPRISPlayer::replacePlugin()
{
    QMap<QString, QString> replaceMap;
    replaceMap.insert("amarok2_mediaplayer", "Amarok");
    replaceMap.insert("audacious_mediaplayer", "Audacious");
    // BMPx has no version 2 address to be given and is no longer in the list shipped with Kadu,
    // the project having ended before version 2 existed. It stays here so that a profile still
    // carrying the old module gets it turned off; choosePlayer() will find nothing to choose and
    // the search goes on to whatever else is enabled.
    replaceMap.insert("bmpx_mediaplayer", "BMPx");
    replaceMap.insert("dragon_mediaplayer", "Dragon Player");
    replaceMap.insert("mpris_mediaplayer", "MPRIS Media Player");
    replaceMap.insert("vlc_mediaplayer", "VLC");
    replaceMap.insert("xmms2_mediaplayer", "XMMS2");

    for (auto const &value : replaceMap)
    {
        QString key = replaceMap.key(value);
        if (m_pluginStateService->pluginState(key) != PluginState::Enabled)
            continue;

        // Turned off whether or not it can be turned into a choice: the module is gone either way.
        m_pluginStateService->setPluginState(key, PluginState::Disabled);

        if (choosePlayer(key, value))
            break;
    }
}

bool MPRISPlayer::choosePlayer(const QString &key, const QString &value)
{
    // Save service value from mpris_mediaplayer module
    if (key == "mpris_mediaplayer")
    {
        // Moved on here as well as in the chosen address, since this is the entry the player is
        // listed under from now on and choosing it again later would otherwise bring version 1
        // back.
        QString oldMPRISService =
            movedToMpris2(m_configuration->deprecatedApi()->readEntry("MediaPlayer", "MPRISService"));
        if (oldMPRISService.isEmpty())
            return false;

        QSettings userPlayersSettings(MPRISPlayer::userPlayersListFileName(m_pathsProvider), QSettings::IniFormat);

        userPlayersSettings.setValue(value + "/player", value);
        userPlayersSettings.setValue(value + "/service", oldMPRISService);
        userPlayersSettings.sync();

        m_configuration->deprecatedApi()->writeEntry("MPRISPlayer", "Player", value);
        m_configuration->deprecatedApi()->writeEntry("MPRISPlayer", "Service", oldMPRISService);

        return true;
    }

    // Choose player based on old module loaded.
    QSettings globalPlayersSettings(MPRISPlayer::globalPlayersListFileName(m_pathsProvider), QSettings::IniFormat);
    auto const service = globalPlayersSettings.value(value + "/service").toString();

    // Nothing is written for a player the shipped list does not name. It used to be written all the
    // same, which left the user with a player chosen and no address to reach it at -- worse than
    // leaving the choice as it was, since it also replaced whatever was there before.
    if (service.isEmpty())
        return false;

    m_configuration->deprecatedApi()->writeEntry("MPRISPlayer", "Player", value);
    m_configuration->deprecatedApi()->writeEntry("MPRISPlayer", "Service", service);

    return true;
}

void MPRISPlayer::configurationApplied()
{
    auto name = m_configuration->deprecatedApi()->readEntry("MPRISPlayer", "Player");
    setName(name);
    setService(m_configuration->deprecatedApi()->readEntry("MPRISPlayer", "Service"));

    if (name == "Audacious")
        m_mediaPlayer->setInterval(5);
    else
        m_mediaPlayer->setInterval(0);
}
