#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

#include "activate.h"

#include "core/core.h"
#include "kadu-config.h"

#if HAVE_KWINDOWSYSTEM
#include <KWindowSystem>
#endif

#if defined(Q_OS_WIN)

#include <stdio.h>
#include <windows.h>

bool _isActiveWindow(QWidget *window)
{
    // !isMinimized() is a workaround for QTBUG-19026
    return window->isActiveWindow() && !window->isMinimized();
}

void _activateWindow(Configuration *configuration, QWidget *window)
{
    Q_UNUSED(configuration);

    window = window->window();
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    window->show();
    window->raise();
    window->activateWindow();
    SetForegroundWindow((HWND)(window->winId()));
}

#else

bool _isActiveWindow(QWidget *window)
{
    return window->window()->isActiveWindow();
}

void _activateWindow(Configuration *configuration, QWidget *window)
{
    // Whether a window may raise itself is the compositor's decision, not ours. Under Wayland a
    // client presents an activation token instead of ordering the window to the front, and a
    // request without one is ignored by design -- so the most that can be done here is to ask.
    Q_UNUSED(configuration);

    window = window->window();
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    window->show();
    window->raise();

#if HAVE_KWINDOWSYSTEM
    // Carries the activation token when there is one, and falls back to the plain request when
    // there is not.
    if (auto *windowHandle = window->windowHandle())
        KWindowSystem::activateWindow(windowHandle);
    else
        window->activateWindow();
#else
    window->activateWindow();
#endif
}

#endif

bool _isWindowActiveOrFullyVisible(QWidget *window)
{
    // "Fully visible" used to mean asking X11 whether the window was shaded, on another desktop or
    // covered by something else. A Wayland client is told none of that: it learns that it is
    // active or that it is not, and nothing about its position among other windows. The answer is
    // therefore the same one every non-X11 platform has always given here.
    return _isActiveWindow(window);
}
