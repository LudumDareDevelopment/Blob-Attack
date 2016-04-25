#ifndef BLOBS_BITMAP_H
#define BLOBS_BITMAP_H

byte PROGMEM blob00_bitmap[] =
{
  8,6,
  //
  //
  //
  //
  //
  //
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

byte PROGMEM blob00inverted_bitmap[] =
{
  8,6,
  // ######
  // ######
  // ######
  // ######
  // ######
  // ######
  0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
};


byte PROGMEM blob01_bitmap[] =
{
  8,6,
  //   #
  //  ###
  // #   #
  // ## ##
  //  ###
  //
  0x0C, 0x1A, 0x13, 0x1A, 0x0C, 0x00,
};

byte PROGMEM blob02_bitmap[] =
{
  8,6,
  //  ###
  // # # #
  // #####
  //  # #
  //  ###
  //
  0x06, 0x1D, 0x17, 0x1D, 0x06, 0x00,
};

byte PROGMEM blob03_bitmap[] =
{
  8,6,
  //  # #
  //  ###
  // #####
  // # # #
  //  ###
  //
  0x0C, 0x17, 0x1E, 0x17, 0x0C, 0x00,
};

byte PROGMEM blob04_bitmap[] =
{
  8,6,
  // #####
  //  # #
  //
  // #   #
  // #####
  //
  0x19, 0x13, 0x11, 0x13, 0x19, 0x00,
};

byte PROGMEM blob05_bitmap[] =
{
8,6,
  // #####
  // # # #
  // #####
  // #####
  // #####
  //
  0x1F, 0x1D, 0x1F, 0x1D, 0x1F, 0x00,
};

byte *blobs_bitmap[] =
{
  blob00_bitmap, blob01_bitmap, blob02_bitmap, blob03_bitmap, blob04_bitmap, blob05_bitmap
};

#endif
