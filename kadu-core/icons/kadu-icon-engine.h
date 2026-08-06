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
 * @short Icon engine that scales down from a larger file rather than up from a smaller one.
 *
 * QIcon's own engine never returns a pixmap larger than the one asked for, so a request that falls
 * between the sizes a theme ships is answered with the smaller file and the caller enlarges it.
 * Asking for sixteen units on a screen magnified twice needs thirty-two pixels and gets them
 * exactly; asking for twenty-two needs forty-four and gets a thirty-two pixel image stretched by
 * more than a third. That is why the status button and the tray looked coarse while the same icons
 * in a menu did not.
 *
 * This engine picks the smallest file that has at least the pixels asked for, and only falls back
 * to the largest one when nothing is big enough. A size the theme provides exactly is still used
 * as it is, so hand-drawn small icons keep their detail.
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
