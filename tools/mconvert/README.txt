mconvert 2017 Lee Weber released under GPL 3

This tool converts the standard midi output file from midiconv, into a compressed version.
The compression will vary depending on the input music data, but it should always be smaller than
the original. The main aim of this program is to increase the efficiency of the Uzebox streaming
music system, in that there is less data that must be buffered therefore requiring less buffer
and less cycles to fill that buffer(ie. SD access).

There are a number of command lines supported which mean to aid developers easily integrate the
data to their project.

	-p This will output all the song data in hex(0xXX) format, comma seperated. No statistics
	will be displayed afterward. There may be no use for this, but someone might desire to
	pipe the data to other tools they use without a file intermediary.

	-b This changed the output to binary mode, where values encoded as 1 byte are written.
	The output can be put into an existing file, at any offset. This is primarily intended
	to make developing resource files to place on the SD card, to later stream music from.
	Only raw data is output, no C array details, commas, etc.

	-a This command makes the output data always be a multiple of 512(sector size) bytes.
	It may be useful for those building SD resources, to make visibility easier when viewing
	from within a hex editor.

	-o For binary mode, this specifies the offset to put the output data. This option is not
	useful or supported in the default text mode. This allows developers to build files to 
	place on the SD card.

	-n For text mode, specifies the name of the output C array.

	-d This is primarily intended at testing and feature development for the streaming music
	system. This will output a C array(text mode) with binary 0bXXXXXXXX, comma seperated bytes.
	This allows a user to manually read through the data easier and investigate events based on
	bit patterns.

	-f This is a controller event filter. The following byte will indicate a number for which the
	bit values will filter certain events from the output stream. This can have the effect of
	making output files smaller and cleaner, as well as higher performance when played back.




