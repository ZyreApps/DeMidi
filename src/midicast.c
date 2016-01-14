/*  =========================================================================
    midicast - Take MIDI events from port and broadcast on Zyre MIDI group

    Copyright (c) the Contributors as noted in the AUTHORS file.

    This file is part of DeMidi, a distributed MIDI project.
    https://github.com/zyreapps/demidi

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#include <zyre.h>
#include <alsa/asoundlib.h>

int main (int argc, char *argv [])
{
    int argn = 1;
    if (argn == argc
    ||  streq (argv [argn], "-h") || streq (argv [argn], "--help")) {
        puts ("midicast [-v] [-p port] [-i interface]");
        puts ("Reads MIDI events from port and sends to Zyre MIDI group");
        puts (" -h, --help: this help");
        puts (" -v, --verbose: trace events as they happen");
        puts (" -p, --port: specify port name, e.g. '-p hw:1,0,0'");
        puts (" -i, --interface: specify WiFi interface, e.g. '-i wlan0'");
        return 0;
    }
    char *midi_port_name = "hw:1,0,0";
    char *wifi_interface = NULL;
    bool verbose = false;
    while (argn < argc) {
        if (streq (argv [argn], "-p") || streq (argv [argn], "--port"))
            midi_port_name = argv [++argn];
        else
        if (streq (argv [argn], "-i") || streq (argv [argn], "--interface"))
            wifi_interface = argv [++argn];
        else
        if (streq (argv [argn], "-v") || streq (argv [argn], "--verbose"))
            verbose = true;
        argn++;
    }
    snd_rawmidi_t *input;
    int rc = snd_rawmidi_open (&input, NULL, midi_port_name, 0);
    if (rc < 0) {
        zsys_error ("cannot open port \"%s\": %s", midi_port_name, snd_strerror (rc));
        return 0;
    }
    //  Flush any waiting input
    zclock_sleep (100);
    zsys_info ("casting MIDI events from %s", midi_port_name);

    zyre_t *zyre = zyre_new (NULL);
    if (wifi_interface)
        zyre_set_interface (zyre, wifi_interface);
    zyre_start (zyre);
    zyre_join (zyre, "MIDI");
    zsys_info ("this player is %s", zyre_name (zyre));

    while (!zsys_interrupted) {
        byte buffer [256];
        int rc = snd_rawmidi_read (input, buffer, sizeof (buffer));
        if (rc == -EAGAIN)
            continue;
        else
        if (rc == -EINTR) {
            printf (" interrupted\n");
            break;
        }
        if (rc < 0) {
            zsys_error ("cannot read MIDI data: %s", snd_strerror (rc));
            break;
        }
        //  Broadcast the MIDI event
        zmsg_t *msg = zmsg_new ();
        zmsg_addmem (msg, buffer, rc);
        zyre_shout (zyre, "MIDI", &msg);

        if (verbose) {
            printf ("%d:", rc);
            int byte_nbr;
            for (byte_nbr = 0; byte_nbr < rc; byte_nbr++)
                printf (" %02X", buffer [byte_nbr]);
            printf ("\n");
        }
    }
    snd_rawmidi_close (input);
    zyre_destroy (&zyre);
	return 0;
}
