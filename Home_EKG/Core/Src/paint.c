/*
 * paint.c
 *
 *  Created on: Nov 13, 2020
 *      Author: Mateusz Salamon
 */
#include "main.h"
#include "paint.h"
#include "TFT_ILI9341.h"
#include "XPT2064.h"
#include "GFX_Color.h"
#include "GFX_EnhancedFonts.h"

#define DRAW_WINDOW_X_START 0
#define DRAW_WINDOW_X_STOP ILI9341_TFTWIDTH
#define DRAW_WINDOW_Y_START 30
#define DRAW_WINDOW_Y_STOP (ILI9341_TFTHEIGHT-60)

#define CLEAR_BUTTON_X ILI9341_TFTWIDTH-130
#define CLEAR_BUTTON_Y 3
#define CLEAR_BUTTON_WIDTH 120
#define CLEAR_BUTTON_HEIGHT 26

#define COLOR_BUTTON_WIDTH 36
#define COLOR_BUTTON_HEIGHT 20

#define USED_COLORS	5

typedef enum
{
	PAINT_INIT, // Build GUI
	PAINT_DRAW, // Read Touch and draw pixels
	PAINT_CLEAR // Clear drawing area
} PaintState;

PaintState State = PAINT_INIT; // Initialization state for Paint State Machine

uint16_t UsedColors[USED_COLORS] = {ILI9341_BLACK, ILI9341_BLUE, ILI9341_GREEN, ILI9341_RED, ILI9341_ORANGE}; // Colors table

uint16_t CurrentColor = ILI9341_BLACK; // Default color

//
// Draw Color indicator as Circle above the drawing area
//
void ColorIndicator(void)
{
	GFX_DrawFillCircle(ILI9341_TFTWIDTH/2, 15, 12, CurrentColor); // Drawing circle with GFX Library
}

//
// Draw Clearing Button above the drawing area
//
void ClearButton(void)
{
	GFX_DrawFillRectangle(CLEAR_BUTTON_X, CLEAR_BUTTON_Y, CLEAR_BUTTON_WIDTH, CLEAR_BUTTON_HEIGHT, ILI9341_DARKGREEN); // Button Color
	  //ILI9341_ClearArea(CLEAR_BUTTON_X, CLEAR_BUTTON_Y, CLEAR_BUTTON_WIDTH, CLEAR_BUTTON_HEIGHT, ILI9341_DARKGREEN); // Homework - to write this function in TFT library
	EF_PutString((const uint8_t*)"CZYŚĆ", CLEAR_BUTTON_X+5, CLEAR_BUTTON_Y, ILI9341_WHITE, BG_TRANSPARENT, ILI9341_DARKGREEN); // Button text
}

//
// Draw each button for color change in one loop
//
//
//
void ColorButtons(void)
{
	uint8_t i;
	for(i = 1; i <= USED_COLORS; i++) // For each color
	{
		GFX_DrawFillRectangle((i*(ILI9341_TFTWIDTH/(USED_COLORS+1)))-(COLOR_BUTTON_WIDTH/2), ILI9341_TFTHEIGHT-25, COLOR_BUTTON_WIDTH, COLOR_BUTTON_HEIGHT, UsedColors[i-1]);
		// X calculation: (i*(ILI9341_TFTWIDTH/(USED_COLORS+1)))-(COLOR_BUTTON_WIDTH/2)
		//					(i*(ILI9341_TFTWIDTH/(USED_COLORS+1))) - define a middle of each button. That's why (USED_COLORS+1)
		//					-(COLOR_BUTTON_WIDTH/2) - Move back by half of button.

		//ILI9341_ClearArea((i*(ILI9341_TFTWIDTH/(USED_COLORS+1)))-(BUTTON_WIDTH/2), ILI9341_TFTHEIGHT-25, BUTTON_WIDTH, BUTTON_HEIGHT, UsedColors[i-1]);
	}
}

