/* 
 * mtz2json.c: MTZ to JSON converter
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
    options_mtz2json opts;
    opterr = 0;

    opts.compact = 0;
    opts.version = 0;
    opts.help = 0;
    opts.timestamp = 1;
    opts.force = 0;

    while (TRUE)
    {
        static struct option long_options[] = {
            {"compact", no_argument, 0, 'c'},
            {"help", no_argument, 0, 'h'},
            {"version", no_argument, 0, 'v'},
            {"no-timestamp", no_argument, 0, 'n'},
            {"force", no_argument, 0, 'f'},
            {0, 0, 0, 0}};

        int option_index = 0;

        o = getopt_long(argc, argv, "chvnf", long_options, &option_index);

        if (o == -1)
        {
            break;
        }

        switch (o)
        {
        case 'h':
            opts.help = 1;
            break;
        case 'c':
            opts.compact = 1;
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
            fprintf(stderr, "%s", "mtz2json --help\n");
            return 1;
        }
    }

    if (opts.help)
    {
        puts("");
        puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        puts("~~ MTZ to JSON converter ~~");
        puts("");
        puts("Usage:");
        puts("    mtz2json [options] in.mtz out.json");
        puts("");
        puts("Options:");
        puts("    -c --compact          Write compact JSON file.");
        puts("    -v --version          Print program version.");
        puts("    -n --no-timestamp     Do not add timestamp to history.");
        puts("    -h --help             Print help.");
        puts("    -f --force            Input and output filenames can be the same.");
        puts("");
        exit(0);
    }

    if (opts.version)
    {
        printf("mtz2json v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        exit(0);
    }

    if (argc - optind != 2)
    {
        fprintf(stderr, "%s", "mtz2json --help\n");
        return 1;
    }

    if (strcmp(argv[optind], argv[optind + 1]) != 0 || opts.force)
    {
        ret = mtz2json(argv[optind], argv[optind + 1], &opts);
    }
    else
    {
        fprintf(stderr, "%s", "Input and output filenames must be different.\n");
        return 1;
    }

    if (ret == 0)
    {
        printf("%s\n", argv[optind + 1]);
        return 0;
    }
    if (ret == 2)
    {
        puts("No such file.");
    }
    else
    {
        fprintf(stderr, "%s", "Failed.\n");
        return 1;
    }
}