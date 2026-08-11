/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QDateTime>
#include <algorithm>

#include <QtCore/QLocale>
#include <QtWidgets/QApplication>

#ifdef Q_OS_WIN
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

#include "date-time.h"

namespace
{
/**
 * @short A translated prefix turned into a format that says what it means.
 *
 * These prefixes are translated whole -- "dddd at " becomes "dddd o " in Polish -- so a translator
 * writes the date tokens and the words around them into one string. Qt then reads the whole thing
 * as a format, and the words are read as tokens too: the "m" of the German "um" comes out as the
 * minutes, the "s" of the Portuguese "às" as the seconds, the "a" and "t" of the English "at" as
 * am/pm and the time zone. Fifteen of the thirty-one translations of these strings were mangled
 * that way, in whichever language their author spoke.
 *
 * Anything here that is not a date token is quoted, and Qt writes quoted text out as it stands.
 * Only date tokens are honoured, because these prefixes carry a date and the time is added
 * separately -- which is why an "m" or an "s" in them is a letter of a word and not a request.
 */
QString asDateFormat(const QString &translated)
{
    static const QStringList tokens = {
        QStringLiteral("dddd"), QStringLiteral("ddd"),  QStringLiteral("dd"), QStringLiteral("d"),
        QStringLiteral("MMMM"), QStringLiteral("MMM"),  QStringLiteral("MM"), QStringLiteral("M"),
        QStringLiteral("yyyy"), QStringLiteral("yy")};

    QString format;
    QString literal;

    auto const flush = [&format, &literal]() {
        if (literal.isEmpty())
            return;
        // A quote of its own is written twice to stand for itself.
        format += QLatin1Char('\'') + QString{literal}.replace(QLatin1Char('\''), QStringLiteral("''")) +
                  QLatin1Char('\'');
        literal.clear();
    };

    for (int at = 0; at < translated.length();)
    {
        auto const token = std::find_if(
            tokens.begin(), tokens.end(),
            [&translated, at](const QString &candidate) { return translated.mid(at, candidate.length()) == candidate; });

        if (token == tokens.end())
        {
            literal += translated.at(at);
            ++at;
            continue;
        }

        flush();
        format += *token;
        at += token->length();
    }

    flush();

    return format;
}
}

QString printDateTime(bool niceDateFormat, const QDateTime &datetime)
{
    QString ret;
    QDateTime current_date;
    int delta;

    current_date.setSecsSinceEpoch(static_cast<qint64>(time(NULL)));
    //	current_date.setTime(QTime(0, 0));

    delta = datetime.daysTo(current_date);
    ret = datetime.toString("hh:mm:ss");

    if (delta != 0)
    {
        if (niceDateFormat)
        {
            if (delta == 1)   // 1 day ago
                ret.prepend(QCoreApplication::translate("@default", "Yesterday at "));
            else if (delta < 7)   // less than week ago
            {
                // Asked of the locale rather than of the date. QDateTime::toString(format) spells
                // the day out in English whatever language is in use -- it reads the names from the
                // C locale -- so a Polish Kadu showed "Friday o 13:19:57": the word between them
                // translated, the day not. The default locale is set from the language Kadu was
                // asked to speak, so the two now come from the same place.
                ret.prepend(QLocale{}.toString(
                    datetime, asDateFormat(QCoreApplication::translate("@default", "dddd at "))));
                ret[0] = ret.at(0).toUpper();   // looks ugly lowercase ;)
            }
            else if (delta < 14)
            {
                int tmp = delta % 7;
                if (tmp == 0)
                    ret.prepend(QCoreApplication::translate("@default", "week ago at "));
                else if (tmp == 1)
                    ret.prepend(QCoreApplication::translate("@default", "week and day ago at "));
                else
                    ret.prepend(QCoreApplication::translate("@default", "week and %2 days ago at ").arg(delta % 7));
            }
            else if (delta < 6 * 7)
            {
                int tmp = delta % 7;
                if (tmp == 0)
                    ret.prepend(QCoreApplication::translate("@default", "%1 weeks ago at ").arg(delta / 7));
                else if (tmp == 1)
                    ret.prepend(QCoreApplication::translate("@default", "%1 weeks and day ago at ").arg(delta / 7));
                else
                    ret.prepend(QCoreApplication::translate("@default", "%1 weeks and %2 days ago at ")
                                    .arg(delta / 7)
                                    .arg(delta % 7));
            }
            else
                ret.prepend(QLocale{}.toString(
                    datetime, asDateFormat(QCoreApplication::translate("@default", "d MMMM yyyy at "))));
        }
        else
            ret.append(datetime.toString(" (dd.MM.yyyy)"));
    }
    return ret;
}

void getTime(time_t *sec, long int *msec)
{
#ifdef Q_OS_WIN
    struct _timeb timebuffer;

    _ftime(&timebuffer);

    *sec = timebuffer.time;
    *msec = timebuffer.millitm * 1000;
#else
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    *sec = tv.tv_sec;
    *msec = tv.tv_usec * 1000;
#endif
}
