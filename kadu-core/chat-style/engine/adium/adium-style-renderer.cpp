/*
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

#include "adium-style-renderer.h"
#include "adium-style-renderer.moc"

#include "avatars/avatar-id.h"
#include "avatars/avatars.h"
#include "chat-style/chat-style-manager.h"
#include "chat-style/engine/adium/adium-style-engine.h"
#include "chat-style/engine/adium/adium-style.h"
#include "chat-style/engine/adium/adium-time-formatter.h"
#include "contacts/contact-global-id.h"
#include "contacts/contact-set.h"
#include "core/core.h"
#include "gui/configuration/chat-configuration-holder.h"
#include "html/normalized-html-string.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "identities/identity.h"
#include "message/message-html-renderer-service.h"
#include "message/message-render-info-factory.h"
#include "message/message-render-info.h"
#include "misc/date-time.h"
#include "misc/misc.h"
#include "misc/paths-provider.h"
#include "os/generic/system-info.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "protocols/services/chat-image.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QFile>
#include <QtGui/QTextDocument>
#include <QtWebEngineCore/QWebEnginePage>


AdiumStyleRenderer::AdiumStyleRenderer(
    ChatStyleRendererConfiguration configuration, std::shared_ptr<AdiumStyle> style, QObject *parent)
        : ChatStyleRenderer{std::move(configuration), parent}, m_style{std::move(style)}
{
}

AdiumStyleRenderer::~AdiumStyleRenderer()
{
}

void AdiumStyleRenderer::setAvatars(Avatars *avatars)
{
    m_avatars = avatars;
}

void AdiumStyleRenderer::setChatConfigurationHolder(ChatConfigurationHolder *chatConfigurationHolder)
{
    m_chatConfigurationHolder = chatConfigurationHolder;
}

void AdiumStyleRenderer::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void AdiumStyleRenderer::setMessageHtmlRendererService(MessageHtmlRendererService *messageHtmlRendererService)
{
    m_messageHtmlRendererService = messageHtmlRendererService;
}

void AdiumStyleRenderer::setSystemInfo(SystemInfo *systemInfo)
{
    m_systemInfo = systemInfo;
}

void AdiumStyleRenderer::init()
{
    // Relative references inside the style are resolved by the <base> element the Adium template
    // carries, but the document still needs a local origin of its own -- otherwise QtWebEngine
    // refuses every file:// resource while reporting a successful load.
    // Connected before the load is started: setHtml() is asynchronous, and hooking the signal
    // afterwards is a race that only happens to work.
    connect(&configuration().page(), &QWebEnginePage::loadFinished, this, &AdiumStyleRenderer::pageLoaded);
    configuration().page().setHtml(
        preprocessStyleBaseHtml(configuration().useTransparency()),
        QUrl::fromLocalFile(m_style->baseHref()));
}

void AdiumStyleRenderer::pageLoaded()
{
    configuration().page().runJavaScript(configuration().javaScript());

    // runJavaScript() is asynchronous, so readiness has to be announced from the callback of the
    // last script. Declaring it right after the calls would let the view start appending messages
    // through functions the document has not defined yet.
    configuration().page().runJavaScript(QStringLiteral("initStyle()"), [this](const QVariant &) { setReady(); });
}

void AdiumStyleRenderer::removeFirstMessage()
{
    configuration().page().runJavaScript("adium_removeFirstMessage()");
}

void AdiumStyleRenderer::clearMessages()
{
    configuration().page().runJavaScript("adium_clearMessages()");
}

void AdiumStyleRenderer::appendChatMessage(const Message &message, const MessageRenderInfo &messageRenderInfo)
{
    QString formattedMessageHtml;

    switch (message.type())
    {
    case MessageTypeReceived:
    {
        if (messageRenderInfo.includeHeader())
            formattedMessageHtml = m_style->incomingHtml();
        else
            formattedMessageHtml = m_style->nextIncomingHtml();
        break;
    }
    case MessageTypeSent:
    {
        if (messageRenderInfo.includeHeader())
            formattedMessageHtml = m_style->outgoingHtml();
        else
            formattedMessageHtml = m_style->nextOutgoingHtml();
        break;
    }
    case MessageTypeSystem:
    {
        formattedMessageHtml = m_style->statusHtml();
        break;
    }

    default:
        break;
    }

    formattedMessageHtml = replacedNewLine(
        replaceKeywords(m_style->baseHref(), formattedMessageHtml, message, messageRenderInfo.nickColor()),
        QStringLiteral(" "));
    formattedMessageHtml.replace('\\', QStringLiteral("\\\\"));
    formattedMessageHtml.replace('\'', QStringLiteral("\\'"));
    if (!message.id().isEmpty())
        formattedMessageHtml.prepend(QString("<span id=\"message_%1\">").arg((message.id()).toHtmlEscaped()));
    else
        formattedMessageHtml.prepend("<span>");
    formattedMessageHtml.append("</span>");

    if (messageRenderInfo.includeHeader())
        configuration().page().runJavaScript("appendMessage('" + formattedMessageHtml + "')");
    else
        configuration().page().runJavaScript("appendNextMessage('" + formattedMessageHtml + "')");
}

QString AdiumStyleRenderer::preprocessStyleBaseHtml(bool useTransparency)
{
    QString styleBaseHtml = m_style->templateHtml();
    styleBaseHtml.replace(styleBaseHtml.indexOf("%@"), 2, (PathsProvider::webKitPath(m_style->baseHref())).toHtmlEscaped());
    styleBaseHtml.replace(
        styleBaseHtml.lastIndexOf("%@"), 2, replaceKeywords(m_style->baseHref(), m_style->footerHtml()));
    styleBaseHtml.replace(
        styleBaseHtml.lastIndexOf("%@"), 2, replaceKeywords(m_style->baseHref(), m_style->headerHtml()));

    if (m_style->usesCustomTemplateHtml() && m_style->styleViewVersion() < 3)
    {
        if (m_style->currentVariant() != m_style->defaultVariant())
            styleBaseHtml.replace(
                styleBaseHtml.lastIndexOf("%@"), 2, "Variants/" + (m_style->currentVariant()).toHtmlEscaped());
        else
            styleBaseHtml.replace(styleBaseHtml.lastIndexOf("%@"), 2, (m_style->mainHref()).toHtmlEscaped());
    }
    else
    {
        styleBaseHtml.replace(
            styleBaseHtml.lastIndexOf("%@"), 2,
            (m_style->styleViewVersion() < 3 && m_style->defaultVariant() == m_style->currentVariant())
                ? (PathsProvider::webKitPath(m_style->mainHref())).toHtmlEscaped()
                : "Variants/" + (m_style->currentVariant()).toHtmlEscaped());
        styleBaseHtml.replace(
            styleBaseHtml.lastIndexOf("%@"), 2,
            (m_style->styleViewVersion() < 3)
                ? QString()
                : QString("@import url( \"" + (PathsProvider::webKitPath(m_style->mainHref())).toHtmlEscaped() + "\" );"));
    }

    if (useTransparency && !m_style->defaultBackgroundIsTransparent())
        styleBaseHtml.replace(
            styleBaseHtml.lastIndexOf("==bodyBackground=="), static_cast<int>(qstrlen("==bodyBackground==")),
            "background-image: none; background: none; background-color: rgba(0, 0, 0, 0)");

    return styleBaseHtml;
}

// Some parts of the code below are borrowed from Kopete project (http://kopete.kde.org/)
QString AdiumStyleRenderer::replaceKeywords(const QString &styleHref, const QString &style)
{
    if (!configuration().chat())
        return {};

    QString result = style;

    // TODO: get Chat name (contacts' nicks?)
    // Replace %chatName% //TODO. Find way to dynamic update this tag (add id ?)
    int contactsCount = configuration().chat().contacts().count();

    QString chatName;
    if (!configuration().chat().display().isEmpty())
        chatName = configuration().chat().display();
    else if (contactsCount > 1)
        chatName = tr("Conference [%1]").arg(contactsCount);
    else
        chatName = configuration().chat().name();

    result.replace(QString("%chatName%"), (chatName).toHtmlEscaped());

    // Replace %sourceName%
    result.replace(QString("%sourceName%"), (configuration().chat().chatAccount().accountIdentity().name()).toHtmlEscaped());
    // Replace %destinationName%
    result.replace(QString("%destinationName%"), (configuration().chat().name()).toHtmlEscaped());
    // For %timeOpened%, display the date and time. TODO: get real time
    result.replace(
        QString("%timeOpened%"),
        (printDateTime(m_chatConfigurationHolder->niceDateFormat(), QDateTime::currentDateTime())).toHtmlEscaped());

    // TODO 0.10.0: get real time!!!
    static const QRegularExpression timeOpenedRegExp{QStringLiteral("%timeOpened\\{([^}]*)\\}%")};
    int pos = 0;
    QRegularExpressionMatch timeOpenedMatch;
    while ((timeOpenedMatch = timeOpenedRegExp.match(result, pos)).hasMatch())
    {
        pos = timeOpenedMatch.capturedStart();
        result.replace(
            pos, timeOpenedMatch.capturedLength(),
            (AdiumTimeFormatter::convertTimeDate(m_systemInfo, timeOpenedMatch.captured(1), QDateTime::currentDateTime()))
                .toHtmlEscaped());
    }

    QString photoIncoming;
    QString photoOutgoing;

    int contactsSize = configuration().chat().contacts().size();
    if (contactsSize == 1)
    {
        auto const &contact = configuration().chat().contacts().toContact();
        auto path = m_avatars->path(avatarIds(contact));
        if (!path.isEmpty())
            photoIncoming = PathsProvider::webKitPath(path);
        else
            photoIncoming = PathsProvider::webKitPath(styleHref + QStringLiteral("Incoming/buddy_icon.png"));
    }
    else
        photoIncoming = PathsProvider::webKitPath(styleHref + QStringLiteral("Incoming/buddy_icon.png"));

    auto const &contact = configuration().chat().chatAccount().accountContact();
    auto path = m_avatars->path(avatarIds(contact));
    if (!path.isEmpty())
        photoIncoming = PathsProvider::webKitPath(path);
    else
        photoOutgoing = PathsProvider::webKitPath(styleHref + QStringLiteral("Outgoing/buddy_icon.png"));

    result.replace(QString("%incomingIconPath%"), (photoIncoming).toHtmlEscaped());
    result.replace(QString("%outgoingIconPath%"), (photoOutgoing).toHtmlEscaped());

    return result;
}

QString AdiumStyleRenderer::replaceKeywords(
    const QString &styleHref, const QString &source, const Message &message, const QString &nickColor)
{
    QString result = source;

    // Replace sender (contact nick)
    result.replace(QString("%sender%"), (message.messageSender().display(true)).toHtmlEscaped());
    // Replace %screenName% (contact ID)
    result.replace(QString("%senderScreenName%"), (message.messageSender().id()).toHtmlEscaped());
    // Replace service name (protocol name)
    if (message.messageChat().chatAccount().protocolHandler() &&
        message.messageChat().chatAccount().protocolHandler()->protocolFactory())
    {
        result.replace(
            QString("%service%"),
            (message.messageChat().chatAccount().protocolHandler()->protocolFactory()->displayName()).toHtmlEscaped());
        // Replace protocolIcon (sender statusIcon). TODO:
        result.replace(
            QString("%senderStatusIcon%"),
            (m_iconsManager->iconPath(
                message.messageChat().chatAccount().protocolHandler()->protocolFactory()->icon())).toHtmlEscaped());
    }
    else
    {
        result.replace(
            QString("%service%"), ((message.messageChat().chatAccount().accountIdentity().name()).toHtmlEscaped()).toHtmlEscaped());
        result.remove("%senderStatusIcon%");
    }

    // Replace time
    QDateTime time = message.sendDate().isNull() ? message.receiveDate() : message.sendDate();
    result.replace(QString("%time%"), (printDateTime(m_chatConfigurationHolder->niceDateFormat(), time)).toHtmlEscaped());
    // Look for %time{X}%
    static const QRegularExpression timeRegExp{QStringLiteral("%time\\{([^}]*)\\}%")};
    int pos = 0;
    QRegularExpressionMatch timeMatch;
    while ((timeMatch = timeRegExp.match(result, pos)).hasMatch())
    {
        pos = timeMatch.capturedStart();
        result.replace(
            pos, timeMatch.capturedLength(),
            (AdiumTimeFormatter::convertTimeDate(m_systemInfo, timeMatch.captured(1), time)).toHtmlEscaped());
    }

    result.replace("%shortTime%", (printDateTime(m_chatConfigurationHolder->niceDateFormat(), time)).toHtmlEscaped());

    // Look for %textbackgroundcolor{X}%
    // TODO: highlight background color: use the X value.
    static const QRegularExpression textBackgroundRegExp{QStringLiteral("%textbackgroundcolor\\{([^}]*)\\}%")};
    int textPos = 0;
    QRegularExpressionMatch textBackgroundMatch;
    while ((textBackgroundMatch = textBackgroundRegExp.match(result, textPos)).hasMatch())
    {
        textPos = textBackgroundMatch.capturedStart();
        result.replace(textPos, textBackgroundMatch.capturedLength(), "inherit");
    }

    // Replace userIconPath
    QString photoPath;
    if (message.type() == MessageTypeReceived)
    {
        result.replace(QString("%messageClasses%"), "message incoming");

        auto const &contact = message.messageSender();
        auto path = m_avatars->path(avatarIds(contact));
        if (!path.isEmpty())
            photoPath = PathsProvider::webKitPath(path);
        else
            photoPath = (PathsProvider::webKitPath(styleHref + QStringLiteral("Incoming/buddy_icon.png"))).toHtmlEscaped();
    }
    else if (message.type() == MessageTypeSent)
    {
        result.replace(QString("%messageClasses%"), "message outgoing");
        auto const &contact = message.messageChat().chatAccount().accountContact();
        auto path = m_avatars->path(avatarIds(contact));
        if (!path.isEmpty())
            photoPath = PathsProvider::webKitPath(path);
        else
            photoPath = (PathsProvider::webKitPath(styleHref + QStringLiteral("Outgoing/buddy_icon.png"))).toHtmlEscaped();
    }
    else
        result.remove(QString("%messageClasses%"));

    result.replace(QString("%userIconPath%"), (photoPath).toHtmlEscaped());

    // Message direction ("rtl"(Right-To-Left) or "ltr"(Left-to-right))
    result.replace(QString("%messageDirection%"), "ltr");

    // Replace contact's color
    QString lightColorName;
    static const QRegularExpression senderColorRegExp{QStringLiteral("%senderColor(?:\\{([^}]*)\\})?%")};
    textPos = 0;
    QRegularExpressionMatch senderColorMatch;
    while ((senderColorMatch = senderColorRegExp.match(result, textPos)).hasMatch())
    {
        textPos = senderColorMatch.capturedStart();

        int light = 100;
        bool doLight = false;
        // The group is optional, so it may not have participated; captured(1) is then null and
        // toInt() reports failure, which is what selects the unlightened colour.
        light = senderColorMatch.captured(1).toInt(&doLight);

        if (doLight && lightColorName.isNull())
            lightColorName = QColor(nickColor).lighter(light).name();

        result.replace(
            textPos, senderColorMatch.capturedLength(), (doLight ? lightColorName : nickColor).toHtmlEscaped());
    }

    auto messageText = m_messageHtmlRendererService ? m_messageHtmlRendererService.data()->renderMessage(message)
                                                    : message.content().string();

    if (!message.id().isEmpty())
        messageText.prepend(QString("<span id=\"message_%1\">").arg((message.id()).toHtmlEscaped()));
    else
        messageText.prepend("<span>");
    messageText.append("</span>");

    result.replace(QString("%messageId%"), (message.id()).toHtmlEscaped());
    result.replace(QString("%messageStatus%"), (QString::number(message.status())).toHtmlEscaped());

    result.replace(QString("%message%"), messageText);

    return result;
}

void AdiumStyleRenderer::displayMessageStatus(const QString &id, MessageStatus status)
{
    configuration().page().runJavaScript(
        QString("adium_messageStatusChanged(\"%1\", %2);").arg((id).toHtmlEscaped()).arg(static_cast<int>(status)));
}

void AdiumStyleRenderer::displayChatState(ChatState state, const QString &message, const QString &name)
{
    configuration().page().runJavaScript(QString("adium_contactActivityChanged(%1, \"%2\", \"%3\");")
                                                      .arg(static_cast<int>(state))
                                                      .arg((message).toHtmlEscaped())
                                                      .arg((name).toHtmlEscaped()));
}

void AdiumStyleRenderer::displayChatImage(const ChatImage &chatImage, const QString &fileName)
{
    configuration().page().runJavaScript(QString("adium_chatImageAvailable(\"%1\", \"%2\");")
                                                      .arg((chatImage.key()).toHtmlEscaped())
                                                      .arg((fileName).toHtmlEscaped()));
}
