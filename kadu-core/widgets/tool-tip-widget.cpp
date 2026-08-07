/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "tool-tip-widget.h"
#include "tool-tip-widget.moc"

#include "parser/parser.h"

#include <QtCore/QBuffer>
#include <QtCore/QRegularExpression>
#include <QtGui/QGuiApplication>
#include <QtGui/QPixmap>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

namespace
{
/**
 * @short Replaces the contact's picture with the same picture at the screen's resolution.
 *
 * Qt's rich text engine reads an image file and takes its pixel count for that many logical units,
 * so on a magnified screen a picture is enlarged to fill the box it is given rather than drawn
 * into it. An image carried in the markup itself is not read from disk, so it can be prepared at
 * the pixel count the screen will actually draw while the box stays the size it was.
 */
QString withPictureAtScreenResolution(QString text, int boxSize)
{
    static const QRegularExpression pictureTag{
        QStringLiteral("<img style=\"max-width:%1px; max-height:%1px;\" src=\"([^\"]*)\">").arg(boxSize)};

    auto const match = pictureTag.match(text);
    if (!match.hasMatch())
        return text;

    auto path = match.captured(1);
    path.remove(QStringLiteral("file://"));

    QPixmap picture{path};
    if (picture.isNull())
        return text;

    auto const ratio = qApp->devicePixelRatio();
    auto displaySize = picture.size();
    displaySize.scale(QSize{boxSize, boxSize}, Qt::KeepAspectRatio);

    picture = picture.scaled(
        QSize{qRound(displaySize.width() * ratio), qRound(displaySize.height() * ratio)}, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    QByteArray png;
    QBuffer buffer{&png};
    buffer.open(QIODevice::WriteOnly);
    if (!picture.save(&buffer, "PNG"))
        return text;

    return text.replace(match.capturedStart(), match.capturedLength(),
                        QStringLiteral("<img width=\"%1\" height=\"%2\" src=\"data:image/png;base64,%3\">")
                            .arg(displaySize.width())
                            .arg(displaySize.height())
                            .arg(QString::fromLatin1(png.toBase64())));
}
}

ToolTipWidget::ToolTipWidget(const Talkable &talkable, QWidget *parent)
        // A tool window is a window in its own right, and a Wayland client may not say where its
        // windows go -- measured on a live compositor, a tool window asked to appear at one corner
        // was placed in the middle of the screen instead. A tooltip is a popup anchored to the
        // widget it belongs to, which is placed where it is asked for, so that is what this is.
        : QFrame{parent, Qt::FramelessWindowHint | Qt::ToolTip},
          m_talkable{talkable}
{
    setObjectName(QStringLiteral("tool_tip"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet(
        QStringLiteral("#tool_tip { border: 1px solid %1; }").arg(palette().window().color().darker().name()));

    auto layout = make_owned<QHBoxLayout>(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_tipLabel = make_owned<QLabel>(this);
    m_tipLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_tipLabel->setContentsMargins(10, 10, 10, 10);
    m_tipLabel->setTextFormat(Qt::RichText);
    m_tipLabel->setWordWrap(true);

    layout->addWidget(m_tipLabel);
}

ToolTipWidget::~ToolTipWidget()
{
}

void ToolTipWidget::setParser(Parser *parser)
{
    m_parser = parser;
}

void ToolTipWidget::init()
{
    auto syntax = QStringLiteral(QT_TRANSLATE_NOOP(
        "HintManager",
        "<table>"
        "<tr>"
        "<td align=\"left\" valign=\"top\">"
        "<img style=\"max-width:64px; max-height:64px;\" "
        "src=\"{#{avatarPath} #{avatarPath}}{~#{avatarPath} @{kadu_icons/kadu:64x64}}\""
        ">"
        "</td>"
        "<td width=\"100%\">"
        "<div>[<b>%a</b>][&nbsp;<b>(%g)</b>]</div>"
        "[<div><img height=\"16\" width=\"16\" src=\"#{statusIconPath}\">&nbsp;&nbsp;%u</div>]"
        "[<div><img height=\"16\" width=\"16\" src=\"@{phone:16x16}\">&nbsp;&nbsp;%m</div>]"
        "[<div><img height=\"16\" width=\"16\" src=\"@{mail-message-new:16x16}\">&nbsp;&nbsp;%e</div>]"
        "</td>"
        "</tr>"
        "</table>"
        "[<hr><b>%s</b>][<b>:</b><br><small>%d</small>]"));

    auto text = m_parser->parse(syntax, m_talkable, ParserEscape::HtmlEscape);
    while (text.endsWith(QStringLiteral("<br/>")))
        text.resize(text.length() - QString("<br/>").length());
    while (text.startsWith(QStringLiteral("<br/>")))
        text = text.right(text.length() - QString("<br/>").length());

#ifdef Q_OS_UNIX
    text = text.remove("file://");
#endif

    m_tipLabel->setText(withPictureAtScreenResolution(text, 64));

    setFixedSize(m_tipLabel->sizeHint() + QSize{2, 2});
}
