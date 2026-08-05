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

#include <QtCore/QFile>
#include <QtCore/QMimeDatabase>
#include <QtCore/QUrl>
#include <QtWebEngineCore/QWebEngineUrlRequestJob>
#include <QtWebEngineCore/QWebEngineUrlScheme>

#include "services/image-storage-service.h"

#include "kadu-image-scheme-handler.h"
#include "kadu-image-scheme-handler.moc"

QByteArray KaduImageSchemeHandler::schemeName()
{
    return QByteArrayLiteral("kaduimg");
}

void KaduImageSchemeHandler::registerScheme()
{
    QWebEngineUrlScheme scheme{schemeName()};

    // Messages address images as kaduimg:///<id> -- no host, just a path.
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);

    // The images live on disk and are displayed by a document loaded from a file:// base, so the
    // scheme has to count as local for that document to be allowed to reference it. The content
    // security policy is ignored because the styles are local files Kadu ships, not remote pages.
    scheme.setFlags(
        QWebEngineUrlScheme::LocalScheme | QWebEngineUrlScheme::LocalAccessAllowed |
        QWebEngineUrlScheme::ContentSecurityPolicyIgnored);

    QWebEngineUrlScheme::registerScheme(scheme);
}

KaduImageSchemeHandler::KaduImageSchemeHandler(ImageStorageService *imageStorageService, QObject *parent)
        : QWebEngineUrlSchemeHandler{parent}, m_imageStorageService{imageStorageService}
{
}

KaduImageSchemeHandler::~KaduImageSchemeHandler()
{
}

void KaduImageSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    if (!m_imageStorageService)
    {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    auto const filePath = m_imageStorageService->toFileUrl(job->requestUrl()).toLocalFile();
    if (filePath.isEmpty())
    {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    // Parented to the job so QtWebEngine disposes of it once the reply has been consumed.
    auto *file = new QFile{filePath, job};
    if (!file->open(QIODevice::ReadOnly))
    {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    auto const mimeType = QMimeDatabase{}.mimeTypeForFile(filePath).name().toUtf8();
    job->reply(mimeType, file);
}
