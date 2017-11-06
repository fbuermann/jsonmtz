/* 
 * jsonmtz.c: JSON<->MTZ conversion library
 * 
 * Copyright (c) 2017 Frank Buermann <fburmann@mrc-lmb.cam.ac.uk>
 * 
 * mtz2json is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 * This software makes use of the jansson library (http://www.digip.org/jansson/)
 * licensed under the terms of the MIT license,
 * and the CCP4io library (http://www.ccp4.ac.uk/) licensed under the
 * Lesser GNU General Public License 3.0.
 */

/**
 * @mainpage jsonmtz - An MTZ to JSON converter
 * 
 * @section Introduction
 * 
 * This program can be used to interconvert between the binary reflection file
 * format MTZ, commonly used in X-ray crystallography, and the widespread
 * human-readable data exchange format JSON. The program contains two executables.
 * mtz2json converts MTZ reflection files to JSON format, and json2mtz does the
 * reverse. jsonmtz can easily be used as a C library. 
 * This program contains two executables. mtz2json converts MTZ files to 
 * JSON format, and json2mtz does the reverse.
 * 
 * @section Usage Example usage
 * $ mtz2json <i>in.mtz</i> <i>out.json</i>
 * 
 * $ json2mtz <i>in.json</i> <i>out.mtz</i>
 * 
 * @section Building Building from Source
 * Use <a href="https://cmake.org/">CMake</a> to build from source.
 * 
 * @subsection UNIX UNIX-like operating systems
 * $ cd jsonmtz\n 
 * $ mkdir build\n
 * $ cd build\n
 * $ cmake -G "Unix Makefiles" ..\n
 * $ make
 * 
 * @subsection WIN32 Windows
 * Building from source has been tested with the <a href="http://mingw.org/">MinGW</a> compiler, 
 * which should be on the path in addition to <a href="https://cmake.org/">CMake</a>. In PowerShell:\n
 * \> cd jsonmtz\n
 * \> mkdir build\n
 * \> cd build\n
 * \> cmake -G "MinGW Makefiles" ..\n
 * \> mingw32-make
 * 
 * @subsection Library Use as a (static) library
 * <b>jsonmtz</b> can be used as a C library. Build <b>jsonmtz</b> from source. 
 * Include the <i>jsonmtz.h</i> header in your code and make sure that all 
 * <i>ccp4io</i> headers and the <i>jansson.h</i> / <i>jansson_config.h</i> 
 * headers are on the include path. Link against <i>libjsonmtz.a</i>, 
 * <i>libcmtz.a</i> and <i>libjansson.a</i>. On linux platforms, also link 
 * against <i>libm.a</i>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "jsonmtz.h"

/**
 * Converts an MTZ reflection file to a JSON file.
 * @param[in] file_in The input MTZ file.
 * @param[in] file_out The ouptut JSON file.
 * @param[in] opts Options struct.
 * @return 0 on success, 1 on failure.
 */

