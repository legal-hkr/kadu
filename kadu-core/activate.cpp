#include <QtWidgets/QWidget>

#include "activate.h"

#include "core/core.h"

#if defined(Q_OS_UNIX)

#include "compat/x11-display.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "os/x11/x11tools.h"
#undef KeyPress
#undef Status

bool _isActiveWindow(QWidget *window)
{
    // we need to ensure we operate on widget's window, if not passed
    window = window->window();

    if (X11_isWindowShaded(kaduX11Display(), window->winId()))   // not needed in Qt 5.4
        return false;
    if (X11_isWindowMinimized(kaduX11Display(), window->winId()))   // not needed in Qt 5.4
        return false;

    // desktop
    unsigned long desktopofwindow = X11_getDesktopOfWindow(kaduX11Display(), window->winId());
    if ((desktopofwindow != X11_ALLDESKTOPS) && (desktopofwindow != X11_NODESKTOP) &&
        (desktopofwindow != X11_getCurrentDesktop(kaduX11Display())))
        return false;
    // standard isActiveWindow() method
    return window->isActiveWindow();
}

void _activateWindow(Configuration *configuration, QWidget *window)
{
    // we need to ensure we operate on widget's window, if not passed
    window = window->window();
    // unminimize
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    // show window (in case it's hidden)
    window->show();
    // unshade the window if needed (important!)
    if (X11_isWindowShaded(kaduX11Display(), window->winId()))
        X11_shadeWindow(kaduX11Display(), window->winId(), false);
    // read user settings
    int action = configuration->deprecatedApi()->readNumEntry("General", "WindowActivationMethod");
    // window & desktop
    if (X11_getDesktopsCount(kaduX11Display()) > 1)
    {
        auto desktopofwindow = X11_getDesktopOfWindow(kaduX11Display(), window->winId());
        auto currentdesktop = X11_getCurrentDesktop(kaduX11Display());
        if ((desktopofwindow != currentdesktop) && (desktopofwindow != X11_ALLDESKTOPS))
        {
            if ((action == 1) && (desktopofwindow != X11_NODESKTOP))
            {
                X11_setCurrentDesktop(kaduX11Display(), desktopofwindow);
            }
            else
            {
                if (X11_isWholeWindowOnOneDesktop(kaduX11Display(), window->winId()))
                    X11_moveWindowToDesktop(kaduX11Display(), window->winId(), currentdesktop);
                else
                    X11_centerWindow(kaduX11Display(), window->winId(), currentdesktop);
            }
        }
    }
    // activate
    X11_setActiveWindow(kaduX11Display(), window->winId());
    window->raise();
    window->activateWindow();
}

#elif defined(Q_OS_WIN)

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
    return window->isActiveWindow();
}

void _activateWindow(Configuration *configuration, QWidget *window)
{
    Q_UNUSED(configuration);

    window = window->window();
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    window->show();
    window->raise();
    window->activateWindow();
}

#endif

bool _isWindowActiveOrFullyVisible(QWidget *window)
{
#if defined(Q_OS_UNIX)
    if (_isActiveWindow(window))
        return true;

    if (X11_isWindowShaded(kaduX11Display(), window->winId()))   // not needed in Qt 5.4
        return false;
    if (X11_isWindowMinimized(kaduX11Display(), window->winId()))   // not needed in Qt 5.4
        return false;

    // we need to ensure we operate on widget's window, if not passed
    window = window->window();

    Display *display = kaduX11Display();
    WId wId = window->winId();
    return !window->isMinimized() && X11_isWindowOnDesktop(display, wId, X11_getCurrentDesktop(display)) &&
           X11_isWholeWindowOnOneDesktop(display, wId) && !X11_isWindowCovered(display, wId);
#else
    return _isActiveWindow(window);
#endif
}
