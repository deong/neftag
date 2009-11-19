/*
 * nikond90.h
 * useful constants for dealing with D90 raw files
 */

#ifndef _NIKOND90_H_
#define _NIKOND90_H_

/* tags for 0th ifd */
#define NewSubFileType 0x00fe
#define ImageWidth 0x0100
#define ImageLength 0x0101
#define BitsPerSample 0x0102
#define Compression 0x0103
#define PhotometricInterpretation 0x0106
#define Make 0x010f
#define Model 0x0110
#define StripOffsets 0x0111
#define Orientation 0x0112
#define SamplesPerPixel 0x0115
#define RowsPerStrip 0x0116
#define StripByteCounts 0x0117
#define XResolution 0x011a
#define YResolution 0x011b
#define PlanarConfiguration 0x011c
#define ResolutionUnit 0x0128
#define Software 0x0131
#define DateTime 0x0132
#define SubIFDs 0x014a
#define ReferenceBlackWhite 0x0214
#define ExifIFDPointer 0x8769
#define GPSInfoIFDPointer 0x8825
#define DateTimeOriginal 0x9003
#define TIFFEPSStandard 0x9216


/* tags for ExifIFD */
#define ExposureTime 0x829a
#define Fnumber 0x829d
#define ExposureProgram 0x8822
#define ISOSpeedRatings 0x8827
#define DateTimeOriginal 0x9003 
#define DateTimeDigitized 0x9004
#define ExposureBiasValue 0x9204
#define MaxApertureValue 0x9205
#define MeteringMode 0x9207
#define LightSource 0x9208
#define Flash 0x9209
#define FocalLength 0x920a
#define MakerNote 0x927c
#define UserComment 0x9286
#define SubSecTime 0x9290       /* subsecond timing for DateTime in 0th ifd */
#define SubSecTimeOriginal 0x9291 /* subsecond timing for DateTimeOriginal */
#define SubSecTimeDigitized 0x9292 /* subsecond timing for DateTimeDigitized */
#define SensingMethod 0xa217
#define FileSource 0xa300
#define SceneType 0xa301
#define CFAPattern 0xa302
#define CustomRendered 0xa401
#define ExposureMode 0xa402
#define WhiteBalance 0xa403
#define DigitalZoomRatio 0xa404
#define FocalLengthIn35mmFilm 0xa405
#define SceneCaptureType 0xa406
#define GainControl 0xa407
#define Contrast 0xa408
#define Saturation 0xa409
#define Sharpness 0xa40a
#define SubjectDistanceRange 0xa40c


/* tags for the GPSInfoIFD */
#define GPSVersionID 0x0000
#define GPSLatitudeRef 0x0001 /* north or south latitude */
#define GPSLatitude 0x0002
#define GPSLongitudeRef 0x0003 /* east or west latitude */
#define GPSLongitude 0x0004
#define GPSAltitudeRef 0x0005 /* unit of altitude */
#define GPSAltitude 0x0006
#define GPSTimeStamp 0x0007
#define GPSSatellites 0x0008
#define GPSImgDirectionRef 0x0010
#define GPSImgDirection 0x0011
#define GPSMapDatum 0x0012
#define GPSDateStamp 0x001d


/* tags for SubIFD1 */
#define SubIFD1NewSubFileType 0x00fe
#define SubIFD1Compression 0x0103
#define SubIFD1XResolution 0x011a
#define SubIFD1YResolution 0x011b
#define SubIFD1ResolutionUnit 0x0128
#define SubIFD1JPEGInterchangeFormat 0x0201
#define SubIFD1JPEGInterchangeFormatLength 0x0202
#define SubIFD1YCbCrPositioning 0x0213


/* tags for SubIFD2 */
#define SubIFD2NewSubFileType 0x00fe
#define SubIFD2ImageWidth 0x0100
#define SubIFD2ImageLength 0x0101
#define SubIFD2BitsPerSample 0x0102
#define SubIFD2Compression 0x0103
#define SubIFD2PhotometricInterpretation 0x0106
#define SubIFD2StripOffsets 0x0111
#define SubIFD2SamplesPerPixel 0x0115
#define SubIFD2RowsPerStrip 0x0116
#define SubIFD2StripByteCounts 0x0117
#define SubIFD2XResolution 0x011a
#define SubIFD2YResolution 0x011b
#define SubIFD2PlanarConfiguration 0x011c
#define SubIFD2ResolutionUnit 0x0128
#define SubIFD2CFARepeatPattern 0x828d
#define SubIFD2CFAPattern 0x828e
#define SubIFD2SensingMethod 0x9217


/* PhotometricInterpretation values */
#define WhiteIsZero 0x0000
#define BlackIsZero 0x0001
#define RGB 0x0002
#define RGBPalette 0x0003
#define TransparencyMask 0x0004
#define CMYK 0x0005
#define YCbCr 0x0006
#define CIELab 0x0008
#define CFA 0x8023
/* > 8000, Vendor Unique */

#endif
