#include "graphics.h"
#include "../kernel.h"
#include "font.h"
#include "kernel.h"
#include "../io.h"
#include "../mouse.h"
#include "window.h"
#include "kernel.h"
#include "graphics/fbterminal.h"

#define MAX_BUTTONS 10
#define MAX_LABELS 10

unsigned char *backbuffer;
static unsigned int cursor_background[25];
unsigned int buttonlist_pos = 0;

static int clip_enabled = 0;
static int clip_x;
static int clip_y;
static int clip_width;
static int clip_height;


Button total_buttons[MAX_BUTTONS];
Label total_labels[MAX_LABELS];

int label_list_pos = 0;

void SetClipRect(int x, int y, int width, int height)
{
    clip_x = x;
    clip_y = y;
    clip_width = width;
    clip_height = height;
    clip_enabled = 1;
}

void ClearClipRect(void)
{
    clip_enabled = 0;
}

void PutPixel(int x, int y, unsigned int color){
    if(x >= framebuffer_width || y >= framebuffer_height || x < 0 || y < 0){
        return;
    }

    if(clip_enabled)
    {
        if(x < clip_x ||
        x >= clip_x + clip_width ||
        y < clip_y ||
        y >= clip_y + clip_height)
        {
            return;
        }
    }
    unsigned int offset = y * framebuffer_pitch + x * 4;

    unsigned int *pixel = (unsigned int *)(backbuffer + offset);
    *pixel = color;
}

unsigned int GetPixel(int x, int y){
    if(x >= framebuffer_width || y >= framebuffer_height || x < 0 || y < 0){
        return 0;
    }
    unsigned int offset = y * framebuffer_pitch + x * 4;

    
    unsigned int *pixel = (unsigned int *)(backbuffer + offset);
    return *pixel;

}   

void DrawRect(int x, int y, int width, int height, unsigned int color){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            PutPixel(x + j, y + i, color);
        }
    }
}

void DrawLine(int x1, int y1, int x2, int y2, unsigned int color){
    int dx = x2 - x1;
    int dy = y2 - y1;

    int sx;
    int sy;

    if(dx > 0){
        sx = 1;
    }else if(dx < 0){
        sx = -1;
    }else{
        sx = 0;
    }

    if(dy > 0){
        sy = 1;
    }else if(dy < 0){
        sy = -1;
    }else{
        sy = 0;
    }

    dx *= sx;
    dy *= sy;

    int x = x1;
    int y = y1;
    int err = dx - dy;

    while(1){
        PutPixel(x, y, color);

        if(x == x2 && y == y2){
            break;
        }

        int e2 = 2 * err;
        if(e2 > -dy){
            err -= dy;
            x += sx;
        }

        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }


}

void DrawChar(int x, int y, char c, unsigned int color){
    for(int row = 0; row < 8; row++){
        unsigned char rowData = font[(unsigned char)c][row];
        for(int col = 0; col < 8; col++){
            
            if((rowData)&(0x80 >> col)){
                PutPixel(x + col, y + row, color);
            }
        }
    }
}


void DrawString(int x, int y, const char *text, unsigned int color){
    int z = 0;
    int currentx = x;
    int currenty = y;

    while(text[z] != '\0'){
        if(text[z] == '\n' || currentx + 8 >= framebuffer_width){
            currentx = x;
            currenty += 10;
        }else{
            DrawChar(currentx, currenty, text[z], color);
            currentx += 9;
        }



        z++;
    }
}


void PresentFrame(){
    unsigned int *src = (unsigned int *)backbuffer;
    unsigned int *dst = (unsigned int *)framebuffer;

    unsigned int count = (framebuffer_size) / 4;

    for(unsigned int i = 0; i < count; i++)
    {
        dst[i] = src[i];
    }
}

