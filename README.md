# DeMidi

## Goals

This project aims to build decentralized MIDI over Zyre. The goal is that you can bring MIDI devices, sequencers, and synths together over WiFi without any cables and without any configuration.

## Policies

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

This project uses the [C4.1 (Collective Code Construction Contract)](http://rfc.zeromq.org/spec:22) process for contributions.

This project uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

## Usage

Build using either of:

```
./autogen.sh && configure && make
cmake . && make
```

Currently provides two tools:

* midicast - broadcasts MIDI events from a local MIDI device to the Zyre ether
* midigrab - grabs MIDI events off the Zyre ether and forwards them to a local MIDI port

Run these as follows:

```
midicast hw:1,0,0
```

and

```
midigrab hw:2,0
```

## Sound Configuration on Linux

[Here is a good tutorial](http://askubuntu.com/questions/19764/how-does-one-set-up-a-midi-keyboard) on setting up MIDI synthesis on Linux.

On the receiving box you will want to use a virtual MIDI device. Load the kernel module thus:

```
sudo modprobe snd-virmidi snd_index=1
```

To check the MIDI devices you have, use these commands to taste:

```
amidi -l
cat /proc/asound/cards
cat /proc/asound/devices
```
