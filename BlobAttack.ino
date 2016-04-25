#include <SPI.h>
#include <Gamebuino.h>
#include <EEPROM.h>

#include "blob_bitmaps.h"
#include "menu_bitmap.h"

#define PLAYFIELD_WIDTH 8
#define PLAYFIELD_HEIGHT 8

#define GAME_ID 28

#define STATE_MAIN_MENU 1
#define STATE_PLAYING 2
#define STATE_HELP 3
#define STATE_INFO 4
#define STATE_PAUSE 5
#define STATE_GAME_OVER 6

#define BLOB_PIXELS 6

#define BLOB_FREE 0
#define PLAYFIELD_ZERO_X         0         // zero x-position of the playfield in pixel position on the screen
#define PLAYFIELD_ZERO_Y         0         // zero x-position of the playfield in pixel position on the screen

#define BLOB_CURRENT             0
#define BLOB_NEXT                2
#define BLOB_WAITING             4

#define TILES_IN_BLOBS           3 

#define NO_FLAG_ON_FIELD         0
#define FLAG_ON_FIELD            1

int field[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];
int fieldFlags[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];

int Current_Blobs[] =
{
  // array for current blob
  0, 0, 0,
  0, 0, 0,
  0, 0, 0,
};

int RandomBlobPit[6];
int BlobNumbers;

int BlobsXY[2];          // X and Y coordinates for the small 3x3 blob grid

boolean canMoveBlobsDown;
boolean giveExtraScore;

unsigned long scorePlayer;
unsigned long extraScoreForChain;

byte state;

byte selector;

unsigned long highScore;

Gamebuino gb;
void setup() {
  gb.begin();
  gb.titleScreen(F("Blob Attack"));
  gb.pickRandomSeed();
   for(int y = 0; y < PLAYFIELD_HEIGHT;y++) {
    for(int x = 0; x < PLAYFIELD_WIDTH;x++) {
      field[x][y] = BLOB_FREE;
    }
   }
  FillBlobPit();
  CreateCurrentBlobs();
  removeFlag();
  canMoveBlobsDown = true;
  giveExtraScore = false;
  scorePlayer = 0;
  removeFlag();
  InitPlayfield();
  extraScoreForChain = 0;
  state = STATE_MAIN_MENU;
  selector = 0;
  
  highScore = EEPROM.read(0);
}

void DrawField() {
  for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
  {
    for (int x = 0 ; x < PLAYFIELD_WIDTH; x++)
    {
      // draw every tile in the playfield
      gb.display.drawBitmap(x * BLOB_PIXELS, y * BLOB_PIXELS, blobs_bitmap[field[x][y]], ROTCCW, NOFLIP);
    }
  }
}

void InitPlayfield()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      field[x][y] = BLOB_FREE;
    }
  }
}


void FillBlobPit()
{
  for (byte x = 0; x < 6; x++)
  {
    RandomBlobPit[x] = random(1, 4);
  }
}

void CreateCurrentBlobs ()
{
  BlobsXY[0] = 2;     //player X
  BlobsXY[1] = 0;     //player Y
  for (byte i = 0; i < 9; i++)
  {
    Current_Blobs[i] = 0;
  }
  Current_Blobs[1] = RandomBlobPit[0];
  Current_Blobs[4] = RandomBlobPit[1];

  for (byte i = 0; i < 4; i++)
  {
    RandomBlobPit[i] = RandomBlobPit[i + 2];
  }

  RandomBlobPit[4] = random(1, 6);
  RandomBlobPit[5] = random(1, 6);
}

void DrawBlobs (int draw_x, int draw_y, int which_blobs)
{
  switch(which_blobs) {
    case BLOB_CURRENT: {
        int draw_pointer = 0;
        for (int y = draw_y; y < draw_y + 18; y = y + BLOB_PIXELS)
        {
          for (int x = draw_x; x < draw_x + 18; x = x + BLOB_PIXELS)
          {
            int temp = Current_Blobs[draw_pointer];
            if (temp > 0)
            {
              gb.display.drawBitmap(x, y, blobs_bitmap[temp], ROTCCW, NOFLIP);
            }
            draw_pointer++;
          }
        }
        break;
    }
    case BLOB_NEXT: {
      gb.display.cursorX = 84 - 21;
      gb.display.cursorY = 0;
      gb.display.println("Next");
      gb.display.drawBitmap(draw_x, draw_y + 8, blobs_bitmap[RandomBlobPit[0]], ROTCCW, NOFLIP);
      gb.display.drawBitmap(draw_x, draw_y + 8 + 8, blobs_bitmap[RandomBlobPit[1]], ROTCCW, NOFLIP);
      break;
    }
  }
}