void PresentRect(int x, int y, int width, int height)
{
    if(width <= 0 || height <= 0){
        return;
    }

    if(x >= framebuffer_width || y >= framebuffer_height){
        return;
    }

    if(x + width <= 0 || y + height <= 0){
        return;
    }

    if(x < 0){
        width += x;
        x = 0;
    }

    if(y < 0){
        height += y;
        y = 0;
    }

    if(x + width > framebuffer_width)
        width = framebuffer_width - x;

    if(y + height > framebuffer_height)
        height = framebuffer_height - y;

    for(int row = 0; row < height; row++)
    {
        unsigned int offset =
            (y + row) * framebuffer_pitch + x * 4;

        unsigned int *src =
            (unsigned int *)(backbuffer + offset);

        unsigned int *dst =
            (unsigned int *)(framebuffer + offset);

        for(int col = 0; col < width; col++)
        {
            dst[col] = src[col];
        }
    }
}
void PutPixelDirect(int x, int y, unsigned int color)
{
    if(x < 0 || y < 0 ||
       x >= framebuffer_width ||
       y >= framebuffer_height)
        return;

    unsigned int offset =
        y * framebuffer_pitch + x * 4;

    unsigned int *pixel =
        (unsigned int *)(framebuffer + offset);

    *pixel = color;
}

void DrawMouseCursor(int x, int y)
{
    for(int cy = 0; cy < 5; cy++)
    {
        for(int cx = 0; cx < 5; cx++)
        {
            PutPixel(x + cx, y + cy, 0x00FFFFFF);
        }
    }
}
void SaveMouseBackground(int x, int y)
{
    for(int cy = 0; cy < 5; cy++)
    {
        for(int cx = 0; cx < 5; cx++)
        {
            cursor_background[cy * 5 + cx] =
                GetPixel(x + cx, y + cy);
        }
    }
}
void RestoreMouseBackground(int x, int y)
{
    for(int cy = 0; cy < 5; cy++)
    {
        for(int cx = 0; cx < 5; cx++)
        {
            PutPixel(
                x + cx,
                y + cy,
                cursor_background[cy * 5 + cx]
            );
        }
    }
}

void DrawMouseCursorDirect(int x, int y)
{
    for(int cy = 0; cy < 5; cy++)
    {
        for(int cx = 0; cx < 5; cx++)
        {
            PutPixelDirect(x + cx, y + cy, 0x00FFFFFF);
        }
    }
}

void WaitForVSync(void)
{
    /* Wait until current vertical retrace has finished */
    while(inb(0x3DA) & 0x08)
    {
    }

    /* Wait until next vertical retrace begins */
    while(!(inb(0x3DA) & 0x08))
    {
    }
}
void DrawRectOutline(int x, int y, int width, int height, unsigned int color){
    DrawLine(x, y, x+width-1, y,color);
    DrawLine(x, y, x, y+height-1,color);
    DrawLine(x, y+height-1, x+width-1, y+height-1,color);
    DrawLine(x+width-1, y, x+width-1, y+height-1,color);
}

int StringLength(const char *txt){
    int x = 0;
    while(txt[x] != 0){
        x++;
    }
    return x;
}

void DrawButton(Button btn){
    int screen_x = btn.x;
    int screen_y = btn.y;
    unsigned int color;

    if(btn.owner_window_id != 0)
    {
        Window *owner = FindWindowById(btn.owner_window_id);
        if(owner != 0)
        {
            screen_x = owner->x + btn.x;
            screen_y = owner->y + btn.y + 20;
        }
    }



    if(btn.hovered){
        if(btn.clicked){
            color = DarkenColor(btn.color, 30);
        }else{
            color = LightenColor(btn.color, 25);
        }
    }else{
        color = btn.color;
    }
    DrawRect(screen_x, screen_y, btn.width, btn.height, color);
    int stringlength = StringLength(btn.text);
    int pixelLength = (stringlength * 9) -1;
    DrawString(screen_x + (btn.width/2) - (pixelLength/2),screen_y + (btn.height/2) - 4, btn.text, 0x00000000);
    DrawRectOutline(screen_x, screen_y, btn.width, btn.height, 0x00FFFFFF);


}

