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

#include "exports.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QIconEngine>

/**
 * @short Icon engine that reads the file it needs for every size it is asked for.
 *
 * QIcon's own engine keeps the files it has already loaded and prefers to answer from them. A
 * request it cannot satisfy from what is loaded, and that is larger than what is loaded, is met by
 * declaring a smaller device pixel ratio on the image it has -- so the image still covers the right
 * area, made of too few pixels, and the caller enlarges it.
 *
 * That only shows when one icon serves several sizes, which is exactly what IconsManager's cache
 * arranges: one icon per path, handed to everyone. A menu asks for sixteen units first and loads
 * the thirty-two pixel file; the status button and the tray then ask for twenty-two, need
 * forty-four pixels, and are given those thirty-two stretched by three eighths. Asked in the other
 * order, both are sharp.
 *
 * This engine holds file names rather than loaded images and picks, for each request, the smallest
 * file with at least the pixels needed, scaling it down. Nothing is carried over from an earlier
 * request, so the order in which sizes are asked for stops mattering. A size the theme provides
 * exactly is used as it is, so hand-drawn small icons keep their detail.
 */
class KADUAPI KaduIconEngine : public QIconEngine
{
public:
    KaduIconEngine();
    virtual ~KaduIconEngine();

    virtual void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    virtual QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override;
    virtual QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    virtual QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) override;
    virtual QIconEngine *clone() const override;
    virtual QString key() const override;
    virtual bool isNull() override;

private:
    struct Source
    {
        QString FileName;
        QSize Size;
    };

    QList<Source> Sources;

    const Source *sourceFor(const QSize &sizeInPixels) const;
};
