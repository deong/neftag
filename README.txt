neftag: GPS tagging for Nikon RAW images
------------------------------------------------------------------------

Simple C program that takes a log file produced by a GPS logging
device (in NMEA format), one or more Nikon raw (NEF) format images,
and matches the timestamps from the GPS log and the image and uses them
to record the GPS coordinates for that instant in time into the EXIF
headers of the images.
