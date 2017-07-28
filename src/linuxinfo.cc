/*  =========================================================================
    linuxinfo - Class for finding out Linux system info

    Copyright (C) 2014 - 2017 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/*
@header
    linuxinfo - Class for finding out Linux system info
@discuss
@end
*/

#include "fty_info_classes.h"

///////////////////////////////////////////
// Static functions which get metrics values

static linuxinfo_t *
s_uptime (void)
{
    const char *path = "/proc/uptime";
    FILE *uptimef = fopen (path, "r");
    if (!uptimef) {
        zsys_error ("Could not open '%s'", path);
        return NULL;
    }
    double uptime, idletime;
    fscanf (uptimef, "%lf %lf", &uptime, &idletime);

    linuxinfo_t *uptime_info = linuxinfo_new ();
    uptime_info->type = "uptime";
    uptime_info->value = uptime;
    uptime_info->unit = "sec";

    return uptime_info;
}

//  --------------------------------------------------------------------------
//  Create a new linuxinfo

linuxinfo_t *
linuxinfo_new (void)
{
    linuxinfo_t *self = (linuxinfo_t *) zmalloc (sizeof (linuxinfo_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the linuxinfo

void
linuxinfo_destroy (linuxinfo_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        linuxinfo_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//--------------------------------------------------------------------------
//// Create zlistx containing all Linux system info

zlistx_t *
linuxinfo_get_all (void)
{
    zlistx_t *info = zlistx_new ();
    zlistx_set_destructor (info, (void (*)(void**)) linuxinfo_destroy);
    linuxinfo_t *uptime = s_uptime ();
    zlistx_add_end (info, uptime);
    return info;
}
