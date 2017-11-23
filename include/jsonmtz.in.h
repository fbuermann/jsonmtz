/*
 * Copyright (c) 2017 Frank Buermann <fburmann@mrc-lmb.cam.ac.uk>
 * 
 * mtz2json is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 * This software makes use of the jansson library (http://www.digip.org/jansson/)
 * licensed under the terms of the MIT license,
 * and the CCP4io library (http://www.ccp4.ac.uk/) licensed under the
 * Lesser GNU General Public License 3.0.
 */

#pragma once

#define VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define VERSION_MINOR @PROJECT_VERSION_MINOR@
#define VERSION_PATCH @PROJECT_VERSION_PATCH@

#include <stdint.h>
#include <stdbool.h>
#include "jansson.h"
#include "cmtzlib.h"

typedef struct options_mtz2json_t
{
    bool compact;
    bool help;
    bool version;
    bool timestamp;
    bool force;
} options_mtz2json_t;

typedef struct options_json2mtz_t
{
    bool help;
    bool version;
    bool timestamp;
    bool force;
} options_json2mtz_t;

json_t *readMtz(const MTZ *mtzin);
json_t *readMtzBatch(const MTZBAT *batch);
json_t *readMtzXtal(const MTZXTAL *xtal, size_t nref, const MTZ *mtzin);
json_t *readMtzSet(const MTZSET *set, size_t nref, const MTZ *mtzin);
json_t *readMtzSymmetry(SYMGRP sym);
uint8_t mtz2json(const char *file_in, const char *file_out, const options_mtz2json_t *opts);
uint8_t json2mtz(const char *file_in, const char *file_out, const options_json2mtz_t *opts);
MTZ *makeMtz(json_t *json);
MTZ *setMtzSymmetry(MTZ *mtzout, json_t *jsymm);
MTZ *setMtzBatches(MTZ *mtzout, const json_t *jbatches);
MTZ *setMtzXtals(MTZ *mtzout, const json_t *jcrystals);
MTZSET *setMtzSet(MTZSET *xtal, json_t *jset, MTZ *mtzout);
MTZCOL *findColumnBySource(const MTZ *mtzout, size_t source);
uint8_t json_array_is_homogenous_object(const json_t *json);
uint8_t json_array_is_homogenous_array(const json_t *json);
uint8_t json_array_is_homogenous_string(const json_t *json);
uint8_t json_array_is_homogenous_integer(const json_t *json);
uint8_t json_array_is_homogenous_real(const json_t *json);
uint8_t json_array_check_dimensions(const json_t *json, const size_t *dim, size_t len);
uint8_t json_array_check_dimensions_f(const json_t *json, const size_t *dim, size_t len, uint8_t (*inner_check_function)(const json_t *json));
uint8_t json_truth(const json_t *);
char *makeTimestamp(const char *jobstring, const char *datestring, char *timestamp);
char *stringtrimn(const char *str, size_t len);