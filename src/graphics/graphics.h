#ifndef GRAPHICS_H
#define GRAPHICS_H

extern unsigned char *framebuffer;
extern unsigned char *backbuffer;
extern unsigned short framebuffer_pitch;
extern unsigned short framebuffer_width;
extern unsigned short framebuffer_height;
extern unsigned char framebuffer_bpp;



typedef struct
{
    int x;
    int y;
    int width;
    int height;
    unsigned int color;
    int hovered;
    int clicked;
    void (*action)(void);
    int press_started_inside;

    const char *text;
    int owner_window_id;
} Button;

typedef struct
{
    int x;
    int y;
    const char *text;
    unsigned int color;
    int owner_window_id;
} Label;
void PutPixel(int x, int y, unsigned int color);
void DrawRect(int x, int y, int width, int height, unsigned int color);
void DrawLine(int x1, int y1, int x2, int y2, unsigned int color);


void DrawMouseCursorDirect(int x, int y);
void PutPixelDirect(int x, int y, unsigned int color);
void DrawChar(int x, int y, char c, unsigned int color);
void DrawString(int x, int y, const char *text, unsigned int color);
int StringLength(const char *txt);
unsigned int GetPixel(int x, int y);
void PresentFrame();
void PresentRect(int x, int y, int width, int height);
void PutPixelDirect(int x, int y, unsigned int color);
void DrawMouseCursor(int x, int y);


void SaveMouseBackground(int x, int y);
void RestoreMouseBackground(int x, int y);
void WaitForVSync(void);
void DrawRectOutline(int x, int y, int width, int height, unsigned int color);

void DrawButton(Button btn);
void ButtonAdd(int x, int y, int width, int height, unsigned int color, const char *text, void (*action)(void), int owner_window_id);
int UpdateButton(Button *btn);
int ButtonClickDetect(Button *btn, int just_pressed, int just_released);
int UpdateTotalButtons();
void PresentButtons();

void DrawAllButtons();
unsigned int LightenColor(unsigned int color, unsigned char amount);
unsigned int DarkenColor(unsigned int color, unsigned char amount);
void SetClipRect(int x, int y, int width, int height);
void ClearClipRect(void);

void RemoveButtonsForWindow(int window_id);

void AddLabel(int x, int y, const char *text, unsigned int color, int owner_id);
void DrawLabel(Label lbl);
void DrawAllLabels(void);
void RemoveLabelsForWindow(int window_id);

void DrawButtonsForWindow(int window_id);
void DrawLabelsForWindow(int window_id);

void DrawGlobalButtons(void);
void DrawGlobalLabels(void);
#endif