boolean aboveIsSameBlob(int array_x, int array_y)
{
  if ((array_y - 1 > 0) && (field [array_x][array_y] == field [array_x][array_y - 1])) return true;
  else return false;
}

boolean underIsSameBlob(int array_x, int array_y)
{
  if ((array_y + 1 < PLAYFIELD_HEIGHT) && (field [array_x][array_y] == field [array_x][array_y + 1])) return true;
  else return false;
}

boolean rightIsSameBlob(int array_x, int array_y)
{
  if ((array_x + 1 < PLAYFIELD_WIDTH ) && (field [array_x][array_y] == field [array_x + 1][array_y])) return true;
  else return false;
}

boolean leftIsSameBlob(int array_x, int array_y)
{
  if ((array_x - 1 > 0 ) && (field [array_x][array_y] == field [array_x - 1][array_y])) return true;
  else return false;
}

boolean IsMovePossible (int array_x, int array_y)
{
  // checks collision with blocks already stored in the playfield
  // check if the 3x3 tiles of a blob with the correct area in the playfield provide by draw_x and draw_y
  int draw_pointer = 0;
  for (int y = array_y; y < array_y + TILES_IN_BLOBS; y++)
  {
    for (int x = array_x; x < array_x + TILES_IN_BLOBS; x++)
    {
      // check if the block is outside the limits of the playfield
      if (x < 0 || x > PLAYFIELD_WIDTH - 1 || y > PLAYFIELD_HEIGHT - 1)
      {
        byte temp = Current_Blobs[draw_pointer];
        if ( temp != 0) return false;
      }

      // check if the block has collided with tile already stored in the playfield array
      byte temp = Current_Blobs[draw_pointer];
      if ((temp != 0) && (IsTileFree(x, y) == false)) return false;
      draw_pointer++;
    }
  }
  return true;
}

void removeFlag()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      fieldFlags[x][y] = NO_FLAG_ON_FIELD;
    }
  }
}

void fourInPack()
{
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!IsTileFree(column, row))
      {
        if (aboveIsSameBlob(column, row) && rightIsSameBlob(column, row) && aboveIsSameBlob(column + 1, row))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
        }
        if (rightIsSameBlob(column, row) && aboveIsSameBlob(column + 1, row) && rightIsSameBlob(column + 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 2][row - 1] = FLAG_ON_FIELD;
        }
        if (rightIsSameBlob(column, row) && underIsSameBlob(column + 1, row) && rightIsSameBlob(column + 1, row + 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row + 1] = FLAG_ON_FIELD;
          fieldFlags[column + 2][row + 1] = FLAG_ON_FIELD;
        }
        if (aboveIsSameBlob(column, row) && rightIsSameBlob(column, row - 1) && aboveIsSameBlob(column + 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 2] = FLAG_ON_FIELD;
        }
        if (aboveIsSameBlob(column, row) && leftIsSameBlob(column, row - 1) && aboveIsSameBlob(column - 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column - 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column - 1][row - 2] = FLAG_ON_FIELD;
        }
      }
    }
  }
}


void fourInColumn()
{
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!IsTileFree(column, row))
      {
        if (aboveIsSameBlob(column, row) && aboveIsSameBlob(column, row - 1))
        {
          if (aboveIsSameBlob(column, row - 2))
          {
            fieldFlags[column][row] = FLAG_ON_FIELD;
            fieldFlags[column][row - 1] = FLAG_ON_FIELD;
            fieldFlags[column][row - 2] = FLAG_ON_FIELD;
            fieldFlags[column][row - 3] = FLAG_ON_FIELD;
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (rightIsSameBlob(column, row - temp))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column][row - 1] = FLAG_ON_FIELD;
              fieldFlags[column][row - 2] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row - temp] = FLAG_ON_FIELD;
            }
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (leftIsSameBlob(column, row - temp))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column][row - 1] = FLAG_ON_FIELD;
              fieldFlags[column][row - 2] = FLAG_ON_FIELD;
              fieldFlags[column - 1][row - temp] = FLAG_ON_FIELD;
            }
          }
        }
      }
    }
  }
}


