/* 
 * json2mtz.c: JSON to MTZ converter
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

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "jsonmtz.h"

int main(int argc, char *argv[])
{
    int ret;
    int o;
    options_json2mtz opts;
    opterr = 0;

    opts.version = 0;
    opts.help = 0;
    opts.timestamp = 1;
    opts.force = 0;

    while (TRUE)
    {
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"version", no_argument, 0, 'v'},
            {"no-timestamp", no_argument, 0, 'n'},
            {"force", no_argument, 0, 'f'},
            {0, 0, 0, 0}};

        int option_index = 0;

        o = getopt_long(argc, argv, "hvnf", long_options, &option_index);

        if (o == -1)
        {
            break;
        }

        switch (o)
        {
        case 'h':
            opts.help = 1;
            break;
        case 'v':
            opts.version = 1;
            break;
        case 'n':
            opts.timestamp = 0;
            break;
        case 'f':
            opts.force = 1;
        case '?':
            fprintf(stderr, "%s", "\njson2mtz --help\n\n");
            return 1;
        }
    }

    if (opts.help)
    {
        puts("");
        puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        puts("~~ JSON to MTZ converter ~~");
        puts("");
        puts("Usage:");
        puts("    json2mtz [options] in.json out.mtz");
        puts("");
        puts("Options:");
        puts("    -v --version          Print program version.");
        puts("    -n --no-timestamp     Do not add timestamp to history.");
        puts("    -h --help             Print help.");
        puts("    -f --force            Input and output filenames can be the same.");
        puts("");
        exit(0);
    }

    if (opts.version)
    {
        printf("\njson2mtz v%d.%d.%d\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        exit(0);
    }

    if (argc - optind != 2)
    {
        fprintf(stderr, "%s", "\njson2mtz --help\n\n");
        return 1;
    }

    if (strcmp(argv[optind], argv[optind + 1]) != 0 || opts.force)
    {
        ret = json2mtz(argv[optind], argv[optind + 1], &opts);
    }
    else
    {
        fprintf(stderr, "%s", "Input and output filenames must be different.\n");
        return 1;
    }

    switch (ret)
    {
    case 0:
        puts(argv[optind + 1]);
        return 0;
    case 1:
        fprintf(stderr, "%s", "Unable to read JSON file.");
        return 1;
    case 2:
        fprintf(stderr, "%s", "Unable to convert to MTZ file / write MTZ file.");
        return 1;
    }
}