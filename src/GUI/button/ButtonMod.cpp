#include "ButtonMod.h"






/***************************************************************************************
** Code for the GFX button UI element
** Grabbed from Adafruit_GFX library and enhanced to handle any label font
***************************************************************************************/
TFT_eSPI_Button_Mod::TFT_eSPI_Button_Mod(void) {
  _gfx       = nullptr;
  _xd        = 0;
  _yd        = 0;
  _textdatum = MC_DATUM;
  _label[9]  = '\0';
}

// Classic initButton() function: pass center & size
void TFT_eSPI_Button_Mod::initButton(
 TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h,
 uint16_t outline, uint16_t fill, uint16_t textcolor,
 char *label, uint8_t textfont)
{
  // Tweak arguments and pass to the newer initButtonUL() function...
  initButtonUL(gfx, x - (w / 2), y - (h / 2), w, h, outline, fill,
    textcolor, label, textfont);
}

// Newer function instead accepts upper-left corner & size
void TFT_eSPI_Button_Mod::initButtonUL(
 TFT_eSPI *gfx, int16_t x1, int16_t y1, uint16_t w, uint16_t h,
 uint16_t outline, uint16_t fill, uint16_t textcolor,
 char *label, uint8_t textfont)
{
  _x1           = x1;
  _y1           = y1;
  _w            = w;
  _h            = h;
  _outlinecolor = outline;
  _fillcolor    = fill;
  _textcolor    = textcolor;
  _textfont     = textfont;
  _gfx          = gfx;
  strncpy(_label, label, 9);
}

// Adjust text datum and x, y deltas
void TFT_eSPI_Button_Mod::setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum)
{
  _xd        = x_delta;
  _yd        = y_delta;
  _textdatum = datum;
}

void TFT_eSPI_Button_Mod::drawButton(bool inverted, String long_name) {
  uint16_t fill, outline, text;

  if(!inverted) {
    fill    = _fillcolor;
    outline = _outlinecolor;
    text    = _textcolor;
  } else {
    fill    = _textcolor;
    outline = _outlinecolor;
    text    = _fillcolor;
  }

  //uint8_t r = min(_w, _h) / 4; // Corner radius
  _gfx->fillRect(_x1, _y1, _w, _h, fill);
  _gfx->drawRect(_x1, _y1, _w, _h, outline);

  if (_gfx->textfont == 255) {
    _gfx->setCursor(_x1 + (_w / 8),
                    _y1 + (_h / 4));
    _gfx->setTextColor(text);
    _gfx->setTextSize(1);
    _gfx->print(_label);
  }
  else {
    _gfx->setTextColor(text, fill);
    _gfx->setTextSize(1);

    uint8_t tempdatum = _gfx->getTextDatum();
    _gfx->setTextDatum(_textdatum);
    uint16_t tempPadding = _gfx->getTextPadding();
    _gfx->setTextPadding(0);

    uint8_t correction = 10;
    if (_textfont == 2) {
      correction = 8;
    } 

    if (long_name == "") {
      _gfx->setTextColor(outline);
      _gfx->drawCentreString(_label, _x1 + (_w/2) + 1, _y1 + (_h/2 - correction) + 1, _textfont);
      _gfx->setTextColor(text);
      _gfx->drawCentreString(_label, _x1 + (_w/2), _y1 + (_h/2 - correction), _textfont);
    } else {
      _gfx->drawCentreString(long_name, _x1 + (_w/2), _y1 + (_h/2 - correction), _textfont);
    }

    _gfx->setTextDatum(tempdatum);
    _gfx->setTextPadding(tempPadding);
  }
}

bool TFT_eSPI_Button_Mod::contains(int16_t x, int16_t y) {
  return ((x >= _x1) && (x < (_x1 + _w)) &&
          (y >= _y1) && (y < (_y1 + _h)));
}

void TFT_eSPI_Button_Mod::press(bool p) {
  laststate = currstate;
  currstate = p;
}

bool TFT_eSPI_Button_Mod::isPressed()    { return currstate; }
bool TFT_eSPI_Button_Mod::justPressed()  { return (currstate && !laststate); }
bool TFT_eSPI_Button_Mod::justReleased() { return (!currstate && laststate); }
