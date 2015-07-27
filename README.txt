Devcheck

!!! Be careful, running this program on a block device will overwrite 8 bytes at the start of all sectors !!! 

Background

This program was made to check a block device for read/write errors.

The background for this check is that while I were on vacation in China a few years ago, I bought a cheap 16 GB SD card for my camera. When I arrived back, almost all pictures I had taken was gone. A forensic examination and of the SD card image and carving of JPEG data revealed hundreds of copies of the last few images I had taken.

In order to find out more about this, I made a program like this that first nulled the whole card, then wrote the index number of the current sector in the first QWORD of the sector, after first checking that the QWORD was 0. if it wasn't, it meant that there were several addresses mapping to the same physical sector. At last it would read all the index numbers from the sectors and make sure they were still intact and readable.

The faulty SD card. This was labelled as 16 GB, and the card reported that it was 16 GB, but it was in fact a 4 GB card with about 100 MB ring buffer at the end. So the first 4 GB was intact, and the last 100 MB was intact, but everything in between was just a copy of the last 100 MB.

As I am on vacation again, and bought a new SD card, I rewrote this program just to check that the SD card was in fact a good one. 

This is just a simple test, meant for being easy to pipe the output to other tools for examination.

The output on stdout is a space delimited list containing;
<id> <sector number> <index number>

- <id> is the identification of the check. Only two checks are implemented.
- <sector number> is the sector number of the faulty sector.
- <index number> is the number that is read from the first 8 bytes of the sector <sector number>. In the first test this is supposed to be 0, and in the second test it is supposed to be equal to the sector number. 

If no faulty sectors are found, there are no output to stdout. Information is written to stderr. 

Usage:

# devcheck <block device>
(e.g. # devcheck /dev/sdb)
