/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QBuffer>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#if defined(Q_OS_UNIX)
#endif

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

#include "gui/widgets/crop-image-widget.h"
#include "gui/widgets/screenshot-tool-box.h"

#include "screenshot-widget.h"
#include "screenshot-widget.moc"

ScreenshotWidget::ScreenshotWidget(QWidget *parent)
        : QWidget(
              parent,
              Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
          ShotMode(ShotModeStandard)
{
    setWindowRole("kadu-screenshot");

    setFocusPolicy(Qt::StrongFocus);
    // Staying above other windows and keeping out of the taskbar used to be set by writing window
    // manager properties directly. Qt::WindowStaysOnTopHint above says the same thing in the only
    // way a Wayland client can, and it is the compositor's to honour or refuse.
    setAttribute(Qt::WA_TranslucentBackground, true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    CropWidget = new CropImageWidget(this);
    connect(CropWidget, SIGNAL(pixmapCropped(QPixmap)), this, SLOT(pixmapCapturedSlot(QPixmap)));
    connect(CropWidget, SIGNAL(canceled()), this, SLOT(canceledSlot()));
    layout->addWidget(CropWidget);
}

ScreenshotWidget::~ScreenshotWidget()
{
}

void ScreenshotWidget::setShotMode(ScreenShotMode shotMode)
{
    ShotMode = shotMode;
}

void ScreenshotWidget::setPixmap(QPixmap pixmap)
{
    CropWidget->setPixmap(pixmap);

    resize(pixmap.size());
}

void ScreenshotWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        CropWidget->crop();
}

void ScreenshotWidget::pixmapCapturedSlot(QPixmap pixmap)
{
    hide();

    emit pixmapCaptured(pixmap);
    deleteLater();
}

void ScreenshotWidget::canceledSlot()
{
    hide();

    emit canceled();
    deleteLater();
}
