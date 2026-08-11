/*
 * Copyright 2007, 2008, 2009 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2004, 2005, 2006, 2007 Marcin Ślusarz (joi@kadu.net)
 * Copyright 2002, 2003, 2004, 2005, 2007 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2002, 2003 Tomasz Chiliński (chilek@chilan.com)
 * Copyright 2007, 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2004, 2007, 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2008, 2009 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2008, 2009, 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
 * Copyright 2004, 2005 Paweł Płuciennik (pawel_p@kadu.net)
 * Copyright 2002, 2003 Dariusz Jagodzik (mast3r@kadu.net)
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "core/application.h"
#include "core/application.moc"

#include "configuration/configuration-writer.h"

#include <QtWidgets/QApplication>

Application::Application(QObject *parent) : QObject{parent}
{
#ifdef Q_OS_WIN
    // Fix for #2491
    qApp->setStyleSheet("QToolBar{border:0px}");
#endif
}

Application::~Application()
{
}

void Application::setConfigurationWriter(ConfigurationWriter *configurationWriter)
{
    m_configurationWriter = configurationWriter;
}

void Application::flushConfiguration()
{
    m_configurationWriter->write();
}

void Application::backupConfiguration()
{
    m_configurationWriter->backup();
}

bool Application::isSavingSession() const
{
    return qApp->isSavingSession();
}

bool Application::isQuitting() const
{
    return m_quitting;
}

void Application::quit()
{
    // Since Qt 6.5 quit() closes every window first and gives up if any of them refuses. The
    // contact list refuses whenever it can hide into the tray instead, so with a conversation open
    // as well the program went on running with its contact list merely hidden -- the user having
    // asked it, twice over, to finish. Measured on a bare Qt program: one stubborn window and
    // quit() still ends it; add a second window and it does not.
    //
    // Saying so first is what lets the windows agree to close.
    m_quitting = true;
    qApp->quit();

    // And taking it back afterwards is what keeps the word from standing for the rest of the
    // session. A window is still free to refuse -- an open conversation asks about a message that
    // has just come in, and Cancel means no -- and the program then goes on running. The next
    // close of the contact list after that is an ordinary one, to be answered by hiding into the
    // tray, which it would not be if this were left true. Nothing reads it once the windows have
    // been round, so clearing it costs a successful quit nothing.
    m_quitting = false;
}
