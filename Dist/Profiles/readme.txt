Profiles
--------


Unfortunately, the weird behaviour of some DVD players require
different muxing patterns than those which are necessary for
for optimal seeking in C/DVD-ROM drives.

To make configuration easier, here are a few files which configure
AVI-Mux GUI to create files that (hopefully!) work on the desired
device.

For Standalone players, I chose to make 2 different profiles, for
AC3 and MP3, to guaranty that the amount of audio data always
exactly matches the amount of video data for PAL videos. For
NTSC, such a setting is almost impossible.

Please *test* those profiles and files on players. If something does
not work, then please tell me.

If you want to go back to a configuration leading to smaller files
that work better on normal players, just use Profile - CDVD AVI.amg
or Profile - low overhead.amg
