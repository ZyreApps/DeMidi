/*  =========================================================================
    midigrab - Take MIDI events from Zyre MIDI group and send to port

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
    char *midi_port_name = "hw:2,0";
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

    snd_rawmidi_t *output;
    int rc = snd_rawmidi_open (NULL, &output, midi_port_name, SND_RAWMIDI_SYNC);
    if (rc < 0) {
        zsys_error ("cannot open port \"%s\": %s", midi_port_name, snd_strerror (rc));
        return 0;
    }
    zsys_info ("forwarding MIDI cast to %s", midi_port_name);

    zyre_t *zyre = zyre_new (NULL);
    if (wifi_interface)
        zyre_set_interface (zyre, wifi_interface);
    zyre_start (zyre);
    zyre_join (zyre, "MIDI");
    zsys_info ("this player is %s", zyre_name (zyre));

    while (!zsys_interrupted) {
        zyre_event_t *event = zyre_event_new (zyre);
        if (!event) {
            printf (" interrupted\n");
            break;
        }
        if (zyre_event_type (event) == ZYRE_EVENT_JOIN)
            zsys_info ("[%s] player joined", zyre_event_name (event));
        else
        if (zyre_event_type (event) == ZYRE_EVENT_LEAVE)
            zsys_info ("[%s] player left", zyre_event_name (event));
        else
        if (zyre_event_type (event) == ZYRE_EVENT_SHOUT) {
            if (streq (zyre_event_group (event), "MIDI")) {
                zframe_t *frame = zmsg_first (zyre_event_msg (event));
                //  Forward the MIDI event
                snd_rawmidi_write (output, zframe_data (frame), zframe_size (frame));

                if (verbose) {
                    printf ("%zd:", zframe_size (frame));
                    int byte_nbr;
                    for (byte_nbr = 0; byte_nbr < zframe_size (frame); byte_nbr++)
                        printf (" %02X", zframe_data (frame) [byte_nbr]);
                    printf ("\n");
                }
            }
        }
        zyre_event_destroy (&event);
    }
    snd_rawmidi_close (output);
    zyre_destroy (&zyre);
	return 0;
}
