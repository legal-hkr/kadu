/*
 * Copyright 2007, 2008 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2004, 2005, 2006, 2007 Marcin Ślusarz (joi@kadu.net)
 * Copyright 2002, 2003, 2004, 2006 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2003, 2004 Tomasz Chiliński (chilek@chilan.com)
 * Copyright 2007, 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2004, 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2008, 2009 Piotr Galiszewski (piotrgaliszewski@gmail.com)
 * Copyright 2003, 2005 Paweł Płuciennik (pawel_p@kadu.net)
 * Copyright 2003, 2004 Dariusz Jagodzik (mast3r@kadu.net)
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtGui/QEnterEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QMovie>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "expander/emoticon-path-provider.h"
#include "gui/emoticon-image.h"

#include "emoticon-selector-button.h"
#include "emoticon-selector-button.moc"

EmoticonSelectorButton::EmoticonSelectorButton(
    const Emoticon &emoticon, const QPixmap &image, qreal scale, bool animate, EmoticonPathProvider *pathProvider,
    QWidget *parent)
        : QLabel(parent), DisplayEmoticon(emoticon), PathProvider(pathProvider), Scale(scale), Movie(nullptr),
          MovingOnlyWhilePointed(false)
{
    // The still picture first, and the size from it, so that the list can be laid out without
    // waiting on any film to start. Whatever moves afterwards is the same picture at the same size,
    // so nothing shifts once it does.
    StillPicture = emoticonForScreen(image, Scale, devicePixelRatio());
    setPixmap(StillPicture);
    setAlignment(Qt::AlignCenter);
    setContentsMargins(4, 4, 4, 4);
    setFixedSize(sizeHint());
    setToolTip(DisplayEmoticon.triggerText());

    if (animate)
        startMoving(false);
}

EmoticonSelectorButton::~EmoticonSelectorButton()
{
}

void EmoticonSelectorButton::startMoving(bool onlyWhilePointed)
{
    if (Movie)
        return;

    MovingOnlyWhilePointed = onlyWhilePointed;

    Movie = new QMovie(this);
    Movie->setFileName(PathProvider->emoticonPath(DisplayEmoticon));
    connect(Movie, &QMovie::frameChanged, this, &EmoticonSelectorButton::showFrame);
    Movie->start();
}

void EmoticonSelectorButton::showFrame()
{
    setPixmap(emoticonForScreen(Movie->currentPixmap(), Scale, devicePixelRatio()));
}

void EmoticonSelectorButton::enterEvent(QEnterEvent *event)
{
    QLabel::enterEvent(event);

    // The cell wears the highlight itself. This used to be a second window laid over it, which is
    // where two complaints came from: it had a film of its own, so an emoticon already moving
    // started again from the beginning the moment the pointer arrived, and it carried a style sheet
    // where the cell carries margins -- two ways of asking for the same four pixels that do not
    // always answer alike, which walked the picture a pixel sideways.
    //
    // Asked for by role rather than by colour, so it still follows whatever the desktop is wearing.
    setBackgroundRole(QPalette::Highlight);
    setAutoFillBackground(true);

    // Already moving if the whole list is; then this does nothing and nothing restarts.
    startMoving(true);
}

void EmoticonSelectorButton::leaveEvent(QEvent *event)
{
    QLabel::leaveEvent(event);

    setAutoFillBackground(false);

    if (!Movie || !MovingOnlyWhilePointed)
        return;

    delete Movie;
    Movie = nullptr;
    setPixmap(StillPicture);
}

void EmoticonSelectorButton::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel::mouseReleaseEvent(event);

    emit clicked(DisplayEmoticon);
}