int mtz2json(const char *file_in, const char *file_out, const options_mtz2json *opts)
{
    MTZ *mtzin = NULL;
    json_t *jsonmtz = NULL;
    time_t current_time;
    char *timestring = NULL;
    char *hist = NULL;
    char timestamp[80];
    char jobstring[57];
    int ret;
    int format = JSON_INDENT(4);

    if (access(file_in, F_OK | R_OK) == -1) // Input not readable
    {
        return 2;
    }

    mtzin = MtzGet(file_in, 1);
    MtzAssignHKLtoBase(mtzin);

    // Add timestamp
    if (opts->timestamp)
    {
        snprintf(jobstring, 56, "mtz2json v%d.%d.%d run on", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        time(&current_time);
        timestring = ctime(&current_time);
        makeTimestamp(jobstring, timestring, timestamp);
        hist = MtzCallocHist(mtzin->histlines + 1);
        strncpy(hist, mtzin->hist, MTZRECORDLENGTH * mtzin->histlines);
        strncpy(hist + MTZRECORDLENGTH * mtzin->histlines, timestamp, MTZRECORDLENGTH);
        MtzFreeHist(mtzin->hist);
        mtzin->hist = hist;
        mtzin->histlines += 1;
    }

    jsonmtz = readMtz(mtzin);
    MtzFree(mtzin);

    opts->compact ? format = 0 : 0;

    ret = json_dump_file(jsonmtz, file_out, format | JSON_COMPACT);
    json_decref(jsonmtz);

    return ret;
}

/**
 * Converts a JSON reflection file to MTZ format.
 * @param[in] file_in The input file.
 * @param[in] file_out The output file.
 * @param[in] opts Options struct.
 * @return 0 for success, other error codes for failure.
 */

int json2mtz(const char *file_in, const char *file_out, const options_json2mtz *opts)
{
    MTZ *mtzout = NULL;
    json_t *json;
    json_error_t err;
    time_t current_time;
    char *timestring = NULL;
    char *hist = NULL;
    char timestamp[80];
    char jobstring[57];
    int ret;

    json = json_load_file(file_in, 0, &err);

    if (!json)
    {
        // Unable to read JSON file
        return 1;
    }

    mtzout = makeMtz(json);

    if (!mtzout)
    {
        // Unable to make MTZ file
        return 2;
    }

    // Add timestamp
    if (opts->timestamp)
    {
        snprintf(jobstring, 56, "json2mtz v%d.%d.%d run on", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        time(&current_time);
        timestring = ctime(&current_time);
        makeTimestamp(jobstring, timestring, timestamp);
        hist = MtzCallocHist(mtzout->histlines + 1);
        strncpy(hist, mtzout->hist, MTZRECORDLENGTH * mtzout->histlines);
        strncpy(hist + MTZRECORDLENGTH * mtzout->histlines, timestamp, MTZRECORDLENGTH);
        MtzFreeHist(mtzout->hist);
        mtzout->hist = hist;
        mtzout->histlines += 1;
    }

    MtzPut(mtzout, file_out);
    MtzFree(mtzout);
    json_decref(json);

    return 0;
}

/**
 * Trims trailing whitespaces from a string and adds a null terminator.
 * @param[in] str The string.
 * @param[in] len The string length (including null terminator).
 * @return The string
 */

char *stringtrimn(const char *str, int len)
{
    char *start = malloc((len + 1) * sizeof(char));
    char *end = start + len;
    memset(start, '\0', len + 1);

    strncpy(start, str, len);

    while (*end == '\0' || *end == ' ')
    {
        end--;
    }

    *(end + 1) = '\0';

    return start;
}

/**
 * Reads an MTZ struct into a json object and returns a pointer to that object.
 * @param[in] mtzin The MTZ struct.
 * @return Pointer to json_t object.
 */

json_t *readMtz(const MTZ *mtzin)
{
    json_t *jsonmtz = json_object();
    json_t *jsonxtals = json_array();
    json_t *jsonbatches = json_array();
    MTZBAT *batch = NULL;
    int order[5] = {0};
    json_t *jorder = json_array();
    json_t *junknown_headers = json_array();
    json_t *jhist = json_array();

    // Read crystals
    for (int i = 0; i < mtzin->nxtal; i++)
    {
        json_array_append_new(jsonxtals, readMtzXtal(mtzin->xtal[i], mtzin->nref_filein, mtzin));
    }

    // Read batches
    batch = mtzin->batch;
    while (TRUE)
    {
        if (batch)
        {
            json_array_append_new(jsonbatches, readMtzBatch(batch));
            batch = batch->next;
        }
        else
        {
            break;
        }
    }

    // Read history
    for (int i = 0; i < mtzin->histlines; i++)
    {
        char *line;
        line = stringtrimn(mtzin->hist + i * MTZRECORDLENGTH, MTZRECORDLENGTH);
        json_array_append_new(jhist, json_string(line));
        free(line);
    }

    // Read sort order
    for (int i = 0; i < 5; i++)
    {
        if (mtzin->order[i])
        {
            json_array_append_new(jorder, json_integer(mtzin->order[i]->source));
        }
    }

    // Read unknown headers
    if (mtzin->n_unknown_headers)
    {
        for (int i = 0; i < mtzin->n_unknown_headers / 2; i++) // Bug in cmtzlib? Headers duplicate if not divided by 2.
        {
            char *line;
            line = stringtrimn(mtzin->unknown_headers + i * MTZRECORDLENGTH, MTZRECORDLENGTH);
            json_array_append_new(junknown_headers, json_string(line));
            free(line);
        }
    }

    // Populate object
    json_object_set_new(jsonmtz, "Title", json_string(mtzin->title));
    json_object_set_new(jsonmtz, "History", jhist);
    json_object_set_new(jsonmtz, "Crystals", jsonxtals);
    json_object_set_new(jsonmtz, "Symmetry", readMtzSymmetry(mtzin->mtzsymm));
    json_object_set_new(jsonmtz, "Batches", jsonbatches);
    json_object_set_new(jsonmtz, "SortOrder", jorder);
    json_object_set_new(jsonmtz, "UnknownHeaders", junknown_headers);

    return jsonmtz;
}

/**
 * Reads a SYMGRP struct into a json object and returns a pointer to that object.
 * @param[in] sym The SYMGRP struct.
 * @return Pointer to json_t object.
 */

json_t *readMtzSymmetry(SYMGRP sym)
{
    json_t *jsonsym = json_object();
    json_t *symop = json_array();

    // Read symmetry operations
    for (int i = 0; i < 192; i++)
    {
        json_t *symopOuter = json_array();
        for (int j = 0; j < 4; j++)
        {
            json_t *symopInner = json_array();
            for (int k = 0; k < 4; k++)
            {
                json_array_append_new(symopInner, json_real(sym.sym[i][j][k]));
            }
            json_array_append_new(symopOuter, symopInner);
        }
        json_array_append_new(symop, symopOuter);
    }

    // Populate object
    json_object_set_new(jsonsym, "SpaceGroupNumber", json_integer(sym.spcgrp));
    json_object_set_new(jsonsym, "SpaceGroupName", json_string(sym.spcgrpname));
    json_object_set_new(jsonsym, "PointGroupName", json_string(sym.pgname));
    json_object_set_new(jsonsym, "SpaceGroupConfidence", json_stringn(&(sym.spg_confidence), 1));
    json_object_set_new(jsonsym, "NumberOfSymmetryOperations", json_integer(sym.nsym));
    json_object_set_new(jsonsym, "NumberOfPrimitiveSymmetryOperations", json_integer(sym.nsymp));
    json_object_set_new(jsonsym, "SymmetryOperations", symop);
    json_object_set_new(jsonsym, "LatticeType", json_stringn(&(sym.symtyp), 1));

    return jsonsym;
}

/**
 * Reads an MTZBAT struct into a json object and returns a pointer to that object.
 * @param[in] batch The MTZBAT struct.
 * @return Pointer to json_t object.
 */

json_t *readMtzBatch(const MTZBAT *batch)
{
    json_t *jsonbatch = json_object();
    json_t *cell = json_array();
    json_t *mosaicity = json_array();
    json_t *datum = json_array();
    json_t *detlm = NULL;
    json_t *vec1 = json_array();
    json_t *vec2 = json_array();
    json_t *vec3 = json_array();
    json_t *axesLabels = json_array();
    json_t *cellRefineFlags = json_array();
    json_t *missettingAngles;
    json_t *rotAxis = json_array();
    json_t *svec = json_array();
    json_t *idealsvec = json_array();
    json_t *umat = json_array();

    // Read cell dimensions
    for (int i = 0; i < 6; i++)
    {
        json_array_append_new(cell, json_real(batch->cell[i]));
    }

    // Read mosaicity
    for (int i = 0; i < 12; i++)
    {
        json_array_append_new(mosaicity, json_real(batch->crydat[i]));
    }

    // Read datum
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(datum, json_real(batch->datum[i]));
    }

    // Read detector limits
    detlm =
        json_pack("[[[f,f],[f,f]],[[f,f],[f,f]]]",
                  batch->detlm[0][0][0],
                  batch->detlm[0][0][1],
                  batch->detlm[0][1][0],
                  batch->detlm[0][1][1],
                  batch->detlm[1][0][0],
                  batch->detlm[1][0][1],
                  batch->detlm[1][1][0],
                  batch->detlm[1][1][1]);

    // Read vectors
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(vec1, json_real(batch->e1[i]));
        json_array_append_new(vec2, json_real(batch->e2[i]));
        json_array_append_new(vec3, json_real(batch->e3[i]));
    }

    // Read axes labels
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(axesLabels, json_string(batch->gonlab[i]));
    }

    // Read refinement flags for cells
    for (int i = 0; i < 6; i++)
    {
        json_array_append_new(cellRefineFlags, json_integer(batch->lbcell[i]));
    }

    // Read missetting angles
    missettingAngles =
        json_pack(
            "[[f,f,f],[f,f,f]]",
            batch->phixyz[0][0],
            batch->phixyz[0][1],
            batch->phixyz[0][2],
            batch->phixyz[1][0],
            batch->phixyz[1][1],
            batch->phixyz[1][2]);

    // Read rotation axis
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(rotAxis, json_real(batch->scanax[i]));
    }

    // Read source vector
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(svec, json_real(batch->so[i]));
    }

    // Read idealised source vector
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(idealsvec, json_real(batch->source[i]));
    }

    // Read orientation matrix
    for (int i = 0; i < 9; i++)
    {
        json_array_append_new(umat, json_real(batch->umat[i]));
    }

    // Populate object
    json_object_set_new(jsonbatch, "Title", json_string(batch->title));
    json_object_set_new(jsonbatch, "DatasetID", json_integer(batch->nbsetid));
    json_object_set_new(jsonbatch, "CrystalNumber", json_integer(batch->ncryst));
    json_object_set_new(jsonbatch, "BatchNumber", json_integer(batch->num));
    json_object_set_new(jsonbatch, "Wavelength", json_real(batch->alambd));
    json_object_set_new(jsonbatch, "CellDimensions", cell);
    json_object_set_new(jsonbatch, "OrientationMatrix", umat);
    json_object_set_new(jsonbatch, "TemperatureFactor", json_real(batch->bbfac));
    json_object_set_new(jsonbatch, "Scale", json_real(batch->bscale));
    json_object_set_new(jsonbatch, "Mosaicity", mosaicity);
    json_object_set_new(jsonbatch, "GoniostatDatum", datum);
    json_object_set_new(jsonbatch, "Dispersion", json_real(batch->delamb));
    json_object_set_new(jsonbatch, "CorrelatedComponent", json_real(batch->delcor));
    json_object_set_new(jsonbatch, "DetectorLimits", detlm);
    json_object_set_new(jsonbatch, "HorizontalBeamDivergence", json_real(batch->divhd));
    json_object_set_new(jsonbatch, "VerticalBeamDivergence", json_real(batch->divvd));
    json_object_set_new(jsonbatch, "DetectorDistance", json_pack("[f,f]", batch->dx[0], batch->dx[1]));
    json_object_set_new(jsonbatch, "Vector1", vec1);
    json_object_set_new(jsonbatch, "Vector2", vec2);
    json_object_set_new(jsonbatch, "Vector3", vec3);
    json_object_set_new(jsonbatch, "AxesLabels", axesLabels);
    json_object_set_new(jsonbatch, "OrientationBlockType", json_integer(batch->iortyp));
    json_object_set_new(jsonbatch, "GoniostatScanAxisNumber", json_integer(batch->jsaxs));
    json_object_set_new(jsonbatch, "JumpAxis", json_integer(batch->jumpax));
    json_object_set_new(jsonbatch, "CellRefinementFlags", cellRefineFlags);
    json_object_set_new(jsonbatch, "BeamInfoFlag", json_integer(batch->lbmflg));
    json_object_set_new(jsonbatch, "MosaicityModelFlag", json_integer(batch->lcrflg));
    json_object_set_new(jsonbatch, "DataTypeFlag", json_integer(batch->ldtype));
    json_object_set_new(jsonbatch, "MisFlag", json_integer(batch->misflg));
    json_object_set_new(jsonbatch, "NumberOfBatchScales", json_integer(batch->nbscal));
    json_object_set_new(jsonbatch, "NumberOfDetectors", json_integer(batch->ndet));
    json_object_set_new(jsonbatch, "NumberOfGoniostatAxes", json_integer(batch->ngonax));
    json_object_set_new(jsonbatch, "EndOfPhi", json_real(batch->phiend));
    json_object_set_new(jsonbatch, "PhiRange", json_real(batch->phirange));
    json_object_set_new(jsonbatch, "StartOfPhi", json_real(batch->phistt));
    json_object_set_new(jsonbatch, "MissettingAngles", missettingAngles);
    json_object_set_new(jsonbatch, "RotationAxis", rotAxis);
    json_object_set_new(jsonbatch, "BFactorSD", json_real(batch->sdbfac));
    json_object_set_new(jsonbatch, "BScaleSD", json_real(batch->sdbscale));
    json_object_set_new(jsonbatch, "SourceVector", svec);
    json_object_set_new(jsonbatch, "IdealisedSourceVector", idealsvec);
    json_object_set_new(jsonbatch, "Theta", json_pack("[f,f]", batch->theta[0], batch->theta[1]));
    json_object_set_new(jsonbatch, "StartTime", json_real(batch->time1));
    json_object_set_new(jsonbatch, "StopTime", json_real(batch->time2));

    return jsonbatch;
}

