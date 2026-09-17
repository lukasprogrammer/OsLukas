#include "window.h"
#include "graphics.h"
#include "../mouse.h"
#include "../kernel.h"
#include "fbterminal.h"


#define MAX_WINDOWS 30

Window windows_total[MAX_WINDOWS];
int window_full_redraw_needed = 0;
int window_list_pos = 0;
static int next_window_id = 1;


void DrawWindow(Window win)
{
    int close_x = win.x + win.width - 18;
    int close_y = win.y + 2;

    DrawRect(
        win.x,
        win.y,
        win.width,
        win.height,
        0x00D3D3D3
    );

    DrawRect(
        win.x,
        win.y,
        win.width,
        20,
        0x005B5B5B
    );

    DrawString(
        win.x + 6,
        win.y + 6,
        win.title,
        0x00000000
    );

    //BUTTON

    DrawRect(close_x, close_y, 16, 16, 0x00FF0000);
    DrawString(close_x + 4, close_y + 3, "X", 0x00000000);

    //BUTTON


    DrawRectOutline(
        win.x,
        win.y,
        win.width,
        win.height,
        0x00444444
    );
}


void PresentWindow(Window win)
{
    PresentRect(
        win.x,
        win.y,
        win.width,
        win.height
    );
}


int AddWindow(
    int x,
    int y,
    int width,
    int height,
    const char *title
)
{
    if(window_list_pos >= MAX_WINDOWS){
        return 0;
    }

    Window addWin;

    addWin.x = x;
    addWin.y = y;

    addWin.width = width;
    addWin.height = height;

    addWin.title = title;

    addWin.dragging = 0;

    addWin.last_mouse_x = 0;
    addWin.last_mouse_y = 0;

    addWin.old_x = x;
    addWin.old_y = y;

    addWin.dirty_x = x;
    addWin.dirty_y = y;
    addWin.dirty_width = width;
    addWin.dirty_height = height;
    addWin.id = next_window_id++;

    windows_total[window_list_pos] = addWin;

    window_list_pos++;
    return addWin.id;
}

Window *FindWindowById(int id){
    for(int i = 0; i < window_list_pos; i++){
        if(id == windows_total[i].id){
            return &windows_total[i];
        }
    }
    return 0;
}

void DrawAllWindows(void)
{
    ClearClipRect();    
    for(int i = 0; i < window_list_pos; i++)
    {

        ClearClipRect();
        DrawWindow(windows_total[i]);

        

        SetClipRect(
            windows_total[i].x,
            windows_total[i].y + 20,
            windows_total[i].width,
            windows_total[i].height - 20
        );

        DrawLabelsForWindow(windows_total[i].id);
        DrawButtonsForWindow(windows_total[i].id);

        ClearClipRect();
    }
}


void PresentWindows(void)
{
    for(int i = 0; i < window_list_pos; i++){
        PresentWindow(windows_total[i]);
    }
}


int MouseInsideTitleBar(Window win)
{
    if(
        mouse_x >= win.x &&
        mouse_x < win.x + win.width &&
        mouse_y >= win.y &&
        mouse_y < win.y + 20
    ){
        return 1;
    }

    return 0;
}


int UpdateWin(Window *win, int allow_drag_start)
{
    int changed = 0;



    if(MouseInsideTitleBar(*win) && just_pressed && allow_drag_start)
    {
        if(win->dragging == 0){
            changed = 1;
        }

        win->last_mouse_x = mouse_x;
        win->last_mouse_y = mouse_y;

        win->dragging = 1;
    }


    if(just_released && win->dragging)
    {
        win->dragging = 0;
        changed = 1;
    }


    if(win->dragging)
    {
        int move_x =
            mouse_x - win->last_mouse_x;

        int move_y =
            mouse_y - win->last_mouse_y;


        if(move_x != 0 || move_y != 0)
        {
            win->old_x = win->x;
            win->old_y = win->y;


            win->x += move_x;
            win->y += move_y;


            win->last_mouse_x = mouse_x;
            win->last_mouse_y = mouse_y;

            int old_right =
                win->old_x + win->width;

            int new_right =
                win->x + win->width;

            int old_bottom =
                win->old_y + win->height;

            int new_bottom =
                win->y + win->height;


            win->dirty_x =
                win->old_x < win->x
                ? win->old_x
                : win->x;


            win->dirty_y =
                win->old_y < win->y
                ? win->old_y
                : win->y;


            int dirty_right =
                old_right > new_right
                ? old_right
                : new_right;


            int dirty_bottom =
                old_bottom > new_bottom
                ? old_bottom
                : new_bottom;


            win->dirty_width =
                dirty_right - win->dirty_x;

            win->dirty_height =
                dirty_bottom - win->dirty_y;


            changed = 1;
        }
    }


    return changed;
}


int UpdateAllWindows(void)
{
    int changed = 0;

    for(int i = 0; i < window_list_pos; i++)
    {


        int allow_drag_start = (i == window_list_pos - 1);
        if(just_pressed && allow_drag_start){
            if(MouseInsideCloseButton(windows_total[i])){
                RemoveButtonsForWindow(windows_total[i].id);
                RemoveLabelsForWindow(windows_total[i].id);
                CloseWindow(i);
                
                window_full_redraw_needed = 1;
                return 1;
            }
        }
        if(UpdateWin(&windows_total[i], allow_drag_start)){
            changed = 1;
        }
    }

    return changed;
}


