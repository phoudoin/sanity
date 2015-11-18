#ifndef SANE_UTILS_H
#define SANE_UTILS_H

#include <stdlib.h>
#include <sane/sane.h>

float GetSaneVal(SANE_Handle device, const char *name);
void SetSaneVal(SANE_Handle device, const char *name, float val);
float GetSaneMaxVal(SANE_Handle device, const char *name);

#endif
