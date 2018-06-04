//  This code is used for the ELEC9607 project in 2018
//  3D game machine--To Hell With Johnny
//  This is a free software and we will be happy if it can help you.
//  Actually the code is little messy and clumsy. orz...
//  Created by Charles & Shuang on 2018/6/4.
//  Copyright © 2018 Charles & Shuang All rights reserved.


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#define HC05 Serial1

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

/*The kind of brick*/
#define normal 0
#define unstable 1
#define harsh 2
#define broken 3

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

static const unsigned char PROGMEM floor_bmp[] =
{
  B11110100, B00000000, B00000000,
  B10000100, B00000000, B00000000,
  B10000100, B00000000, B00000000,
  B11110100, B11000110, B01010100,
  B10000101, B00101001, B01100000,
  B10000101, B00101001, B01000100,
  B10000100, B11000110, B01000000,
  B00000000, B00000000, B00000000,
};

static const unsigned char PROGMEM man_bmp[] =
{
  B01111100,
  B10000010,
  B10101010,
  B10101010,
  B10000010,
  B10000010,
  B10101010,
  B01010100,
};

static const unsigned char PROGMEM normal_bmp[] =
{
  B11111111, B11111110,
  B11111111, B11111110,
  B11111111, B11111110,
};

static const unsigned char PROGMEM harsh_bmp[] =
{
  B01000100, B01000100,
  B10101010, B10101010,
  B11111111, B11111110,
};

static const unsigned char PROGMEM unstable_bmp[] =
{
  B11111111, B11111110,
  B10000000, B00000010,
  B11111111, B11111110,
};

static const unsigned char PROGMEM broken_bmp[] =
{
  B11000000, B00000110,
  B11000000, B00000110,
  B11000000, B00000110,
};

int brick[5] = { 0, 2, 0, 0, 1 };
int xb0 = 17;              /*the coordinate of the bricks*/
int yb0 = 48;
int xb1 = 3;
int yb1 = 64;
int xb2 = 12;
int yb2 = 79;
int xb3 = 34;
int yb3 = 93;
int xb4 = 22;
int yb4 = 111;

int refreshtime = 150;
int count_broken = 0;   /*time on unstable brick*/

int xm = 21;                            /*the coordinate of the characters*/
int ym = 41;

int score = 0;
int on_brick = 0;/*whether on bricks*/
int start = 0;
int death = 0;
int the_brick = 0;

void setup()
{
  Serial.begin(9600);
  display.begin();
  // init done
  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(53);
  display.clearDisplay();
  display.setRotation(1);
  on_brick = 0;
  Wire.begin();
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  HC05.begin(38400);
}

void loop()
{
  if (start == 0)
    {
      gamestart();
      if (HC05.available() > 0)
        {
          char data = HC05.read();
          if (data == '1')
            {
              refreshtime = 300;
              start = 1;
            }
          if (data == '2')
            {
              refreshtime = 150;
              start = 1;
            }
          if (data == '3')
            {
              refreshtime = 77;
              start = 1;
            }
        }
    }
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  if (start == 1 && death == 0)
    {
      initiate_game();
      whetheronbrick();
      brick_happen();
      movement_of_man();
      display.drawBitmap(xm, ym, man_bmp, 7, 8, BLACK);
      display.display();
      determine_death();
      delay(refreshtime);
      on_brick = 0;
      Serial.println(ax);
    }
  if (death != 0)
    {
      gameover();
    }
}

void initiate_game()
{
  display.drawBitmap(0, 0, floor_bmp, 24, 8, BLACK);
  display.drawRect(0, 8, 48, 76, BLACK);
  display.drawRect(0, 9, 48, 76, BLACK);
  display.drawRect(0, 10, 48, 76, BLACK);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(24, 0);
  display.println(score);
  display.display();
}

//whether on brick
void whetheronbrick()
{
  if (xm >= (xb0 - 5) && xm <= (xb0 + 13) && (yb0 - ym) <= 9 && (yb0 - ym) >= 6)
    {
      on_brick = 1;
      the_brick = 0;
    }

  if (xm >= (xb1 - 5) && xm <= (xb1 + 13) && (yb1 - ym) <= 9 && (yb1 - ym) >= 6)
    {
      on_brick = 1;
      the_brick = 1;
    }

  if (xm >= (xb2 - 5) && xm <= (xb2 + 13) && (yb2 - ym) <= 9 && (yb2 - ym) >= 6)
    {
      on_brick = 1;
      the_brick = 2;
    }

  if (xm >= (xb3 - 5) && xm <= (xb3 + 13) && (yb3 - ym) <= 9 && (yb3 - ym) >= 6)
    {
      on_brick = 1;
      the_brick = 3;
    }

  if (xm >= (xb4 - 5) && xm <= (xb4 + 13) && (yb4 - ym) <= 9 && (yb4 - ym) >= 6)
    {
      on_brick = 1;
      the_brick = 4;
    }
}

