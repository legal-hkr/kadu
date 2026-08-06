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

#include "kadu-icon-engine.h"

#include <QtGui/QImageReader>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

KaduIconEngine::KaduIconEngine()
{
}

KaduIconEngine::~KaduIconEngine()
{
}

void KaduIconEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    if (fileName.isEmpty())
        return;

    auto fileSize = size;
    if (!fileSize.isValid() || fileSize.isEmpty())
    {
        // Reading the header alone is enough to learn the size, and avoids decoding every file of
        // every icon just to build the list.
        QImageReader reader{fileName};
        fileSize = reader.size();
    }

    if (!fileSize.isValid() || fileSize.isEmpty())
        return;

    Sources.append(Source{fileName, fileSize});
}

const KaduIconEngine::Source *KaduIconEngine::sourceFor(const QSize &sizeInPixels) const
{
    const Source *smallestBigEnough = nullptr;
    const Source *largest = nullptr;

    for (auto const &source : Sources)
    {
        if (!largest || source.Size.width() > largest->Size.width())
            largest = &source;

        auto const bigEnough =
            source.Size.width() >= sizeInPixels.width() && source.Size.height() >= sizeInPixels.height();
        if (!bigEnough)
            continue;

        if (!smallestBigEnough || source.Size.width() < smallestBigEnough->Size.width())
            smallestBigEnough = &source;
    }

    // Nothing large enough means the theme simply does not have it, and enlarging the biggest file
    // is the best that can be done.
    return smallestBigEnough ? smallestBigEnough : largest;
}

QPixmap KaduIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    if (size.isEmpty() || scale <= 0.0)
        return QPixmap{};

    auto const sizeInPixels = QSize{qRound(size.width() * scale), qRound(size.height() * scale)};

    auto const *source = sourceFor(sizeInPixels);
    if (!source)
        return QPixmap{};

    QPixmap pixmap{source->FileName};
    if (pixmap.isNull())
        return pixmap;

    if (pixmap.size() != sizeInPixels)
        pixmap = pixmap.scaled(sizeInPixels, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    pixmap.setDevicePixelRatio(scale);
    return pixmap;
}

QPixmap KaduIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

void KaduIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    auto const scale = painter->device() ? painter->device()->devicePixelRatio() : 1.0;
    auto const pixmap = scaledPixmap(rect.size(), mode, state, scale);
    if (pixmap.isNull())
        return;

    // Centred, since a picture whose proportions differ from the rectangle is fitted inside it
    // rather than distorted to fill it.
    auto const drawnSize = pixmap.deviceIndependentSize().toSize();
    painter->drawPixmap(rect.x() + (rect.width() - drawnSize.width()) / 2,
                        rect.y() + (rect.height() - drawnSize.height()) / 2, pixmap);
}

QSize KaduIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    auto const *source = sourceFor(size);
    if (!source)
        return QSize{};

    // Anything up to what the theme can supply is given at exactly the size asked for.
    return source->Size.width() >= size.width() ? size : source->Size;
}

QList<QSize> KaduIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    QList<QSize> sizes;
    sizes.reserve(Sources.size());
    for (auto const &source : Sources)
        sizes.append(source.Size);

    return sizes;
}

QIconEngine *KaduIconEngine::clone() const
{
    auto *copy = new KaduIconEngine{};
    copy->Sources = Sources;
    return copy;
}

QString KaduIconEngine::key() const
{
    return QStringLiteral("KaduIconEngine");
}

bool KaduIconEngine::isNull()
{
    return Sources.isEmpty();
}
