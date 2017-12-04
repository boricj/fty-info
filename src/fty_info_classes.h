/*  =========================================================================
    fty_info_classes - private header file

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
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
    =========================================================================
*/

#ifndef FTY_INFO_CLASSES_H_INCLUDED
#define FTY_INFO_CLASSES_H_INCLUDED

//  Platform definitions, must come first
#include "platform.h"

//  External API
#include "../include/fty_info.h"

//  Extra headers

//  Opaque class structures to allow forward references
#ifndef TOPOLOGYRESOLVER_T_DEFINED
typedef struct _topologyresolver_t topologyresolver_t;
#define TOPOLOGYRESOLVER_T_DEFINED
#endif
#ifndef FTYINFO_T_DEFINED
typedef struct _ftyinfo_t ftyinfo_t;
#define FTYINFO_T_DEFINED
#endif
#ifndef FTY_INFO_RC0_RUNONCE_T_DEFINED
typedef struct _fty_info_rc0_runonce_t fty_info_rc0_runonce_t;
#define FTY_INFO_RC0_RUNONCE_T_DEFINED
#endif

//  Internal API

#include "topologyresolver.h"
#include "ftyinfo.h"
#include "fty_info_rc0_runonce.h"

//  *** To avoid double-definitions, only define if building without draft ***
#ifndef FTY_INFO_BUILD_DRAFT_API

//  *** Draft method, defined for internal use only ***
//  Self test of this class.
FTY_INFO_PRIVATE void
    topologyresolver_test (bool verbose);

//  *** Draft method, defined for internal use only ***
//  Self test of this class.
FTY_INFO_PRIVATE void
    ftyinfo_test (bool verbose);

//  *** Draft method, defined for internal use only ***
//  Self test of this class.
FTY_INFO_PRIVATE void
    fty_info_rc0_runonce_test (bool verbose);

//  Self test for private classes
FTY_INFO_PRIVATE void
    fty_info_private_selftest (bool verbose);

#endif // FTY_INFO_BUILD_DRAFT_API

#endif
