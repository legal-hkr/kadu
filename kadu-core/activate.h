#ifndef ACTIVATE_H
#define ACTIVATE_H

class Configuration;

class QWidget;

#include "exports.h"

bool KADUAPI _isActiveWindow(QWidget *window);
void KADUAPI _activateWindow(Configuration *configuration, QWidget *window);

/**
 * Returns true if passed widget's window is active or fully visible to the user.
 *
 * NOTE: Whether a window is covered, shaded or on another desktop is knowledge a Wayland client is
 * not given, so this is equivalent to QWidget::isActiveWindow().
 */
bool KADUAPI _isWindowActiveOrFullyVisible(QWidget *window);

#endif   // ACTIVATE_H