//generate and move bricks
void brick_happen()
{
  display.clearDisplay();
  initiate_game();
  if (yb0 >= 9)
    {
      switch (brick[0])
        {
        case 0: display.drawBitmap(xb0, yb0, normal_bmp, 15, 3, BLACK);

        case 1: display.drawBitmap(xb0, yb0, unstable_bmp, 15, 3, BLACK);

        case 2: display.drawBitmap(xb0, yb0, harsh_bmp, 15, 3, BLACK);

        case 3: display.drawBitmap(xb0, yb0, broken_bmp, 15, 3, BLACK);
        }
      yb0--;
    }
  else
    {
      yb0 = 83;
      xb0 = rand() % 33;
      brick[0] = rand() % 3;
      if (brick[2] | brick[3] | brick[4] != 0)
        brick[0] = 0;
      score++;
    }

  if (yb1 >= 9)
    {
      switch (brick[1])
        {
        case 0: display.drawBitmap(xb1, yb1, normal_bmp, 15, 3, BLACK);

        case 1: display.drawBitmap(xb1, yb1, unstable_bmp, 15, 3, BLACK);

        case 2: display.drawBitmap(xb1, yb1, harsh_bmp, 15, 3, BLACK);

        case 3: display.drawBitmap(xb1, yb1, broken_bmp, 15, 3, BLACK);
        }
      yb1--;
    }
  else
    {
      yb1 = 83;
      xb1 = rand() % 33;
      brick[1] = rand() % 3;
      if (brick[0] | brick[3] | brick[4] != 0)
        brick[1] = 0;
      score++;
    }

  if (yb2 >= 9)
    {
      switch (brick[2])
        {
        case 0: display.drawBitmap(xb2, yb2, normal_bmp, 15, 3, BLACK);

        case 1: display.drawBitmap(xb2, yb2, unstable_bmp, 15, 3, BLACK);

        case 2: display.drawBitmap(xb2, yb2, harsh_bmp, 15, 3, BLACK);

        case 3: display.drawBitmap(xb2, yb2, broken_bmp, 15, 3, BLACK);
        }
      yb2--;
    }
  else
    {
      yb2 = 83;
      xb2 = rand() % 33;
      brick[2] = rand() % 3;
      score++;
      if (brick[0] | brick[1] | brick[4] != 0)
        brick[2] = 0;
    }

  if (yb3 >= 9)
    {
      switch (brick[3])
        {
        case 0: display.drawBitmap(xb3, yb3, normal_bmp, 15, 3, BLACK);

        case 1: display.drawBitmap(xb3, yb3, unstable_bmp, 15, 3, BLACK);

        case 2: display.drawBitmap(xb3, yb3, harsh_bmp, 15, 3, BLACK);

        case 3: display.drawBitmap(xb3, yb3, broken_bmp, 15, 3, BLACK);
        }
      yb3--;
    }
  else
    {
      yb3 = 83;
      xb3 = rand() % 33;
      brick[3] = rand() % 3;
      score++;
      if (brick[0] | brick[1] | brick[2] != 0)
        brick[3] = 0;
    }

  if (yb4 >= 9)
    {
      switch (brick[4])
        {
        case 0: display.drawBitmap(xb4, yb4, normal_bmp, 15, 3, BLACK);

        case 1: display.drawBitmap(xb4, yb4, unstable_bmp, 15, 3, BLACK);

        case 2: display.drawBitmap(xb4, yb4, harsh_bmp, 15, 3, BLACK);

        case 3: display.drawBitmap(xb4, yb4, broken_bmp, 15, 3, BLACK);
        }
      yb4--;
    }
  else
    {
      yb4 = 83;
      xb4 = rand() % 33;
      brick[4] = rand() % 3;
      score++;
      if (brick[1] | brick[2] | brick[3] != 0)
        brick[4] = 0;
    }
}

//movement of man
int movement_of_man()
{
  if (ax >= 5555)
    {
      xm = xm - 2;
    }
  if (ax <= 5555 && ax >= 0)
    {
      xm = xm - 1;
    }
  if (ax <= -5555)
    {
      xm = xm + 2;
    }
  if (ax >= -5555 && ax <= 0)
    {
      xm = xm + 1;
    }


  if (on_brick)                       //if on bricks
    {
      switch (brick[the_brick])
        {
        case 0:
        {
          ym--;
          break;
        }

        case 1:
        {
          if (count_broken <= 3)
            {
              ym--;
              count_broken++;
            }
          else
            {
              on_brick = 0;
              brick[the_brick] = 3;
              count_broken = 0;
            }
          break;
        }
        }
    }
  else
    {
      ym = ym + 3;
    }
  if (xm <= 1)
    {
      xm = 1;
    }
  if (xm >= 40)
    {
      xm = 40;
    }
}

int determine_death()
{
  if (ym <= 9 || ym >= 78 || brick[the_brick] == 2)
    {
      death = 1;
      display.clearDisplay();
      score = 0;
    }
}

void gamestart()
{
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(3, 8);
  display.println("TO HELL");
  display.setCursor(12, 16);
  display.println("WITH");
  display.setCursor(6, 24);
  display.println("JOHNNY");
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.drawBitmap(20, 36, man_bmp, 7, 8, BLACK);
  display.setCursor(0, 49);
  display.println("1.EASY");
  display.setCursor(0, 58);
  display.println("2.NORMAL");
  display.setCursor(0, 67);
  display.println("3.HARD");
  display.display();
}

void gameover()
{
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0, 16);
  display.println("GAME");
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0, 32);
  display.println("OVER");
  display.display();
}