unsigned int LightenColor(unsigned int color, unsigned char amount){
    unsigned int r = (color >> 16) & 0xFF;
    unsigned int g = (color >> 8)  & 0xFF;
    unsigned int b = color & 0xFF;

    r += amount;
    g += amount;
    b += amount;

    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;

    return (r << 16) | (g << 8) | b;
}
unsigned int DarkenColor(unsigned int color, unsigned char amount){
    unsigned int r = (color >> 16) & 0xFF;
    unsigned int g = (color >> 8)  & 0xFF;
    unsigned int b = color & 0xFF;


    if(r > amount) r -= amount;
    else r = 0;

    if(g > amount) g -= amount;
    else g = 0;

    if(b > amount) b -= amount;
    else b = 0;

    return (r << 16) | (g << 8) | b;
}
int UpdateButton(Button *btn){

    int screen_x = btn->x;
    int screen_y = btn->y;

    if(btn->owner_window_id != 0)
    {
        Window *owner = FindWindowById(btn->owner_window_id);
        if(owner != 0)
        {
            screen_x = owner->x + btn->x;
            screen_y = owner->y + btn->y+20;
        }
    }

    if(mouse_x >= screen_x && mouse_x < screen_x + btn->width && mouse_y >= screen_y && mouse_y < screen_y + btn->height){
        if(btn->hovered == 1){
            return 0;
        }
        btn->hovered = 1;
        DrawButton(*btn);
        return 1;
        
    }else{
        if(btn->hovered == 0){
            return 0;
        }
        btn->hovered = 0;
        DrawButton(*btn);
        return 1;
    }
    
}
void ButtonAdd(int x, int y, int width, int height, unsigned int color, const char *text, void (*action)(void), int owner_window_id){
    if(buttonlist_pos < MAX_BUTTONS){
        Button addButton;
        addButton.x = x;
        addButton.y = y;
        addButton.width = width;
        addButton.height = height;
        addButton.color = color;
        addButton.text = text;
        addButton.hovered = 0;
        addButton.clicked = 0;
        addButton.action = action;
        addButton.press_started_inside = 0;
        addButton.owner_window_id = owner_window_id;

        total_buttons[buttonlist_pos] = addButton;
        buttonlist_pos++;

        DrawButton(addButton);

        
    }


}

int ButtonClickDetect(Button *btn, int just_pressed, int just_released)
{
    if(just_pressed && btn->hovered){
        btn->press_started_inside = 1;
        btn->clicked = 1;
        DrawButton(*btn);
        return 1;
    }

    if(just_released){
        int should_activate = btn->press_started_inside && btn->hovered;

        btn->press_started_inside = 0;
        btn->clicked = 0;
        DrawButton(*btn);

        if(should_activate && btn->action != 0){
            btn->action();
        }

        return 1;
    }

    if(btn->press_started_inside){
        int new_clicked = btn->hovered && (mouse_buttons & 0x01);

        if(btn->clicked != new_clicked){
            btn->clicked = new_clicked;
            DrawButton(*btn);
            return 1;
        }
    }

    return 0;
}
int UpdateTotalButtons()
{
    int changed = 0;

    int left_down = mouse_buttons & 0x01;
    int just_pressed = 0;
    int just_released = 0;

    if(!previous_left_down && left_down){
        just_pressed = 1;
    }else if(previous_left_down && !left_down){
        just_released = 1;
    }

    for(int i = 0; i < buttonlist_pos; i++){
        if(UpdateButton(&total_buttons[i])){
            changed = 1;
        }

        if(ButtonClickDetect(&total_buttons[i], just_pressed, just_released)){
            changed = 1;
        }
    }

    previous_left_down = left_down;

    return changed;
}
void PresentButtons(void)
{
    for(int i = 0; i < buttonlist_pos; i++)
    {
        Button *btn = &total_buttons[i];

        int screen_x = btn->x;
        int screen_y = btn->y;

        if(btn->owner_window_id != 0)
        {
            Window *owner =
                FindWindowById(btn->owner_window_id);

            if(owner != 0)
            {
                screen_x = owner->x + btn->x;
                screen_y = owner->y + btn->y + 20;
            }
        }

        PresentRect(
            screen_x,
            screen_y,
            btn->width,
            btn->height
        );
    }
}
void DrawAllButtons(){
    for(int i = 0; i < buttonlist_pos; i++){
        DrawButton(total_buttons[i]);
    }
}

