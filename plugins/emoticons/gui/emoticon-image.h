/*
 * %kadu copyright begin%
 * Copyright 2026 Kadu Qt6 port
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

#pragma once

#include <QtGui/QPixmap>

/**
 * @short An emoticon's picture as it should appear on this screen.
 *
 * Left at its own size -- which is what a scale of one means -- every pixel of the picture is given
 * a whole square of screen pixels, as many across as the screen puts into a logical one, and no
 * colour is invented in between. That is the rule the emoticons in the conversation itself are
 * drawn by, and these are the same pictures.
 *
 * Only a set whose emoticons are bigger than a selector has room for is scaled at all, and there
 * the scaling is downwards, which is where a smooth transformation earns its keep.
 *
 * Both the grid and the picture that appears over it while the pointer rests there go through this,
 * so that resting the pointer somewhere does not change how the emoticon under it is drawn.
 */
inline QPixmap emoticonForScreen(const QPixmap &image, qreal scale, qreal ratio)
{
    if (image.isNull())
        return image;

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
