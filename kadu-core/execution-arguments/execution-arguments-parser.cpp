#include <QtCore5Compat/QRegExp>
/*
#include <QtCore5Compat/QRegExp>
 * %kadu copyright begin%
#include <QtCore5Compat/QRegExp>
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
#include <QtCore5Compat/QRegExp>
 * %kadu copyright end%
#include <QtCore5Compat/QRegExp>
 *
#include <QtCore5Compat/QRegExp>
 * This program is free software; you can redistribute it and/or
#include <QtCore5Compat/QRegExp>
 * modify it under the terms of the GNU General Public License as
#include <QtCore5Compat/QRegExp>
 * published by the Free Software Foundation; either version 2 of
#include <QtCore5Compat/QRegExp>
 * the License, or (at your option) any later version.
#include <QtCore5Compat/QRegExp>
 *
#include <QtCore5Compat/QRegExp>
 * This program is distributed in the hope that it will be useful,
#include <QtCore5Compat/QRegExp>
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
#include <QtCore5Compat/QRegExp>
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#include <QtCore5Compat/QRegExp>
 * GNU General Public License for more details.
#include <QtCore5Compat/QRegExp>
 *
#include <QtCore5Compat/QRegExp>
 * You should have received a copy of the GNU General Public License
#include <QtCore5Compat/QRegExp>
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
#include <QtCore5Compat/QRegExp>
 */
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
#include "execution-arguments-parser.h"
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
#include "execution-arguments/execution-arguments.h"
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
#include <cstdio>
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
ExecutionArguments ExecutionArgumentsParser::parse(const QStringList &arguments) const
#include <QtCore5Compat/QRegExp>
{
#include <QtCore5Compat/QRegExp>
    auto queryVersion = false;
#include <QtCore5Compat/QRegExp>
    auto queryUsage = false;
#include <QtCore5Compat/QRegExp>
    auto debugMask = QString{};
#include <QtCore5Compat/QRegExp>
    auto profileDirectory = QString{};
#include <QtCore5Compat/QRegExp>
    auto openIds = QStringList{};
#include <QtCore5Compat/QRegExp>
    auto openUuid = QString{};
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
    for (auto it = arguments.constBegin(); it != arguments.constEnd(); ++it)
#include <QtCore5Compat/QRegExp>
    {
#include <QtCore5Compat/QRegExp>
        if (*it == QStringLiteral("--version"))
#include <QtCore5Compat/QRegExp>
            queryVersion = true;
#include <QtCore5Compat/QRegExp>
        else if (*it == QStringLiteral("--help"))
#include <QtCore5Compat/QRegExp>
            queryUsage = true;
#include <QtCore5Compat/QRegExp>
        else if (*it == QStringLiteral("--debug") && (it + 1) != arguments.constEnd())
#include <QtCore5Compat/QRegExp>
        {
#include <QtCore5Compat/QRegExp>
            bool ok;
#include <QtCore5Compat/QRegExp>
            (++it)->toInt(&ok);
#include <QtCore5Compat/QRegExp>
            if (ok)
#include <QtCore5Compat/QRegExp>
                debugMask = *it;
#include <QtCore5Compat/QRegExp>
        }
#include <QtCore5Compat/QRegExp>
        else if (*it == QStringLiteral("--config-dir") && (it + 1) != arguments.constEnd())
#include <QtCore5Compat/QRegExp>
            profileDirectory = *(++it);
#include <QtCore5Compat/QRegExp>
        else if (QRegExp("^[a-zA-Z]+:(/){0,3}.+").exactMatch(*it))
#include <QtCore5Compat/QRegExp>
            openIds.append(*it);
#include <QtCore5Compat/QRegExp>
        else if (*it == QStringLiteral("--open-uuid") && (it + 1) != arguments.constEnd())
#include <QtCore5Compat/QRegExp>
            openUuid = *(++it);
#include <QtCore5Compat/QRegExp>
        else
#include <QtCore5Compat/QRegExp>
            fprintf(stderr, "Ignoring unknown parameter '%s'\n", it->toUtf8().constData());
#include <QtCore5Compat/QRegExp>
    }
#include <QtCore5Compat/QRegExp>

#include <QtCore5Compat/QRegExp>
    return ExecutionArguments{
#include <QtCore5Compat/QRegExp>
        queryVersion, queryUsage, std::move(profileDirectory), std::move(openIds), std::move(openUuid)};
#include <QtCore5Compat/QRegExp>
}