void RemoveButtonsForWindow(int window_id){
    for(int i = 0; i < buttonlist_pos;){
        if(total_buttons[i].owner_window_id == window_id){
            for(int j = i + 1; j < buttonlist_pos; j++){
                total_buttons[j - 1] = total_buttons[j];
            }
            buttonlist_pos--;
        }else{
            i++;
        }
    }
}

void AddLabel(int x, int y, const char *text, unsigned int color, int owner_id){
    if(label_list_pos >= MAX_LABELS){
        return;
    }
    Label addLabel;
    addLabel.x = x;
    addLabel.y = y;
    addLabel.text = text;
    addLabel.color = color;
    addLabel.owner_window_id = owner_id;
    total_labels[label_list_pos] = addLabel;
    label_list_pos++;

}

void DrawLabel(Label lbl){
    int screen_x = lbl.x;
    int screen_y = lbl.y;

    if(lbl.owner_window_id != 0)
    {
        Window *owner = FindWindowById(lbl.owner_window_id);
        if(owner != 0)
        {
            screen_x = owner->x + lbl.x;
            screen_y = owner->y + lbl.y + 20;
        }
    }

    int padding_x = 6;
    int padding_y = 2;

    
    int timesEnter = 1;
    int Longestline = 0;
    int currentLineIndex = 0;
    for(int i = 0; i < StringLength(lbl.text); i++){
    
        if(lbl.text[i] == '\n'){
            if(currentLineIndex > Longestline){
                Longestline = currentLineIndex;
            }
            timesEnter++;
            currentLineIndex = 0;
        }else{
            currentLineIndex++;
        }
    }
    if(currentLineIndex > Longestline)
    {
        Longestline = currentLineIndex;
    }
    int pixelLengthY = timesEnter * TERMINAL_CELL_HEIGHT;
    int pixelLengthX = Longestline* TERMINAL_CELL_WIDTH;

    DrawRect(screen_x, screen_y, pixelLengthX + 2*padding_x, pixelLengthY + 2*padding_y, lbl.color);
    DrawString(screen_x + padding_x, screen_y + padding_y, lbl.text, 0x00000000);
}
void DrawAllLabels(void)
{
    for(int i = 0; i < label_list_pos; i++)
    {
        DrawLabel(total_labels[i]);
    }
}
void RemoveLabelsForWindow(int window_id){
    for(int i = 0; i < label_list_pos;){
        if(total_labels[i].owner_window_id == window_id){
            for(int j = i + 1; j < label_list_pos; j++){
                total_labels[j - 1] = total_labels[j];
            }
            label_list_pos--;
        }else{
            i++;
        }
    }
}
void DrawButtonsForWindow(int window_id){
    for(int i = 0; i < buttonlist_pos; i++){
        if(total_buttons[i].owner_window_id == window_id){
            DrawButton(total_buttons[i]);
        }
    }
}
void DrawLabelsForWindow(int window_id){
    for(int i = 0; i < label_list_pos; i++){
        if(total_labels[i].owner_window_id == window_id){
            DrawLabel(total_labels[i]);
        }
    }
}
void DrawGlobalButtons(void)
{
    for(int i = 0; i < buttonlist_pos; i++)
    {
        if(total_buttons[i].owner_window_id == 0)
            DrawButton(total_buttons[i]);
    }
}
void DrawGlobalLabels(void)
{
    for(int i = 0; i < label_list_pos; i++)
    {
        if(total_labels[i].owner_window_id == 0)
            DrawLabel(total_labels[i]);
    }
}