/*  =========================================================================
    midicast - Take MIDI events from port and broadcast on Zyre MIDI group

    Copyright (c) the Contributors as noted in the AUTHORS file.

    This file is part of demidi, a distributed MIDI project.
    https://github.com/zyreapps/demidi

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

//  TODO: set interface via command line
//  TODO: verbose on/off via command line

#include <zyre.h>
#include <alsa/asoundlib.h>

int main (int argc, char *argv [])
{
    int argn = 1;
    char *port_name = "hw:1,0,0";
    if (argn < argc)
        port_name = argv [argn++];

    snd_rawmidi_t *input;
    int rc = snd_rawmidi_open (&input, NULL, port_name, 0);
    if (rc < 0) {
        zsys_error ("cannot open port \"%s\": %s", port_name, snd_strerror (rc));
        return 0;
    }
    //  Flush any waiting input
    zclock_sleep (100);
    zsys_info ("casting MIDI events from %s", port_name);

    zyre_t *zyre = zyre_new (NULL);
    zyre_start (zyre);
    zyre_join (zyre, "MIDI");
    zsys_info ("this player is %s", zyre_name (zyre));

    while (!zsys_interrupted) {
        byte buffer [256];
        int bytes_read = snd_rawmidi_read (input, buffer, sizeof (buffer));
        if (bytes_read == -EAGAIN)
            continue;

        if (bytes_read < 0) {
            zsys_error ("cannot read from port \"%s\": %s", port_name, snd_strerror (bytes_read));
            break;
        }
        //  Broadcast the MIDI event
        zmsg_t *msg = zmsg_new ();
        zmsg_addmem (msg, buffer, bytes_read);
        zyre_shout (zyre, "MIDI", &msg);

        //  Dump the buffer for debugging purposes
        printf ("%d:", bytes_read);
        int byte_nbr;
        for (byte_nbr = 0; byte_nbr < bytes_read; byte_nbr++)
            printf (" %02X", buffer [byte_nbr]);
        printf ("\n");
    }
    snd_rawmidi_close (input);
    zyre_destroy (&zyre);
	return 0;
}
