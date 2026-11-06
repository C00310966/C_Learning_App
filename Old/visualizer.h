#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <stdbool.h>  // Allows us to use bool, true and false

// Reset the demonstration to its starting values.
void ResetVisualizer(void);

// Draw and update the interactive pointer screen.
// Returns true when the user clicks Back.
bool DrawPointerVisualizer(void);

#endif