void PresentDirtyWindow(Window *win)
{
    PresentRect(
        win->dirty_x,
        win->dirty_y,
        win->dirty_width,
        win->dirty_height
    );
}


void PresentDirtyWindows(void)
{
    for(int i = 0; i < window_list_pos; i++)
    {
        if(windows_total[i].dirty_width > 0 &&
           windows_total[i].dirty_height > 0)
        {
            PresentDirtyWindow(&windows_total[i]);
        }
    }
}

void BringWindowToFront(int index){
    if(index < 0 || index >= window_list_pos){
        return;
    }

    Window frontWindow = windows_total[index];
    for(int i = index + 1; i < window_list_pos; i++){
        windows_total[i - 1] = windows_total[i]; 
    }
    windows_total[window_list_pos - 1] = frontWindow;
}
int MouseInsideWindow(Window win){
    if(mouse_x >= win.x && mouse_x < win.x + win.width && mouse_y >= win.y && mouse_y < win.y + win.height){
        return 1;
    }
    return 0;
}
int FocusWindow(void){
    for(int i = window_list_pos - 1; i >= 0; i--){
        if(MouseInsideWindow(windows_total[i])){
            BringWindowToFront(i);
            return 1;
        }
    }
    return 0;
}

void CloseWindow(int index){
    if(index < 0 || index >= window_list_pos){
        return;
    }
    for(int i = index + 1; i < window_list_pos; i++){
        windows_total[i - 1] = windows_total[i];
    }
    window_list_pos--;
}

int MouseInsideCloseButton(Window win){
    if(mouse_x >= win.x + win.width - 18 && mouse_x < win.x + win.width - 2 && mouse_y >= win.y + 2 && mouse_y < win.y + 18){
        return 1;
    }
    return 0;
}

int RectsIntersect(
    int x1, int y1, int w1, int h1,
    int x2, int y2, int w2, int h2
)
{
    if(x1 >= x2 + w2 ||
       y1 >= y2 + h2 ||
       x1 + w1 <= x2 ||
       y1 + h1 <= y2)
    {
        return 0;
    }

    return 1;
}

void DrawTerminalRect(int x, int y, int width, int height)
{
    if(width <= 0 || height <= 0)
        return;

    // Clip dirty rectangle to screen
    if(x < 0)
    {
        width += x;
        x = 0;
    }

    if(y < 0)
    {
        height += y;
        y = 0;
    }

    if(x >= framebuffer_width || y >= framebuffer_height)
        return;

    if(x + width > framebuffer_width)
        width = framebuffer_width - x;

    if(y + height > framebuffer_height)
        height = framebuffer_height - y;

    if(width <= 0 || height <= 0)
        return;

    DrawRect(x, y, width, height, bg_color_sys);


    int start_col = x / TERMINAL_CELL_WIDTH;
    int start_row = y / TERMINAL_CELL_HEIGHT;

    int end_col =
        (x + width - 1) / TERMINAL_CELL_WIDTH;

    int end_row =
        (y + height - 1) / TERMINAL_CELL_HEIGHT;


    // Clamp to terminal grid
    if(start_col < 0)
        start_col = 0;

    if(start_row < 0)
        start_row = 0;

    if(end_col >= terminal_cols)
        end_col = terminal_cols - 1;

    if(end_row >= terminal_rows)
        end_row = terminal_rows - 1;


    for(int row = start_row; row <= end_row; row++)
    {
        for(int col = start_col; col <= end_col; col++)
        {
            TerminalCell *cell =
                GetTerminalCell(col, row);

            if(cell == 0)
                continue;

            int cell_x =
                col * TERMINAL_CELL_WIDTH;

            int cell_y =
                row * TERMINAL_CELL_HEIGHT;

            DrawRect(
                cell_x,
                cell_y,
                TERMINAL_CELL_WIDTH,
                TERMINAL_CELL_HEIGHT,
                cell->bg_color
            );

            if(cell->character != ' ')
            {
                DrawChar(
                    cell_x,
                    cell_y,
                    cell->character,
                    cell->fg_color
                );
            }
        }
    }
}

void RedrawRect(int x, int y, int width, int height)
{
    if(width <= 0 || height <= 0)
        return;

    SetClipRect(x, y, width, height);

    DrawTerminalRect(x, y, width, height);

    for(int i = 0; i < window_list_pos; i++)
    {
        Window *win = &windows_total[i];

        if(RectsIntersect(
            x, y, width, height,
            win->x, win->y,
            win->width, win->height))
        {
            DrawWindowWithWidgets(win);
        }
    }

    DrawGlobalButtons();
    DrawGlobalLabels();

    ClearClipRect();

    PresentRect(x, y, width, height);
}

void DrawWindowWithWidgets(Window *win)
{
    DrawWindow(*win);

    SetClipRect(
        win->x,
        win->y + 20,
        win->width,
        win->height - 20
    );

    DrawLabelsForWindow(win->id);
    DrawButtonsForWindow(win->id);

    ClearClipRect();
}
