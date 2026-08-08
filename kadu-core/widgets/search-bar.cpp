/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QStyle>

#include "search-bar.h"
#include "search-bar.moc"

SearchBar::SearchBar(QWidget *parent) : QToolBar(parent), AutoVisibility(true)
{
    createGui();

    hide();
}

SearchBar::~SearchBar()
{
}

void SearchBar::createGui()
{
    // The three buttons are the tool bar's own, made from actions, rather than tool buttons built
    // here and handed over. Built here they were given a fixed size of sixteen pixels, which is a
    // number and not a measurement: the icon inside is drawn at whatever size the style works out
    // for the screen, and where that came to more than sixteen the button ended up smaller than the
    // symbol it held -- the frame under the pointer cutting through the arrows instead of going
    // round them. A tool bar asks the style how big its icons should be, and asks again whenever
    // the style changes, so the size is left to it and not named here at all.

    auto *closeAction = addAction(qApp->style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Close"));
    // The same as Escape. It used to only hide the bar, leaving what had been found still marked.
    connect(closeAction, &QAction::triggered, this, &SearchBar::close);

    addWidget(new QLabel(tr("Find:"), this));

    FindEdit = new QLineEdit(this);
    connect(FindEdit, &QLineEdit::textChanged, this, &SearchBar::searchTextChanged);
    addWidget(FindEdit);

    auto *previousAction = addAction(qApp->style()->standardIcon(QStyle::SP_ArrowLeft), tr("Previous"));
    connect(previousAction, &QAction::triggered, this, &SearchBar::previous);

    auto *nextAction = addAction(qApp->style()->standardIcon(QStyle::SP_ArrowRight), tr("Next"));
    connect(nextAction, &QAction::triggered, this, &SearchBar::next);
}

void SearchBar::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_F == event->key() && Qt::ControlModifier == event->modifiers())
    {
        event->accept();
        close();
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Escape:
    {
        event->accept();
        close();
        break;
    }

    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_F3:
    {
        if (Qt::ShiftModifier == event->modifiers())
            previous();
        else
            next();
        break;
    }

    default:
    {
        somethingFound(true);

        QWidget::keyPressEvent(event);
    }
    }
}

void SearchBar::showEvent(QShowEvent *event)
{
    FindEdit->setText(QString());
    QWidget::showEvent(event);
}

void SearchBar::setSearchWidget(QWidget *const widget)
{
    if (SearchWidget)
        SearchWidget->removeEventFilter(this);

    SearchWidget = widget;

    if (SearchWidget)
        SearchWidget->installEventFilter(this);
}

void SearchBar::setAutoVisibility(bool autoVisibility)
{
    if (AutoVisibility == autoVisibility)
        return;

    AutoVisibility = autoVisibility;
    if (!AutoVisibility)
        show();
}

bool SearchBar::eventFilter(QObject *object, QEvent *event)
{
    if (QEvent::KeyPress != event->type() || object != SearchWidget.data())
        return false;

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if ((Qt::Key_F == keyEvent->key() && Qt::ControlModifier == keyEvent->modifiers()) ||
        (Qt::Key_F3 == keyEvent->key()))
    {
        show();
        FindEdit->setFocus();
        return true;
    }

    return false;
}

void SearchBar::setSearchText(const QString &search)
{
    FindEdit->setText(search);
}

void SearchBar::previous()
{
    if (!FindEdit->text().isEmpty())
        emit searchPrevious(FindEdit->text());
}

void SearchBar::next()
{
    if (!FindEdit->text().isEmpty())
        emit searchNext(FindEdit->text());
}

void SearchBar::close()
{
    FindEdit->setText(QString());
    emit clearSearch();

    if (AutoVisibility)
        hide();

    if (SearchWidget)
        SearchWidget->setFocus();
}

void SearchBar::searchTextChanged(const QString &text)
{
    Q_UNUSED(text)

    somethingFound(true);
}

void SearchBar::somethingFound(bool found)
{
    QString style = "";

    if (!found)
        style = "QLineEdit{background: #FFB4B4;}";

    FindEdit->setStyleSheet(style);
}