void fourInRow()
{
  //check if 4 or more blobs are equal in the same row
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!IsTileFree(column, row))
      {
        if (rightIsSameBlob(column, row) && rightIsSameBlob(column + 1, row))
        {
          if (rightIsSameBlob(column + 2, row))
          {
            fieldFlags[column][row] = FLAG_ON_FIELD;
            fieldFlags[column + 1][row] = FLAG_ON_FIELD;
            fieldFlags[column + 2][row] = FLAG_ON_FIELD;
            fieldFlags[column + 3][row] = FLAG_ON_FIELD;
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (aboveIsSameBlob(column + temp, row))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row] = FLAG_ON_FIELD;
              fieldFlags[column + 2][row] = FLAG_ON_FIELD;
              fieldFlags[column + temp][row - 1] = FLAG_ON_FIELD;
            }
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (underIsSameBlob(column + temp, row))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row] = FLAG_ON_FIELD;
              fieldFlags[column + 2][row] = FLAG_ON_FIELD;
              fieldFlags[column + temp][row + 1] = FLAG_ON_FIELD;
            }
          }
        }
      }
    }
  }
}

boolean IsOnlyOneBlob()
{
  byte temp = 0;
  for (int i = 0; i < 9; i++)
  {
    if (Current_Blobs[i] != 0)temp++;
  }
  if (temp < 2) return true;
  else return false;
}

boolean IsTileFree(int array_x, int array_y)
{
  if (field [array_x][array_y] == BLOB_FREE) return true;
  else return false;
}

void RotateBlobsRight()
{
  if (!IsOnlyOneBlob())
  {
    byte temp = Current_Blobs[1];
    Current_Blobs[1] = Current_Blobs[3];
    Current_Blobs[3] = Current_Blobs[7];
    Current_Blobs[7] = Current_Blobs[5];
    Current_Blobs[5] = temp;
   gb.sound.playTick();
  }
  if (!IsMovePossible(BlobsXY[0], BlobsXY[1]))
  {
    byte temp = Current_Blobs[1];
    Current_Blobs[1] = Current_Blobs[5];
    Current_Blobs[5] = Current_Blobs[7];
    Current_Blobs[7] = Current_Blobs[3];
    Current_Blobs[3] = temp;
  }
}

void RotateBlobsLeft()
{
  if (!IsOnlyOneBlob())
  {
    byte temp = Current_Blobs[1];
    Current_Blobs[1] = Current_Blobs[5];
    Current_Blobs[5] = Current_Blobs[7];
    Current_Blobs[7] = Current_Blobs[3];
    Current_Blobs[3] = temp;
    gb.sound.playTick();
  }
  if (!IsMovePossible(BlobsXY[0], BlobsXY[1]))
  {
    byte temp = Current_Blobs[1];
    Current_Blobs[1] = Current_Blobs[3];
    Current_Blobs[3] = Current_Blobs[7];
    Current_Blobs[7] = Current_Blobs[5];
    Current_Blobs[5] = temp;
  }
}

void MoveBlobsRight()
{
  if (!IsOnlyOneBlob() && IsMovePossible(BlobsXY[0] + 1, BlobsXY[1])) BlobsXY[0]++;
}


void MoveBlobsLeft()
{
  if (!IsOnlyOneBlob() && IsMovePossible(BlobsXY[0] - 1, BlobsXY[1])) BlobsXY[0]--;
}

void DrawCurrentBlobs()
{
  DrawBlobs((BlobsXY[0]*BLOB_PIXELS) + PLAYFIELD_ZERO_X, (BlobsXY[1]*BLOB_PIXELS) + PLAYFIELD_ZERO_Y, BLOB_CURRENT);
}