/**
 * Reads an MTZXTAL struct into a json object and returns a pointer to that object.
 * @param[in] xtal The MTZXTAL struct.
 * @param[in] nref Number of reflextions.
 * @param[in] mtzin The parental MTZ struct.
 * @return Pointer to json_t object.
 */

json_t *readMtzXtal(const MTZXTAL *xtal, int nref, const MTZ *mtzin)
{
    json_t *jsonxtal = json_object();
    json_t *jsonsets = json_array();
    json_t *cell = json_array();

    // Read cell constants
    for (int i = 0; i < 6; i++)
    {
        json_array_append_new(cell, json_real(xtal->cell[i]));
    }

    // Read datasets
    for (int i = 0; i < xtal->nset; i++)
    {
        json_t *set = json_object();
        set = readMtzSet(xtal->set[i], nref, mtzin);
        json_array_append_new(jsonsets, set);
    }

    // Populate object
    json_object_set_new(jsonxtal, "CrystalName", json_string(xtal->xname));
    json_object_set_new(jsonxtal, "CrystalID", json_integer(xtal->xtalid));
    json_object_set_new(jsonxtal, "CellConstants", cell);
    json_object_set_new(jsonxtal, "ProjectName", json_string(xtal->pname));
    json_object_set_new(jsonxtal, "ResolutionMax", json_real(xtal->resmax));
    json_object_set_new(jsonxtal, "ResolutionMin", json_real(xtal->resmin));
    json_object_set_new(jsonxtal, "Datasets", jsonsets);

    return jsonxtal;
}

/**
 * Reads an MTZSET into a json array and returns a pointer to that array.
 * @param[in] set The MTZSET to read.
 * @param[in] nref Number of reflections. 
 * @param[in] mtzin The parental MTZ struct.
 * @return Pointer to json_t array.
 */

json_t *readMtzSet(const MTZSET *set, int nref, const MTZ *mtzin)
{
    json_t *jset = json_object();
    json_t *jcols = json_array();

    json_object_set_new(jset, "DatasetName", json_string(set->dname));
    json_object_set_new(jset, "DatasetID", json_integer(set->setid));
    json_object_set_new(jset, "Wavelength", json_real(set->wavelength));

    // Read columns
    for (int i = 0; i < set->ncol; i++)
    {
        MTZCOL *col = set->col[i];
        json_t *column = json_object();
        json_t *reflections = json_array();

        // Read reflection data
        for (int i = 0; i < nref; i++)
        {
            float refl = col->ref[i];

            // Check for missing data
            if (ccp4_ismnf(mtzin, refl))
            {
                json_array_append_new(reflections, json_string("NaN"));
            }
            else
            {
                json_array_append_new(reflections, json_real(refl));
            }
        }

        // Populate object
        json_object_set_new(column, "ColumnSource", json_string(col->colsource));
        json_object_set_new(column, "GroupName", json_string(col->grpname));
        json_object_set_new(column, "GroupPosition", json_integer(col->grpposn));
        json_object_set_new(column, "GroupType", json_string(col->grptype));
        json_object_set_new(column, "Label", json_string(col->label));
        json_object_set_new(column, "MaxValue", json_real(col->max));
        json_object_set_new(column, "MinValue", json_real(col->min));
        json_object_set_new(column, "ColumnID", json_integer(col->source));
        json_object_set_new(column, "Type", json_string(col->type));
        json_object_set_new(column, "Data", reflections);

        json_array_append_new(jcols, column);
    }

    json_object_set_new(jset, "Columns", jcols);

    return jset;
}

/**
 * Checks if a json array contains only objects.
 * @param[in] json The json array.
 * @return 1 if true, 0 if false.
 */

