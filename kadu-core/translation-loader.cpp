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

#include "translation-loader.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "misc/paths-provider.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>

TranslationLoader::TranslationLoader(QObject *parent) : QObject{parent}
{
}

TranslationLoader::~TranslationLoader()
{
}

void TranslationLoader::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void TranslationLoader::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void TranslationLoader::init()
{
    auto const lang =
        m_configuration->deprecatedApi()->readEntry("General", "Language", QLocale::system().name().left(2));
    auto const path = m_pathsProvider->dataPath() + QStringLiteral("translations");

    // Installed only where there is something to install. A language Qt has no translation for is
    // not a fault -- that module simply stays in English -- but an empty translator still gets asked
    // about every string that is ever displayed, and load() now insists that its answer be read.
    auto const install = [&lang, &path](QTranslator &translator, const QString &prefix)
    {
        if (translator.load(prefix + lang, path))
            QCoreApplication::installTranslator(&translator);
    };

    install(m_qt, QStringLiteral("qt_"));
    install(m_qtbase, QStringLiteral("qtbase_"));
    install(m_qtdeclarative, QStringLiteral("qtdeclarative_"));
    install(m_qtmultimedia, QStringLiteral("qtmultimedia_"));
    install(m_kadu, QStringLiteral("kadu_"));
}

void TranslationLoader::done()
{
    QCoreApplication::removeTranslator(&m_qt);
    QCoreApplication::removeTranslator(&m_qtbase);
    QCoreApplication::removeTranslator(&m_qtdeclarative);
    QCoreApplication::removeTranslator(&m_qtmultimedia);
    QCoreApplication::removeTranslator(&m_kadu);
}
