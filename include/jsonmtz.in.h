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

#ifndef _JSONMTZ_H_
#define _JSONMTZ_H_

#define VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define VERSION_MINOR @PROJECT_VERSION_MINOR@
#define VERSION_PATCH @PROJECT_VERSION_PATCH@

#include "jansson.h"
#include "cmtzlib.h"

typedef struct options_mtz2json
{
    int compact;
    int help;
    int version;
    int timestamp;
    int force;
} options_mtz2json;

typedef struct options_json2mtz
{
    int help;
    int version;
    int timestamp;
    int force;
} options_json2mtz;

json_t *readMtz(const MTZ *mtzin);
json_t *readMtzBatch(const MTZBAT *batch);
json_t *readMtzXtal(const MTZXTAL *xtal, int nref, const MTZ *mtzin);
json_t *readMtzSet(const MTZSET *set, int nref, const MTZ *mtzin);
json_t *readMtzSymmetry(SYMGRP sym);
int mtz2json(const char *file_in, const char *file_out, const options_mtz2json *opts);
int json2mtz(const char *file_in, const char *file_out, const options_json2mtz *opts);
MTZ *makeMtz(json_t *json);
MTZ *setMtzSymmetry(MTZ *mtzout, json_t *jsymm);
MTZ *setMtzBatches(MTZ *mtzout, const json_t *jbatches);
MTZ *setMtzXtals(MTZ *mtzout, const json_t *jcrystals);
MTZSET *setMtzSet(MTZSET *xtal, json_t *jset, MTZ *mtzout);
MTZCOL *findColumnBySource(const MTZ *mtzout, int source);
int json_array_is_homogenous_object(const json_t *json);
int json_array_is_homogenous_array(const json_t *json);
int json_array_is_homogenous_string(const json_t *json);
int json_array_is_homogenous_integer(const json_t *json);
int json_array_is_homogenous_real(const json_t *json);
int json_array_check_dimensions(const json_t *json, const int *dim, int len);
int json_array_check_dimensions_f(const json_t *json, const int *dim, int len, int (*inner_check_function)(const json_t *json));
int json_truth(const json_t *);
char *makeTimestamp(const char *jobstring, const char *datestring, char *timestamp);
char *stringtrimn(const char *str, int len);

#endif