void moveBlobsDown()
{
  canMoveBlobsDown = false;
  for (int column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (int row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (IsTileFree(column, row))
      {
        if (!IsTileFree(column, row - 1))
        {
          field [column][row] = field [column][row - 1];
          field [column][row - 1] = BLOB_FREE;
          DrawField();
          //gb.display.drawBitmap((column * BLOB_PIXELS), (row * BLOB_PIXELS), blob00inverted_bitmap, NOROT, NOFLIP);
          gb.display.drawBitmap((column * BLOB_PIXELS), (row * BLOB_PIXELS),  blobs_bitmap[field[column][row]], NOROT, NOFLIP);
          canMoveBlobsDown = true;
        }
      }
    }
  }
}

boolean IsOneBlobDropPossible(int array_x, int array_y)
{
  // checks 1 blob collision with blobs already stored in the playfield
  if ((Current_Blobs[1] == 0) && (Current_Blobs[7] == 0))
  {
    for (byte temp = 3; temp < 6; temp++)
    {
      if ((Current_Blobs[temp] != 0) && (IsTileFree(array_x, array_y) == false)) return true;
    }
  }
  else return false;
}

void DeletePossibleBlobs()
{
  while (canMoveBlobsDown)
  {
    fourInPack();
    fourInColumn();
    fourInRow();
    removeGroups();
    moveBlobsDown();
  }
  canMoveBlobsDown = true;
}

void DropBlobs()
{
  if (IsOneBlobDropPossible(BlobsXY[0], BlobsXY[1] + 1)) StoreOneBlob(BlobsXY[0], BlobsXY[1]);
  //move down is no longer possible because the field is full, the game is over
  if ((BlobsXY[1] == 0) && !IsTileFree(BlobsXY[0] + 1, 0)) state = STATE_GAME_OVER;
  if (IsMovePossible(BlobsXY[0], BlobsXY[1] + 1))
  {
    BlobsXY[1]++;
    gb.sound.playTick();
  } else if(state != STATE_GAME_OVER) {
    StoreBlob(BlobsXY[0], BlobsXY[1]);
    scorePlayer += 10;
    DeletePossibleBlobs();
    CreateCurrentBlobs();
  }
}

void StoreBlob(int array_x, int array_y)
{
  int draw_pointer = 0;
  for (int y = array_y; y < array_y + TILES_IN_BLOBS; y++)
  {
    for (int x = array_x; x < array_x + TILES_IN_BLOBS; x++)
    {
      if (Current_Blobs[draw_pointer] != 0) field[x][y] = Current_Blobs[draw_pointer];
      draw_pointer++;
    }
  }
}

void removeGroups()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      if (fieldFlags[x][y] == FLAG_ON_FIELD)
      {
        giveExtraScore = true;
        field[x][y] = BLOB_FREE;
        scorePlayer += 50;
      }
    }
  }
  if (giveExtraScore == true)
  {
    // TODO
    scorePlayer += extraScoreForChain;
    extraScoreForChain += 500;
    gb.sound.playNote(440, 2, 0);
    delay(100);
    gb.sound.playNote(1047, 2, 0);
  }
  giveExtraScore = false;
  removeFlag();
}

void StoreOneBlob(int array_x, int array_y)
{
  // if the blob is not on the floor
  if ((array_y) < PLAYFIELD_HEIGHT - 2)
  {

    for (int x = 0; x < TILES_IN_BLOBS; x++)
    {
      if ((!IsTileFree(array_x + x, array_y + 2)) && (!IsOnlyOneBlob()) && (Current_Blobs[3 + x] != 0))
      {
        field [array_x + x][array_y + 1] = Current_Blobs[3 + x];
        Current_Blobs[3 + x] = 0;
      }
    }
  }
}

