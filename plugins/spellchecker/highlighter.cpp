/*
 * %kadu copyright begin%
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QRegularExpression>

#include "spellchecker.h"

#include "highlighter.h"
#include "highlighter.moc"

QList<Highlighter *> Highlighter::Highlighters;
QTextCharFormat Highlighter::HighlightFormat;

void Highlighter::rehighlightAll()
{
    for (auto highlighter : Highlighters)
        highlighter->rehighlight();
}

Highlighter::Highlighter(SpellChecker *spellChecker, QTextDocument *document)
        : QSyntaxHighlighter(document), m_spellChecker{spellChecker}
{
    Highlighters.append(this);
}

Highlighter::~Highlighter()
{
    Highlighters.removeAll(this);
}

void Highlighter::highlightBlock(const QString &text)
{
    // Both \b and \w are Unicode-aware in QRegExp but ASCII-only in PCRE2 unless asked
    // otherwise. Without UseUnicodePropertiesOption every Polish diacritic would end a word and
    // the spell checker would be handed fragments of it.
    static const QRegularExpression word{
        QStringLiteral("\\b\\w+\\b"), QRegularExpression::UseUnicodePropertiesOption};

    auto matches = word.globalMatch(text);
    while (matches.hasNext())
    {
        auto const match = matches.next();
        if (!m_spellChecker->checkWord(match.captured()))
            setFormat(match.capturedStart(), match.capturedLength(), HighlightFormat);
    }
}

void Highlighter::setHighlightFormat(const QTextCharFormat &format)
{
    HighlightFormat = format;
}

void Highlighter::removeAll()
{
    for (auto highlighter : Highlighters)
        delete highlighter;
    Q_ASSERT(Highlighters.isEmpty());
}
