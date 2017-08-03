/*  =========================================================================
    linuxmetric - Class for finding out Linux system info

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

#ifndef LINUXMETRIC_H_INCLUDED
#define LINUXMETRIC_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct _linuxmetric_t {
    const char *type;
    double value;
    const char *unit;
};

//  @interface
//  Create a new linuxmetric
FTY_INFO_EXPORT linuxmetric_t *
    linuxmetric_new (void);

//  Destroy the linuxmetric
FTY_INFO_EXPORT void
    linuxmetric_destroy (linuxmetric_t **self_p);

// Create zlistx containing all Linux system info
FTY_INFO_EXPORT zlistx_t *
    linuxmetric_get_all (int interval, std::map<std::string, double> &network_bytes);

//  @end

#ifdef __cplusplus
}
#endif

#endif
