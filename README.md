# CHK-Extractor

This program restores media files from CHK files created by Windows.

CHK files are created from CHK-DSK when corrupted files are found. For some reason, Windows combines the files into one big file instead of splitting it into each file.
This program does that by parsing each byte of the CHK file, and restores each file when it finds a byte sequence matchiing that of a regular file header, or the beginninng of a file.

Currently, this program only restores .jpeg, .mov, .heic, .mp4, .png, .gif, .pdf, .zip, .mkv, .webp, .p3g, and .wav files.
Note: I've only been able to test .mov, .jpeg, and .heic files.

If you want to add more file types, add them to the filetypes.h file above the FileHeader class. You can get the beginning byte sequence of a file by using a program like NotePad++ and opening a file of the same type in hexadecimal mode.
