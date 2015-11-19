#ifndef SANE_UTILS_H
#define SANE_UTILS_H

#include <stdlib.h>
#include <sane/sane.h>

SANE_Int GetSaneInt(SANE_Handle device, const char *name);
float GetSaneFloat(SANE_Handle device, const char *name);
float GetSaneMaxFloat(SANE_Handle device, const char *name);
float GetSaneMinFloat(SANE_Handle device, const char *name);

void SetSaneFloat(SANE_Handle device, const char *name, float val);
void SetSaneInt(SANE_Handle device, const char *name, SANE_Int val);

#endif
