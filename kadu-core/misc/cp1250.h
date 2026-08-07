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

#include <QtCore/QByteArray>
#include <QtCore/QString>

#include "exports.h"

/**
 * @addtogroup Misc
 * @{
 */

/**
 * @short Decode CP1250 (Windows-1250) text.
 * @param encoded bytes in CP1250
 * @return the decoded text
 *
 * Two file formats Kadu still reads are CP1250: Gadu-Gadu contact lists exported before version
 * 7.0, and the emots.txt of a Gadu-Gadu emoticon theme.
 *
 * Qt6 dropped QTextCodec, and QStringConverter -- what replaced it -- only supports a fixed set of
 * encodings that does not include CP1250. Rather than keep the whole Qt5Compat module linked for
 * one single-byte code page, it is decoded here.
 */
KADUAPI QString cp1250ToUnicode(const QByteArray &encoded);

/**
 * @}
 */