void loop() {
  if(gb.update()) {
    switch(state) {
      case STATE_MAIN_MENU: {
        FillBlobPit();
        CreateCurrentBlobs();
        removeFlag();
        InitPlayfield();
        canMoveBlobsDown = true;
        giveExtraScore = false;
        scorePlayer = 0;
        extraScoreForChain = 0;
        gb.display.drawBitmap(0, 26, splashscreen1);
        gb.display.drawBitmap(0, 0, splashscreen2);
        gb.display.cursorX = 84 - 16;
        gb.display.cursorY = 13;
        gb.display.println("HELP");
        gb.display.cursorX = 84 - 16;
        gb.display.cursorY = 21;
        gb.display.println("INFO");
        gb.display.cursorX = 84 - 16;
        gb.display.cursorY = 29;
        gb.display.println("PLAY");
        
        if(gb.buttons.pressed(BTN_DOWN)) selector++;
        if(gb.buttons.pressed(BTN_UP)) selector--;
        if(selector < 0) selector = 0;
        if(selector > 2) selector = 2;
        
        switch(selector) {
          case 0: {
            if(gb.buttons.pressed(BTN_A)) state = STATE_HELP;
            break;
          }
          case 1: {
            if(gb.buttons.pressed(BTN_A)) state = STATE_INFO;
            break;
          }
          case 2: {
            if(gb.buttons.pressed(BTN_A)) state = STATE_PLAYING;
            break;
          }
        }
        
        gb.display.drawLine(84 - 16, 11 + (selector * 8), 84, 11 + (selector * 8));
        break;
      }
      case STATE_PLAYING: {
        DrawField();
        
        if(gb.frameCount % 25 == 0) DropBlobs();
        if(gb.buttons.pressed(BTN_A)) {RotateBlobsLeft();}
        if(gb.buttons.pressed(BTN_B)) {RotateBlobsRight();}
        if(gb.buttons.pressed(BTN_C)) {gb.titleScreen(F("Blob Attack"));}
        if(gb.buttons.pressed(BTN_RIGHT)) {MoveBlobsRight();}
        if(gb.buttons.pressed(BTN_LEFT)) {MoveBlobsLeft();}
        if(gb.buttons.pressed(BTN_DOWN)) {DropBlobs();}
        if(gb.buttons.pressed(BTN_UP)) {state = STATE_PAUSE;}
        DrawBlobs(84 - 20, 0, BLOB_NEXT);
        DrawCurrentBlobs();
        break;
      }
      case STATE_INFO: {
        if(gb.buttons.pressed(BTN_B)) state = STATE_MAIN_MENU;
        else {
          gb.display.cursorX = 0;
          gb.display.cursorY = 0;
          gb.display.println("A game made by");
          gb.display.println("TEAM Arg");
          gb.display.println("B to go back");
        }
        break;
      }
      case STATE_HELP: {
        if(gb.buttons.pressed(BTN_B)) state = STATE_MAIN_MENU;
        else {
          gb.display.cursorX = 0;
          gb.display.cursorY = 0;
          gb.display.println("Visit");
          gb.display.println("http://www.team-arg.");
          gb.display.println("org/BLBA-manual.html");
          gb.display.println("B to go back");
        }
        break;
      }
      case STATE_PAUSE: {
        if(gb.buttons.pressed(BTN_B)) state = STATE_PLAYING;
        else {
          gb.display.drawBitmap(LCDWIDTH / 2 - 9, LCDHEIGHT / 2 - 3, pause);
          gb.display.println("B to play");
          gb.display.cursorX = 0;
          gb.display.cursorY = LCDHEIGHT - 5;
          gb.display.print("Score: ");
          gb.display.println(scorePlayer);
        }
        break;
      }
      case STATE_GAME_OVER: {
         if(gb.buttons.pressed(BTN_A)) state = STATE_MAIN_MENU;
         else {
           gb.display.drawBitmap(0, 26, splashscreen1);
           gb.display.cursorX = LCDWIDTH / 2 - 26;
           gb.display.cursorY = 0;
           gb.display.println("Game Over");
           gb.display.cursorX = LCDWIDTH / 2 - 21;
           gb.display.cursorY = 7;
           gb.display.println("Press A");
           gb.display.cursorX = 0;
           gb.display.cursorY = 14;
           gb.display.print("Score ");
           gb.display.println(scorePlayer);
           if(scorePlayer > highScore) {
             EEPROM.write(0, scorePlayer);
             highScore = scorePlayer;
           }
           gb.display.cursorX = 0;
           gb.display.cursorY = 21;
           gb.display.print("HI-SCORE: ");
           gb.display.println(highScore);
         }
        break;
      }
    }
  }
}