int json_array_is_homogenous_object(const json_t *json)
{
    for (int i = 0; i < json_array_size(json); i++)
    {
        if (!json_is_object(json_array_get(json, i)))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if a json array contains only arrays.
 * @param[in] json The json array.
 * @return 1 if true, 0 if false.
 */

int json_array_is_homogenous_array(const json_t *json)
{
    for (int i = 0; i < json_array_size(json); i++)
    {
        if (!json_is_array(json_array_get(json, i)))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if a json array contains only strings.
 * @param[in] json The json array.
 * @return 1 if true, 0 if false.
 */

int json_array_is_homogenous_string(const json_t *json)
{
    for (int i = 0; i < json_array_size(json); i++)
    {
        if (!json_is_string(json_array_get(json, i)))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if a json array contains only integers.
 * @param[in] json The json array.
 * @return 1 if true, 0 if false.
 */

int json_array_is_homogenous_integer(const json_t *json)
{
    for (int i = 0; i < json_array_size(json); i++)
    {
        if (!json_is_integer(json_array_get(json, i)))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if a json array contains only reals.
 * @param[in] json The json array.
 * @return 1 if true, 0 if false.
 */

int json_array_is_homogenous_real(const json_t *json)
{
    for (int i = 0; i < json_array_size(json); i++)
    {
        if (!json_is_real(json_array_get(json, i)))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks the dimensions of a json array.
 * @param[in] json The json array.
 * @param[in] dim The dimensions array, e.g. [2, 3] for an array of dimensions arr[2][3].
 * @param[in] len Length of the dimensions array.
 * @return 1 if true, 0 if false.
 */

int json_array_check_dimensions(const json_t *json, const int *dim, int len)
{
    return json_array_check_dimensions_f(json, dim, len, &json_truth);
}

/**
 * Returns 1.
 * @param[in] json Any json object.
 * @return 1.
 */

int json_truth(const json_t *json)
{
    return 1;
}

/**
 * Checks the dimensions of a json array and checks the inner-most arrays with a function.
 * @param[in] json The json array.
 * @param[in] dim The dimensions array, e.g. [2, 3] for an array of dimensions arr[2][3].
 * @param[in] len Length of the dimensions array.
 * @param[in] inner_check_function Pointer to check function.
 * @return 1 if true, 0 if false.
 */

int json_array_check_dimensions_f(const json_t *json, const int *dim, int len, int (*inner_check_function)(const json_t *json))
{
    int index;
    json_t *value;

    if (len == 0 && json_array_size(json) == 0)
    {
        return 1;
    }

    if (len > 0 && json_array_size(json) != dim[0])
    {
        return 0;
    }

    if (len > 1)
    {
        json_array_foreach(json, index, value)
        {
            if (!json_array_check_dimensions_f(value, dim + 1, len - 1, inner_check_function))
            {
                return 0;
            }
        }
    }

    if (len == 1 && json_array_size(json) == dim[0])
    {
        return (*inner_check_function)(json);
    }

    if (len > 0)
    {
        return 1;
    }

    return 0;
}

/**
 * Transfer batch information from a json object to an MTZ struct.
 * @param[in] mtzout The MTZ struct.
 * @param[in] jbatch The batch json array.
 * @return The MTZ struct.
 */

MTZ *setMtzBatches(MTZ *mtzout, const json_t *jbatches)
{
    int batchindex;
    json_t *batchvalue;
    MTZBAT *currentBatch = NULL;

    if (json_array_size(jbatches) > 0)
    {
        mtzout->batch = MtzMallocBatch();
        if (!mtzout->batch)
        {
            return NULL;
        }
        currentBatch = mtzout->batch;
    }

    json_array_foreach(jbatches, batchindex, batchvalue)
    {
        json_t *jtitle = NULL;
        json_t *jnbsetid = NULL;
        json_t *jncryst = NULL;
        json_t *jnum = NULL;
        json_t *jalambd = NULL;
        json_t *jbbfac = NULL;
        json_t *jbscale = NULL;
        json_t *jdelamb = NULL;
        json_t *jdelcor = NULL;
        json_t *jdivhd = NULL;
        json_t *jdivvd = NULL;
        json_t *jiortyp = NULL;
        json_t *jjsaxs = NULL;
        json_t *jjumpax = NULL;
        json_t *jlbmflg = NULL;
        json_t *jlcrflg = NULL;
        json_t *jldtype = NULL;
        json_t *jmisflg = NULL;
        json_t *jnbscal = NULL;
        json_t *jndet = NULL;
        json_t *jngonax = NULL;
        json_t *jphiend = NULL;
        json_t *jphirange = NULL;
        json_t *jphistt = NULL;
        json_t *jsdbfac = NULL;
        json_t *jsdbscale = NULL;
        json_t *jtime1 = NULL;
        json_t *jtime2 = NULL;
        json_t *jcell = NULL;
        json_t *jumat = NULL;
        json_t *jcrydat = NULL;
        json_t *jdatum = NULL;
        json_t *jdetlm = NULL;
        json_t *jdx = NULL;
        json_t *je1 = NULL;
        json_t *je2 = NULL;
        json_t *je3 = NULL;
        json_t *jlbcell = NULL;
        json_t *jgonlab = NULL;
        json_t *jphixyz = NULL;
        json_t *jscanax = NULL;
        json_t *jso = NULL;
        json_t *jsource = NULL;
        json_t *jtheta = NULL;

        int detlmdimensions[] = {2, 2, 2};
        int jdxdimensions[] = {2};
        int vecdimensions[] = {3};
        int celldimensions[] = {6};
        int phixyzdimensions[] = {2, 3};
        int scanaxdimensions[] = {3};
        int sourcedimensions[] = {3};
        int thetadimensions[] = {2};
        int umatdimensions[] = {9};

        json_unpack(batchvalue, "{s:o}", "Title", &jtitle);
        json_unpack(batchvalue, "{s:o}", "DatasetID", &jnbsetid);
        json_unpack(batchvalue, "{s:o}", "CrystalNumber", &jncryst);
        json_unpack(batchvalue, "{s:o}", "BatchNumber", &jnum);
        json_unpack(batchvalue, "{s:o}", "Wavelength", &jalambd);
        json_unpack(batchvalue, "{s:o}", "TemperatureFactor", &jbbfac);
        json_unpack(batchvalue, "{s:o}", "Scale", &jbscale);
        json_unpack(batchvalue, "{s:o}", "Dispersion", &jdelamb);
        json_unpack(batchvalue, "{s:o}", "CorrelatedComponent", &jdelcor);
        json_unpack(batchvalue, "{s:o}", "HorizontalBeamDivergence", &jdivhd);
        json_unpack(batchvalue, "{s:o}", "VerticalBeamDivergence", &jdivvd);
        json_unpack(batchvalue, "{s:o}", "OrientationBlockType", &jiortyp);
        json_unpack(batchvalue, "{s:o}", "GoniostatScanAxisNumber", &jjsaxs);
        json_unpack(batchvalue, "{s:o}", "JumpAxis", &jjumpax);
        json_unpack(batchvalue, "{s:o}", "BeamInfoFlag", &jlbmflg);
        json_unpack(batchvalue, "{s:o}", "MosaicityModelFlag", &jlcrflg);
        json_unpack(batchvalue, "{s:o}", "DataTypeFlag", &jldtype);
        json_unpack(batchvalue, "{s:o}", "MisFlag", &jmisflg);
        json_unpack(batchvalue, "{s:o}", "NumberOfBatchScales", &jnbscal);
        json_unpack(batchvalue, "{s:o}", "NumberOfDetectors", &jndet);
        json_unpack(batchvalue, "{s:o}", "NumberOfGoniostatAxes", &jngonax);
        json_unpack(batchvalue, "{s:o}", "EndOfPhi", &jphiend);
        json_unpack(batchvalue, "{s:o}", "PhiRange", &jphirange);
        json_unpack(batchvalue, "{s:o}", "StartOfPhi", &jphistt);
        json_unpack(batchvalue, "{s:o}", "BFactorSD", &jsdbfac);
        json_unpack(batchvalue, "{s:o}", "BScaleSD", &jsdbscale);
        json_unpack(batchvalue, "{s:o}", "StartTime", &jtime1);
        json_unpack(batchvalue, "{s:o}", "StopTime", &jtime2);
        json_unpack(batchvalue, "{s:o}", "CellDimensions", &jcell);
        json_unpack(batchvalue, "{s:o}", "OrientationMatrix", &jumat);
        json_unpack(batchvalue, "{s:o}", "Mosaicity", &jcrydat);
        json_unpack(batchvalue, "{s:o}", "GoniostatDatum", &jdatum);
        json_unpack(batchvalue, "{s:o}", "DetectorLimits", &jdetlm);
        json_unpack(batchvalue, "{s:o}", "DetectorDistance", &jdx);
        json_unpack(batchvalue, "{s:o}", "Vector1", &je1);
        json_unpack(batchvalue, "{s:o}", "Vector2", &je2);
        json_unpack(batchvalue, "{s:o}", "Vector3", &je3);
        json_unpack(batchvalue, "{s:o}", "AxesLabels", &jgonlab);
        json_unpack(batchvalue, "{s:o}", "CellRefinementFlags", &jlbcell);
        json_unpack(batchvalue, "{s:o}", "MissettingAngles", &jphixyz);
        json_unpack(batchvalue, "{s:o}", "RotationAxis", &jscanax);
        json_unpack(batchvalue, "{s:o}", "SourceVector", &jso);
        json_unpack(batchvalue, "{s:o}", "IdealisedSourceVector", &jsource);
        json_unpack(batchvalue, "{s:o}", "Theta", &jtheta);

        jtitle &&json_is_string(jtitle) ? snprintf(currentBatch->title, 71, "%s", json_string_value(jtitle)) : 0;
        jalambd &&json_is_real(jalambd) ? currentBatch->alambd = json_real_value(jalambd) : 0;
        jbbfac &&json_is_real(jbbfac) ? currentBatch->bbfac = json_real_value(jbbfac) : 0;
        jbscale &&json_is_real(jbscale) ? currentBatch->bscale = json_real_value(jbscale) : 0;
        jdelamb &&json_is_real(jdelamb) ? currentBatch->delamb = json_real_value(jdelamb) : 0;
        jdelcor &&json_is_real(jdelcor) ? currentBatch->delcor = json_real_value(jdelcor) : 0;
        jdivhd &&json_is_real(jdivhd) ? currentBatch->divhd = json_real_value(jdivhd) : 0;
        jdivvd &&json_is_real(jdivvd) ? currentBatch->divvd = json_real_value(jdivvd) : 0;
        jiortyp &&json_is_real(jiortyp) ? currentBatch->iortyp = json_integer_value(jiortyp) : 0;
        jjsaxs &&json_is_integer(jjsaxs) ? currentBatch->jsaxs = json_integer_value(jjsaxs) : 0;
        jjumpax &&json_is_integer(jjumpax) ? currentBatch->jumpax = json_integer_value(jjumpax) : 0;
        jlbmflg &&json_is_integer(jlbmflg) ? currentBatch->lbmflg = json_integer_value(jlbmflg) : 0;
        jlcrflg &&json_is_integer(jlcrflg) ? currentBatch->lcrflg = json_integer_value(jlcrflg) : 0;
        jldtype &&json_is_integer(jldtype) ? currentBatch->ldtype = json_integer_value(jldtype) : 0;
        jmisflg &&json_is_integer(jmisflg) ? currentBatch->misflg = json_integer_value(jmisflg) : 0;
        jnbscal &&json_is_integer(jnbscal) ? currentBatch->nbscal = json_integer_value(jnbscal) : 0;
        jnbsetid &&json_is_integer(jnbsetid) ? currentBatch->nbsetid = json_integer_value(jnbsetid) : 0;
        jncryst &&json_is_integer(jncryst) ? currentBatch->ncryst = json_integer_value(jncryst) : 0;
        jndet &&json_is_integer(jndet) ? currentBatch->ndet = json_integer_value(jndet) : 0;
        jngonax &&json_is_integer(jngonax) ? currentBatch->ngonax = json_integer_value(jngonax) : 0;
        jnum &&json_is_integer(jnum) ? currentBatch->num = json_integer_value(jnum) : 0;
        jphiend &&json_is_real(jphiend) ? currentBatch->phiend = json_real_value(jphiend) : 0;
        jphirange &&json_is_real(jphirange) ? currentBatch->phirange = json_real_value(jphirange) : 0;
        jphistt &&json_is_real(jphistt) ? currentBatch->phistt = json_real_value(jphistt) : 0;
        jtime1 &&json_is_real(jtime1) ? currentBatch->time1 = json_real_value(jtime1) : 0;
        jtime2 &&json_is_real(jtime2) ? currentBatch->time2 = json_real_value(jtime2) : 0;
        jsdbfac &&json_is_real(jsdbfac) ? currentBatch->sdbfac = json_real_value(jsdbfac) : 0;
        jsdbscale &&json_is_real(jsdbscale) ? currentBatch->sdbscale = json_real_value(jsdbscale) : 0;

        if (jcell && json_array_check_dimensions(jcell, celldimensions, 1) && json_array_is_homogenous_real(jcell))
        {
            for (int i = 0; i < 6; i++)
            {
                currentBatch->cell[i] = json_real_value(json_array_get(jcell, i));
            }
        }

        if (jcrydat && json_array_size(jcrydat) == 12 && json_array_is_homogenous_real(jcrydat))
        {
            for (int i = 0; i < 12; i++)
            {
                currentBatch->crydat[i] = json_real_value(json_array_get(jcrydat, i));
            }
        }

        if (jdatum && json_array_size(jdatum) == 3 && json_array_is_homogenous_real(jdatum))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->datum[i] = json_real_value(json_array_get(jdatum, i));
            }
        }

        if (jdetlm && json_array_check_dimensions_f(jdetlm, detlmdimensions, 3, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    for (int k = 0; k < 2; k++)
                    {
                        currentBatch->detlm[i][j][k] = json_real_value(json_array_get(json_array_get(json_array_get(jdetlm, i), j), k));
                    }
                }
            }
        }

        if (jdx && json_array_check_dimensions_f(jdx, jdxdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 2; i++)
            {
                currentBatch->dx[i] = json_real_value(json_array_get(jdx, i));
            }
        }

        if (je1 && json_array_check_dimensions_f(je1, vecdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->e1[i] = json_real_value(json_array_get(je1, i));
            }
        }

        if (je2 && json_array_check_dimensions_f(je2, vecdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->e2[i] = json_real_value(json_array_get(je2, i));
            }
        }

        if (je3 && json_array_check_dimensions_f(je3, vecdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->e3[i] = json_real_value(json_array_get(je3, i));
            }
        }

        for (int i = 0; i < 3; i++)
        {
            snprintf(currentBatch->gonlab[i], 9, "%s", json_string_value(json_array_get(jgonlab, i)));
        }

        if (jlbcell && json_array_check_dimensions_f(jlbcell, celldimensions, 1, &json_array_is_homogenous_integer))
        {
            for (int i = 0; i < 6; i++)
            {
                currentBatch->lbcell[i] = json_integer_value(json_array_get(jlbcell, i));
            }
        }

        if (jphixyz && json_array_check_dimensions_f(jphixyz, phixyzdimensions, 2, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    currentBatch->phixyz[i][j] = json_real_value(json_array_get(json_array_get(jphixyz, i), j));
                }
            }
        }

        if (jscanax && json_array_check_dimensions_f(jscanax, scanaxdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->scanax[i] = json_real_value(json_array_get(jscanax, i));
            }
        }

        if (jso && json_array_check_dimensions_f(jso, sourcedimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->so[i] = json_real_value(json_array_get(jso, i));
            }
        }

        if (jsource && json_array_check_dimensions_f(jsource, sourcedimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 3; i++)
            {
                currentBatch->source[i] = json_real_value(json_array_get(jsource, i));
            }
        }

        if (jtheta && json_array_check_dimensions_f(jtheta, thetadimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 2; i++)
            {
                currentBatch->theta[i] = json_real_value(json_array_get(jtheta, i));
            }
        }

        if (jumat && json_array_check_dimensions_f(jumat, umatdimensions, 1, &json_array_is_homogenous_real))
        {
            for (int i = 0; i < 9; i++)
            {
                currentBatch->umat[i] = json_real_value(json_array_get(jumat, i));
            }
        }

        if (batchindex < json_array_size(jbatches) - 1)
        {
            currentBatch->next = MtzMallocBatch();
            currentBatch = currentBatch->next;
        }
    }

    return mtzout;
}

/**
 * Transfer set information from a json object to an MTZSET dataset struct.
 * @param[in] set The MTZSET struct.
 * @param[in] jsets The json object.
 * @param[in] mtzout The parent MTZ struct.
 * @return The MTZSET struct.
 */

MTZSET *setMtzSet(MTZSET *set, json_t *jset, MTZ *mtzout)
{
    json_t *jdname = NULL;
    json_t *jwavelength = NULL;
    json_t *jsetid = NULL;
    json_t *jcols = NULL;
    int colindex;
    json_t *colvalue = NULL;

    // Metadata
    json_unpack(jset, "{s:o}", "DatasetName", &jdname);
    json_unpack(jset, "{s:o}", "Wavelength", &jwavelength);
    json_unpack(jset, "{s:o}", "DatasetID", &jsetid);
    json_unpack(jset, "{s:o}", "Columns", &jcols);

    jdname &&json_is_string(jdname) ? snprintf(set->dname, 65, "%s", json_string_value(jdname)) : 0;
    jwavelength &&json_is_real(jwavelength) ? set->wavelength = json_real_value(jwavelength) : 0;
    jsetid &&json_is_integer(jsetid) ? set->setid = json_integer_value(jsetid) : 0;
    jcols &&json_is_array(jcols) ? set->ncol = json_array_size(jcols) : 0;

    // Columns
    if (jcols && json_is_array(jcols) && json_array_is_homogenous_object(jcols))
    {
        json_array_foreach(jcols, colindex, colvalue)
        {
            MTZCOL *mtzcol = NULL;
            json_t *jcolsource = NULL;
            json_t *jgrpname = NULL;
            json_t *jlabel = NULL;
            json_t *jtype = NULL;
            json_t *jsource = NULL;
            json_t *jgrpposn = NULL;
            json_t *jmin = NULL;
            json_t *jmax = NULL;
            json_t *jgrptype = NULL;
            json_t *jref = NULL;
            int dataindex;
            json_t *datavalue = NULL;

            mtzcol = MtzMallocCol(mtzout, mtzout->nref);

            if (!mtzcol)
            {
                return NULL;
            }

            json_unpack(colvalue, "{s:o}", "ColumnSource", &jcolsource);
            json_unpack(colvalue, "{s:o}", "GroupName", &jgrpname);
            json_unpack(colvalue, "{s:o}", "Label", &jlabel);
            json_unpack(colvalue, "{s:o}", "Type", &jtype);
            json_unpack(colvalue, "{s:o}", "ColumnID", &jsource);
            json_unpack(colvalue, "{s:o}", "GroupPosition", &jgrpposn);
            json_unpack(colvalue, "{s:o}", "MinValue", &jmin);
            json_unpack(colvalue, "{s:o}", "MaxValue", &jmax);
            json_unpack(colvalue, "{s:o}", "GroupType", &jgrptype);
            json_unpack(colvalue, "{s:o}", "Data", &jref);

            mtzcol->active = 1;
            jcolsource &&json_is_string(jcolsource) ? snprintf(mtzcol->colsource, 37, "%s", json_string_value(jcolsource)) : 0;
            jgrpname &&json_is_string(jgrpname) ? snprintf(mtzcol->grpname, 31, "%s", json_string_value(jgrpname)) : 0;
            jlabel &&json_is_string(jlabel) ? snprintf(mtzcol->label, 31, "%s", json_string_value(jlabel)) : 0;
            jtype &&json_is_string(jtype) ? snprintf(mtzcol->type, 3, "%s", json_string_value(jtype)) : 0;
            jsource &&json_is_integer(jsource) ? mtzcol->source = json_integer_value(jsource) : 0;
            jgrpposn &&json_is_integer(jgrpposn) ? mtzcol->grpposn = json_integer_value(jgrpposn) : 0;
            jmin &&json_is_real(jmin) ? mtzcol->min = json_real_value(jmin) : 0;
            jmax &&json_is_real(jmax) ? mtzcol->max = json_real_value(jmax) : 0;
            jgrptype &&json_is_string(jgrptype) ? snprintf(mtzcol->grptype, 5, "%s", json_string_value(jgrptype)) : 0;
            if (jref && json_is_array(jref))
            {
                json_array_foreach(jref, dataindex, datavalue)
                {
                    if (json_typeof(datavalue) == JSON_REAL)
                    {
                        mtzcol->ref[dataindex] = json_real_value(datavalue);
                    }
                }
            }

            set->col[colindex] = mtzcol;
        }
    }
    return set;
}

/**
 * Transfer crystal information from a json object to an MTZ struct.
 * @param[in] mtzout The MTZ struct.
 * @param[in] jcrystals The crystals json array.
 * @return The MTZ struct.
 */

MTZ *setMtzXtals(MTZ *mtzout, const json_t *jcrystals)
{
    json_t *crystalvalue = NULL;
    int crystalindex;

    json_array_foreach(jcrystals, crystalindex, crystalvalue)
    {
        json_t *jcell = NULL;
        int cellindex;
        json_t *cellvalue = NULL;
        json_t *jresmin = NULL;
        json_t *jresmax = NULL;
        json_t *jxname = NULL;
        json_t *jpname = NULL;
        json_t *jxtalid = NULL;
        json_t *jsets = NULL;
        json_t *setvalue = NULL;
        int setindex;

        json_unpack(crystalvalue, "{s:o}", "CellConstants", &jcell);
        json_unpack(crystalvalue, "{s:o}", "ResolutionMin", &jresmin);
        json_unpack(crystalvalue, "{s:o}", "ResolutionMax", &jresmax);
        json_unpack(crystalvalue, "{s:o}", "CrystalName", &jxname);
        json_unpack(crystalvalue, "{s:o}", "ProjectName", &jpname);
        json_unpack(crystalvalue, "{s:o}", "CrystalID", &jxtalid);
        json_unpack(crystalvalue, "{s:o}", "Datasets", &jsets);

        if (jcell && json_is_array(jcell) && json_array_is_homogenous_real(jcell))
        {
            json_array_foreach(jcell, cellindex, cellvalue)
            {
                mtzout->xtal[crystalindex]->cell[cellindex] = json_real_value(cellvalue);
            }
        }

        jresmin &&json_is_real(jresmin) ? mtzout->xtal[crystalindex]->resmin = json_real_value(jresmin) : 0;
        jresmax &&json_is_real(jresmax) ? mtzout->xtal[crystalindex]->resmax = json_real_value(jresmax) : 0;
        jxname &&json_is_string(jxname) ? snprintf(mtzout->xtal[crystalindex]->xname, 65, "%s", json_string_value(jxname)) : 0;
        jpname &&json_is_string(jpname) ? snprintf(mtzout->xtal[crystalindex]->pname, 65, "%s", json_string_value(jpname)) : 0;
        jxtalid &&json_is_integer(jxtalid) ? mtzout->xtal[crystalindex]->xtalid = json_integer_value(jxtalid) : 0;
        if (jsets && json_is_array(jsets) && json_array_is_homogenous_object(jsets))
        {
            json_array_foreach(jsets, setindex, setvalue)
            {
                setMtzSet(mtzout->xtal[crystalindex]->set[setindex], setvalue, mtzout);
            }
        }
    }
    return mtzout;
}

/**
 * Transfer symmetry information from a json object to an MTZ struct.
 * @param[in] mtzout The MTZ struct.
 * @param[in] jcrystals The symmetry json object.
 * @return The MTZ struct.
 */

MTZ *setMtzSymmetry(MTZ *mtzout, json_t *jsymm)
{
    json_t *jspcgrp = NULL;
    json_t *jspcgrpname = NULL;
    json_t *jspg_confidence = NULL;
    json_t *jnsym = NULL;
    json_t *jnsymp = NULL;
    json_t *jpgname = NULL;
    json_t *jsym = NULL;
    json_t *ival = NULL;
    json_t *jval = NULL;
    json_t *kval = NULL;
    json_t *jsymtyp = NULL;
    int i, j, k;
    int symmetryDimensions[] = {192, 4, 4};
    int (*real_check)(json_t *);

    json_unpack(jsymm, "{s:o}", "SpaceGroupNumber", &jspcgrp);
    json_unpack(jsymm, "{s:o}", "SpaceGroupName", &jspcgrpname);
    json_unpack(jsymm, "{s:o}", "PointGroupName", &jpgname);
    json_unpack(jsymm, "{s:o}", "SpaceGroupConfidence", &jspg_confidence);
    json_unpack(jsymm, "{s:o}", "NumberOfSymmetryOperations", &jnsym);
    json_unpack(jsymm, "{s:o}", "NumberOfPrimitiveSymmetryOperations", &jnsymp);
    json_unpack(jsymm, "{s:o}", "SymmetryOperations", &jsym);
    json_unpack(jsymm, "{s:o}", "LatticeType", &jsymtyp);

    jspcgrp &&json_is_integer(jspcgrp) ? mtzout->mtzsymm.spcgrp = json_integer_value(jspcgrp) : 0;
    jspcgrpname &&json_is_string(jspcgrpname) ? strncpy(mtzout->mtzsymm.spcgrpname, json_string_value(jspcgrpname), 21) : 0;
    jpgname &&json_is_string(jpgname) ? strncpy(mtzout->mtzsymm.pgname, json_string_value(jpgname), 11) : 0;
    jspg_confidence &&json_is_string(jspg_confidence) ? mtzout->mtzsymm.spg_confidence = json_string_value(jspg_confidence)[0] : 0;
    jnsym &&json_is_integer(jnsym) ? mtzout->mtzsymm.nsym = json_integer_value(jnsym) : 0;
    jnsymp &&json_is_integer(jnsymp) ? mtzout->mtzsymm.nsymp = json_integer_value(jnsymp) : 0;

    if (jsym && json_is_array(jsym) && json_array_check_dimensions_f(jsym, symmetryDimensions, 3, &json_array_is_homogenous_real))
    {
        json_array_foreach(jsym, i, ival)
        {
            json_array_foreach(ival, j, jval)
            {
                json_array_foreach(jval, k, kval)
                {
                    mtzout->mtzsymm.sym[i][j][k] = json_real_value(kval);
                }
            }
        }
    }

    jsymtyp &&json_is_string(jsymtyp) ? mtzout->mtzsymm.symtyp = json_string_value(jsymtyp)[0] : 0;

    return mtzout;
}

/**
 * Find the first MTZCOL struct with a given source ID.
 * @param[in] mtzin The parent MTZ struct.
 * @param[in] source The source ID.
 * @return The MTZCOL struct.
 */

MTZCOL *findColumnBySource(const MTZ *mtz, int source)
{
    for (int i = 0; i < mtz->nxtal; i++)
    {
        for (int j = 0; j < mtz->xtal[i]->nset; j++)
        {
            for (int k = 0; k < mtz->xtal[i]->set[j]->ncol; k++)
            {
                if (mtz->xtal[i]->set[j]->col[k]->source == source)
                {
                    return mtz->xtal[i]->set[j]->col[k];
                }
            }
        }
    }
    return NULL;
};

/**
 * Converts a json reflection object into a MTZ struct and returns a pointer to that struct.
 * @param[in] json The json object.
 * @return The MTZ struct.
 */

MTZ *makeMtz(json_t *json)
{
    MTZ *mtzout = NULL;
    json_t *jtitle = NULL;
    json_t *jcrystals = NULL;
    json_t *jhistory = json_array();
    json_t *jsymm = NULL;
    json_t *jbatches = NULL;
    json_t *jsort = NULL;
    json_t *junknown_headers = json_array();
    int ncryst = 0;
    int *nsets = NULL;
    int **ncols = NULL;
    int ***nrefl = NULL;
    int crystalindex;
    json_t *crystalvalue = NULL;
    int nref = 0;
    int orderindex;
    json_t *ordervalue = NULL;
    int structure_error = 0;

    json_unpack(json,
                "{s:o, s:o, s:o, s:o, s:o, s:o, s?o}",
                "Title", &jtitle,                     // json_string
                "Crystals", &jcrystals,               // json_array
                "History", &jhistory,                 // json_array
                "Symmetry", &jsymm,                   // json_object
                "Batches", &jbatches,                 // json_array
                "SortOrder", &jsort,                  //json_array
                "UnknownHeaders", &junknown_headers); // json_array

    if (jcrystals && json_is_array(jcrystals) && json_array_size(jcrystals) != 0 && json_array_is_homogenous_object(jcrystals))
    {
        ;
    }
    else
    {
        structure_error = 1;
    }

    if (!structure_error)
    {
        ncryst = json_array_size(jcrystals);
        nsets = malloc(ncryst * sizeof(int));
        ncols = malloc(ncryst * sizeof(int *));
        nrefl = malloc(ncryst * sizeof(int *));

        // Count datasets for each crystal
        json_array_foreach(jcrystals, crystalindex, crystalvalue)
        {
            json_t *jsets = NULL;
            int setindex;
            json_t *setvalue = NULL;
            int *ncol = NULL;
            int **nreflections = NULL;
            int columnLength = -1;

            json_unpack(crystalvalue, "{s:o}", "Datasets", &jsets);

            if (jsets && json_is_array(jsets) && json_array_is_homogenous_object(jsets))
            {
                ;
            }
            else
            {
                structure_error = 1;
            }

            if (!structure_error)
            {
                nsets[crystalindex] = json_array_size(jsets);

                // Count columns for each dataset
                ncol = malloc(json_array_size(jsets) * sizeof(int));
                nreflections = malloc(json_array_size(jsets) * sizeof(int *));

                json_array_foreach(jsets, setindex, setvalue)
                {
                    json_t *jcols = NULL;
                    int colindex;
                    json_t *colvalue = NULL;
                    int *refl = NULL;

                    json_unpack(setvalue, "{s:o}", "Columns", &jcols);

                    if (jcols && json_is_array(jcols) && json_array_size(jcols) != 0 && json_array_is_homogenous_object(jcols))
                    {
                        ;
                    }
                    else
                    {
                        structure_error = 1;
                    }

                    if (!structure_error)
                    {
                        refl = malloc(json_array_size(jcols) * sizeof(int));

                        ncol[setindex] = json_array_size(jcols);

                        // Count reflections for each column
                        json_array_foreach(jcols, colindex, colvalue)
                        {
                            json_t *dat = NULL;

                            json_unpack(colvalue, "{s:o}", "Data", &dat);
                            if (dat && json_is_array(dat))
                            {
                                ;
                            }
                            else
                            {
                                structure_error = 1;
                            }

                            if (!structure_error)
                            {
                                refl[colindex] = json_array_size(dat);
                            }
                        }

                        nreflections[setindex] = refl;
                    }

                    ncols[crystalindex] = ncol;
                    nrefl[crystalindex] = nreflections;
                }
            }
        }

        if (!structure_error)
        {
            // Check if columns all have the same length
            nref = -1;
            for (int i = 0; i < ncryst; i++)
            {
                for (int j = 0; j < nsets[i]; j++)
                {
                    for (int k = 0; k < ncols[i][j]; k++)
                    {
                        if (nref < 0)
                        {
                            nref = nrefl[i][j][k];
                        }
                        else if (nref != nrefl[i][j][k]) // Inhomogenous column lengths
                        {
                            free(nsets);
                            free(ncols);
                            free(nrefl);
                            return NULL;
                        }
                    }
                }
            }

            mtzout = MtzMalloc(ncryst, nsets);
            if (!mtzout)
            {
                free(nsets);
                free(ncols);
                free(nrefl);
                return NULL;
            }

            // Set misc properties. Important to set these first.
            mtzout->nref = nrefl[0][0][0];
            mtzout->nxtal = ncryst;
            mtzout->resmax_out = 0;
            mtzout->resmin_out = 999;
            mtzout->refs_in_memory = 1;

            // Set title
            jtitle &&json_is_string(jtitle) ? snprintf(mtzout->title, 71, "%s", json_string_value(jtitle)) : 0;

            // Set history
            if (jhistory && json_is_array(jhistory) && json_array_is_homogenous_string(jhistory))
            {
                mtzout->histlines = json_array_size(jhistory);
                mtzout->hist = MtzCallocHist(mtzout->histlines);

                for (int i = 0; i < mtzout->histlines; i++)
                {
                    strncpy(mtzout->hist + i * MTZRECORDLENGTH, json_string_value(json_array_get(jhistory, i)), MTZRECORDLENGTH);
                }
            }

            // Set symmetry
            jsymm &&json_is_object(jsymm) ? mtzout = setMtzSymmetry(mtzout, jsymm) : 0;

            // Set batches
            jbatches &&json_is_array(jbatches) && json_array_is_homogenous_object(jbatches) ? mtzout = setMtzBatches(mtzout, jbatches) : 0;

            // Set crystals
            jcrystals &&json_is_array(jcrystals) && json_array_is_homogenous_object(jcrystals) ? mtzout = setMtzXtals(mtzout, jcrystals) : 0;

            // Set sort order
            if (jsort && json_is_array(jsort) && json_array_is_homogenous_integer(jsort))
            {
                json_array_foreach(jsort, orderindex, ordervalue)
                {
                    mtzout->order[orderindex] = findColumnBySource(mtzout, json_integer_value(ordervalue));
                }
            }

            // Set unknown headers
            if (junknown_headers && json_is_array(junknown_headers) && json_array_is_homogenous_string(junknown_headers))
            {
                mtzout->n_unknown_headers = json_array_size(junknown_headers);
                mtzout->unknown_headers = malloc(mtzout->n_unknown_headers * MTZRECORDLENGTH * sizeof(char));
                for (int i = 0; i < mtzout->n_unknown_headers; i++)
                {
                    strncpy(mtzout->unknown_headers + i * MTZRECORDLENGTH, json_string_value(json_array_get(junknown_headers, i)), MTZRECORDLENGTH);
                }
            }
        }

        free(nsets);
        free(ncols);
        free(nrefl);
    }

    return mtzout;
}

/**
 * Make a timestamp.
 * @param[in] jobstring Job description. Maximum of 80 chars.
 * @param[in] timestring Time string as returned by ctime().
 * @param[in] timestamp Pointer to the timestamp array. Must be able to hold 80 chars.
 * @return The timestamp.
 */

char *makeTimestamp(const char *jobstring, const char *timestring, char *timestamp)
{
    int timestring_index = 56;

    timestring_index - 1 > strlen(jobstring) ? timestring_index = strlen(jobstring) + 1 : 0;

    strncpy(timestamp, jobstring, timestring_index - 1);
    timestamp[timestring_index - 1] = ' ';
    strncpy(timestamp + timestring_index, timestring, 24);
    memset(timestamp + timestring_index + 24, ' ', MTZRECORDLENGTH - 24 - 1 - strlen(jobstring));

    return timestamp;
}