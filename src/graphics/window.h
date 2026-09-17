#ifndef WINDOW_H
#define WINDOW_H

typedef struct{
    int x;
    int y;
    int width;
    int height;
    const char *title;
    int dragging;
    int last_mouse_x;
    int last_mouse_y;
    int old_x;
    int old_y;

    int dirty_x;
    int dirty_y;
    int dirty_width;
    int dirty_height;
    int id;
}Window;

extern int window_full_redraw_needed;
extern int window_list_pos;
extern Window windows_total[];

void DrawWindow(Window win);
void PresentWindow(Window win);
int AddWindow(int x, int y, int width, int height, const char *title);
void DrawAllWindows();
int UpdateAllWindows();
int UpdateWin(Window *win, int allow_drag_start);
int MouseInsideTitleBar(Window win);
void PresentWindows();
void PresentDirtyWindow(Window *win);
void PresentDirtyWindows(void);
int FocusWindow(void);
int MouseInsideWindow(Window win);
void BringWindowToFront(int index);
void CloseWindow(int index);
int MouseInsideCloseButton(Window win);

Window *FindWindowById(int id);
int RectsIntersect(
    int x1, int y1, int w1, int h1,
    int x2, int y2, int w2, int h2
);


void DrawWindowWithWidgets(Window *win);

void RedrawRect(int x, int y, int width, int height);
#endif