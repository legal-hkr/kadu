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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "gui/emoticon-selector-button-popup.h"

#include "emoticon-selector-button.h"
#include "emoticon-selector-button.moc"

namespace
{
/**
 * @short The picture as it should appear on this screen.
 *
 * Left at its own size -- which is what a scale of one means -- every pixel of the picture is given
 * a whole square of screen pixels, as many across as the screen puts into a logical one, and no
 * colour is invented in between. That is the rule the emoticons in the conversation itself are
 * drawn by, and these are the same pictures.
 *
 * Every one of them used to be brought to eighteen logical units high whatever it started as. The
 * theme Kadu ships is twenty, so the pictures were being shrunk by nine tenths and then, on a
 * magnified screen, stretched back out again -- twice through an interpolation that has nothing to
 * work with at that size, which is why they came out muddy.
 *
 * Only a set whose emoticons are bigger than a selector has room for is scaled at all, and there
 * the scaling is downwards, which is where a smooth transformation earns its keep.
 */
QPixmap forScreen(const QPixmap &image, qreal scale, qreal ratio)
{
    if (qFuzzyCompare(scale, qreal(1)))
    {
        auto const multiple = qMax(1, qRound(ratio));
        auto scaled = image.scaled(image.size() * multiple, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        scaled.setDevicePixelRatio(multiple);
        return scaled;
    }

    auto scaled =
        image.scaled((QSizeF{image.size()} * scale * ratio).toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(ratio);
    return scaled;
}
}

EmoticonSelectorButton::EmoticonSelectorButton(
    const Emoticon &emoticon, const QPixmap &image, qreal scale, EmoticonPathProvider *pathProvider, QWidget *parent)
        : QLabel(parent), DisplayEmoticon(emoticon), PathProvider(pathProvider)
{
    setPixmap(forScreen(image, scale, devicePixelRatio()));
    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setFixedSize(sizeHint());
}

EmoticonSelectorButton::~EmoticonSelectorButton()
{
}

void EmoticonSelectorButton::mouseMoveEvent(QMouseEvent *e)
{
    QLabel::mouseMoveEvent(e);

    EmoticonSelectorButtonPopup *popup = new EmoticonSelectorButtonPopup(DisplayEmoticon, PathProvider, this);
    connect(popup, SIGNAL(clicked(Emoticon)), this, SIGNAL(clicked(Emoticon)));
    popup->show();
}