uint8_t IsColorButtonTouched(uint16_t x, uint16_t y)
{
	uint8_t i;
	for(i = 1; i <= USED_COLORS; i++) // For each color used
	{
		 // Check if Touch point is higher than X begin of current color button
		if(x > (i*(ILI9341_TFTWIDTH/(USED_COLORS+1)))-(COLOR_BUTTON_WIDTH/2))
		{
			 // Check if Touch point is lower than X end of current color button
			if(x < (i*(ILI9341_TFTWIDTH/(USED_COLORS+1)))+(COLOR_BUTTON_WIDTH/2))
			{
				// Check if Touch point is higher than Y begin of current color button
				if(y > ILI9341_TFTHEIGHT-25)
				{
					// Check if Touch point is lower than Y end of current color button
					if(y < (ILI9341_TFTHEIGHT-25+COLOR_BUTTON_HEIGHT))
					{
						// If we are sure that touched point was inside current (i) button - return button number
						return i;
					}
				}
			}
		}
	}
	// If no color button touched
	return 0;
}

uint8_t IsClearButtonTouched(uint16_t x, uint16_t y)
{
	// Check if Touch point is higher than X begin of clear button
	if(x > CLEAR_BUTTON_X)
	{
		// Check if Touch point is higher than X end of clear button
		if(x < (CLEAR_BUTTON_X + CLEAR_BUTTON_WIDTH))
		{
			// Check if Touch point is higher than Y begin of clear button
			if(y > CLEAR_BUTTON_Y)
			{
				// Check if Touch point is higher than Y end of clear button
				if(y < (CLEAR_BUTTON_Y+CLEAR_BUTTON_HEIGHT))
				{
					// If we are sure that touched point was inside clear button - return 1
					return 1;
				}
			}
		}
	}
	// If clear button is not touched
	return 0;
}

//
// PAINT_INIT state function
//
void InitScreen(void)
{
	// Clear whole display
	ILI9341_ClearDisplay(ILI9341_WHITE);
	// Title
	EF_PutString((const uint8_t*)"Paint", 5, 2, ILI9341_BLACK, BG_TRANSPARENT, ILI9341_GREEN);
	// Drawing area
	GFX_DrawRectangle(DRAW_WINDOW_X_START, DRAW_WINDOW_Y_START, DRAW_WINDOW_X_STOP, DRAW_WINDOW_Y_STOP, ILI9341_BLACK);
	// Current color indicator
	ColorIndicator();
	// Clear button
	ClearButton();
	// Color buttons
	ColorButtons();

	State = PAINT_DRAW;
}

//
// PAINT_DRAW state function
//
void DrawScreen(void)
{
	// Check if screen was touched
	if(XPT2046_IsTouched())
	{
		uint16_t x, y; // Touch points
		uint8_t ColorButtonNubmer; // Color number touched for checking that

		XPT2046_GetTouchPoint(&x, &y); // Get the current couched point

		// Check if that point is inside the drawing area
		if((x > DRAW_WINDOW_X_START)&&(x < DRAW_WINDOW_X_STOP)&&(y > DRAW_WINDOW_Y_START)&&(y < DRAW_WINDOW_Y_STOP))
		{
			// If yes - just draw a pixel there with current selected color
			GFX_DrawPixel(x, y, CurrentColor);
		}

		// Check if any Color change button was touched
		ColorButtonNubmer = IsColorButtonTouched(x, y);
		if(ColorButtonNubmer != 0) // If yes
		{
			// Chenge current color
			CurrentColor = UsedColors[ColorButtonNubmer-1];
			// Redrwa current color indicator
			ColorIndicator();
		}

		// Check if any Clearing button was touched
		if(IsClearButtonTouched(x, y))
		{
			// Jump to Clearing state
			State = PAINT_CLEAR;
		}
	}
}

//
// PAINT_CLEAR state function
//
void DrawClear(void)
{
	// Clear whole drawing area
	GFX_DrawFillRectangle(0, 30, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT-60, ILI9341_WHITE);
	//ILI9341_ClearArea(0, 30, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT-60, ILI9341_WHITE);

	// Redraw frame for the drawing area
	GFX_DrawRectangle(0, 30, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT-60, ILI9341_BLACK);
	// Go back to Drawing state
	State = PAINT_DRAW;
}

void Paint(void)
{
	switch(State)
	{
	case PAINT_INIT:
		InitScreen();
		break;
	case PAINT_DRAW:
		DrawScreen();
		  break;
	case PAINT_CLEAR:
		DrawClear();
		break;
	}
}
