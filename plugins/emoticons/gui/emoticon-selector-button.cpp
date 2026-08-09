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

#include <QtGui/QMouseEvent>
#include <QtGui/QMovie>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "expander/emoticon-path-provider.h"
#include "gui/emoticon-image.h"
#include "gui/emoticon-selector-button-popup.h"

#include "emoticon-selector-button.h"
#include "emoticon-selector-button.moc"

EmoticonSelectorButton::EmoticonSelectorButton(
    const Emoticon &emoticon, const QPixmap &image, qreal scale, bool animate, EmoticonPathProvider *pathProvider,
    QWidget *parent)
        : QLabel(parent), DisplayEmoticon(emoticon), PathProvider(pathProvider), Scale(scale), Movie(nullptr)
{
    // The still picture first, and the size from it, so that the list can be laid out without
    // waiting on any film to start. Whatever moves afterwards is the same picture and the same
    // size, so nothing shifts once it does.
    setPixmap(emoticonForScreen(image, Scale, devicePixelRatio()));
    // Placed by the same rule the window that opens over it uses. A label left to itself puts its
    // picture against the left edge, that window centres its own, and the two agree only for as
    // long as the room left over is exactly nothing. A pixel of slack anywhere -- and where that
    // falls depends on the screen -- and the emoticon steps sideways as the pointer arrives.
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setFixedSize(sizeHint());

    if (!animate)
        return;

    Movie = new QMovie(this);
    Movie->setFileName(PathProvider->emoticonPath(DisplayEmoticon));
    connect(Movie, &QMovie::frameChanged, this, &EmoticonSelectorButton::showFrame);
    Movie->start();
}

void EmoticonSelectorButton::showFrame()
{
    setPixmap(emoticonForScreen(Movie->currentPixmap(), Scale, devicePixelRatio()));
}

EmoticonSelectorButton::~EmoticonSelectorButton()
{
}

void EmoticonSelectorButton::mouseMoveEvent(QMouseEvent *e)
{
    QLabel::mouseMoveEvent(e);

    EmoticonSelectorButtonPopup *popup =
        new EmoticonSelectorButtonPopup(DisplayEmoticon, Scale, PathProvider, this);
    connect(popup, SIGNAL(clicked(Emoticon)), this, SIGNAL(clicked(Emoticon)));
    popup->show();
}
