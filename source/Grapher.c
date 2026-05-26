#include "Grapher.h"
#include "ttf_render.h"


#define SCREEN_X 1920
#define SCREEN_Y 1080

#define WINDOW_X 800
#define WINDOW_Y 500

#define AXIS_START_OFFSET_X 100
#define AXIS_START_OFFSET_Y 100

#define AXIS_END_OFFSET_X 100
#define AXIS_END_OFFSET_Y 100

#define MAX_FLOAT_CHAR_LEN 10
#define AXIS_LABEL_COUNT 2;

typedef struct Dot Dot;
typedef struct Label Label;

struct Dot{
    f32 val_x;
    f32 val_y;
    Dot* prev_dot;
};

struct Label{
    u32 pos_x;
    u32 pos_y;
    u32 color;
    char* text;
};


static Dot* dotBuffer;
static Dot* prevDot;
static Dot* curDot;
static Dot* lastDot;

static Label graphLabelX;
static Label graphLabelY;

static Label* webLabelsX;
static Label* webLabelsY;
static Label* curWebLabelX;

static f32 curX;
static f32 stepX;
static f32 accumStepWebLabelX;
static u8 isShifting;
static u8 isLabel;

static u32 dotsCount;
static u32 labelWebCountX;
static u32 labelWebCountY;

static f32 minVal_x;
static f32 maxVal_x;

static f32 minVal_y;
static f32 maxVal_y;

static void addFloatLabel(Label* label, f32 num)
{
	char text[MAX_FLOAT_CHAR_LEN];
    snprintf(text, sizeof(text), "%2f", num);
    strncpy(label->text, text, MAX_FLOAT_CHAR_LEN);
}


static void DrawDot(f32 x, f32 y, f32 dot_size, u32 color)
{
	float x1 = x - dot_size/2;
	float y1 = y - dot_size/2;

	float x2 = x1;
	float y2 = y + dot_size/2;

	float x3 = x + dot_size/2;
	float y3 = y2;

	float x4 = x3;
	float y4 = y - dot_size/2;

	tiny3d_SetPolygon(TINY3D_QUADS);
	tiny3d_VertexPos(x1, y1, 0.0f);
    tiny3d_VertexColor(color);
	tiny3d_VertexPos(x2, y2, 0.0f);
	tiny3d_VertexColor(color);
	tiny3d_VertexPos(x3, y3, 0.0f);
	tiny3d_VertexColor(color);
	tiny3d_VertexPos(x4, y4, 0.0f);
	tiny3d_VertexColor(color);

	tiny3d_End();
}
static void DrawLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 dot_size, u32 color)
{
	DrawDot(x1, y1, dot_size, color);
	DrawDot(x2, y2, dot_size, color);

    tiny3d_SetPolygon(TINY3D_LINES);
    
    tiny3d_VertexPos(x1, y1, 0.0f);
    tiny3d_VertexColor(color);
    
    tiny3d_VertexPos(x2, y2, 0.0f);
	tiny3d_VertexColor(color);
    // цвет можно не повторять, он сохраняется
    
    tiny3d_End();
}
static f32 getPosX(f32 val_x)
{
    f32 gr_offset = (val_x - minVal_x) * (WINDOW_X - AXIS_START_OFFSET_X - AXIS_END_OFFSET_X) / (maxVal_x - minVal_x);
    return AXIS_START_OFFSET_X + gr_offset;
}
static f32 getPosY(f32 val_y)
{
    f32 gr_offset = (val_y - minVal_y) * (WINDOW_Y - AXIS_START_OFFSET_Y - AXIS_END_OFFSET_Y) / (maxVal_y - minVal_y);
    return WINDOW_Y - AXIS_END_OFFSET_Y - gr_offset;
}
static f32 getNextX()
{
    curX += stepX;
    if(isShifting) maxVal_x = curX;
    return curX;
}
static Dot* getNextDot(Dot* ptr, u8 isCur) ///check !!!!
{
    ++ptr;
    if(ptr - dotBuffer >= dotsCount){
        isShifting = isShifting || (1 && isCur);
        ptr = dotBuffer;
    }
    return ptr;
}

static void shiftLabelX()
{
    ++curWebLabelX;
    if(curWebLabelX - webLabelsX >= labelWebCountX){
        curWebLabelX = webLabelsX;
    }
    curWebLabelX->pos_x = getPosX(maxVal_x);
    addFloatLabel(curWebLabelX, maxVal_x);
}

static void drawGraphic(f32 dot_size, u32 color)
{
    for(Dot* ptr = dotBuffer; ptr - dotBuffer < dotsCount; ++ptr)
    {
        Dot* prev = ptr->prev_dot;
        
        if(!prev || ptr == lastDot) continue;

        f32 x1 = getPosX(prev->val_x);
        f32 y1 = getPosY(prev->val_y);

        f32 x2 = getPosX(ptr->val_x);
        f32 y2 = getPosY(ptr->val_y);

        if(isShifting){
            f32 posStepX = (stepX) * (WINDOW_X - AXIS_START_OFFSET_X - AXIS_END_OFFSET_X) / (maxVal_x - minVal_x);
            x1 -= posStepX;
            x2 -= posStepX;
        }

        if(curDot == ptr)
        {
            DrawLine(x1, y1, x2, y2, dot_size, RGBA(0,255,0,255));
        }
        else{
            DrawLine(x1, y1, x2, y2, dot_size, color);
        }
    }
}

static void drawLabels()
{
    display_ttf_string(graphLabelX.pos_x, graphLabelX.pos_y, graphLabelX.text, graphLabelX.color, 0, 15, 15);
    display_ttf_string(graphLabelY.pos_x, graphLabelY.pos_y, graphLabelY.text, graphLabelY.color, 0, 15, 15);
    for(u32 i = 0; i < labelWebCountY; ++i)
    {
        display_ttf_string(webLabelsY[i].pos_x, webLabelsY[i].pos_y, webLabelsY[i].text, webLabelsY[i].color, 0, 10, 10);
    }

    f32 h_x = (maxVal_x - minVal_x) / labelWebCountX;
    f32 val_x = minVal_x;
    for(u32 i = 0; i < labelWebCountX; ++i)
    {
        //webLabelsX[i].pos_x = getPosX(val_x);
        addFloatLabel((webLabelsX + i), val_x);

        display_ttf_string(webLabelsX[i].pos_x, webLabelsX[i].pos_y, webLabelsX[i].text, webLabelsX[i].color, 0, 10, 10);
        val_x += h_x;
    }
}


void Grapher_Init(f32 start_x, f32 end_x, f32 min_y, f32 max_y, u32 dots_count)
{
    dotBuffer = (Dot*)malloc(dots_count * sizeof(Dot));
    for(u32 i = 0; i < dots_count; ++i)
    {
        dotBuffer[i].val_x = start_x;
        dotBuffer[i].val_y = 0;
        dotBuffer[i].prev_dot = 0;
    }
    dotsCount = dots_count;
    curDot = dotBuffer + 1;
    prevDot = dotBuffer;
    lastDot = dotBuffer;
    isLabel = 0;

    curX = start_x;
    stepX = (end_x - start_x)/ dots_count;
    isShifting = 0;

    minVal_x = start_x;
    maxVal_x = end_x;

    minVal_y = min_y;
    maxVal_y = max_y;
}

void Grapher_set_labels(const char* axis_x_name, u8 x_name_len, 
                        const char* axis_y_name, u8 y_name_len, 
                        u32 count_x, u32 count_y)
{
    labelWebCountX = count_x;
    labelWebCountY = count_y;

    webLabelsX = (Label*)malloc(labelWebCountX * sizeof(Label));
    webLabelsY = (Label*)malloc(labelWebCountY * sizeof(Label));

    

    graphLabelX.pos_x = WINDOW_X -  AXIS_END_OFFSET_X;
    graphLabelX.pos_y = WINDOW_Y - (AXIS_END_OFFSET_Y/2);
    graphLabelX.color = RGBA(255,255,255,255);
    graphLabelX.text = (char*)malloc(x_name_len);

    graphLabelY.pos_x =(AXIS_START_OFFSET_X/2);
    graphLabelY.pos_y = AXIS_START_OFFSET_Y;
    graphLabelY.color = RGBA(255,255,255,255);
    graphLabelY.text = (char*)malloc(y_name_len);

    strncpy(graphLabelX.text, axis_x_name, x_name_len);
    strncpy(graphLabelY.text, axis_y_name, y_name_len);

    f32 h_y = (maxVal_y - minVal_y) / labelWebCountY;
    f32 val_y = minVal_y;
    for(u32 i = 0; i < labelWebCountY; ++i)
    {
        webLabelsY[i].pos_x = graphLabelY.pos_x;
        webLabelsY[i].pos_y = getPosY(val_y);
        webLabelsY[i].color = RGBA(255,255,255,255);
        webLabelsY[i].text = (char*)malloc(MAX_FLOAT_CHAR_LEN);
        addFloatLabel((webLabelsY + i), val_y);
        val_y += h_y;
    }

    f32 h_x = (maxVal_x - minVal_x) / labelWebCountX;
    f32 val_x = minVal_x;
    for(u32 i = 0; i < labelWebCountX; ++i)
    {
        webLabelsX[i].pos_x = getPosX(val_x);
        webLabelsX[i].pos_y = graphLabelX.pos_y;
        webLabelsX[i].color = RGBA(255,255,255,255);
        webLabelsX[i].text = (char*)malloc(MAX_FLOAT_CHAR_LEN);
        addFloatLabel((webLabelsX + i), val_x);
        val_x += h_x;
    }
    curWebLabelX = webLabelsX;
    accumStepWebLabelX = 0;
    isLabel = 1;
}

void Grapher_show_axes()
{
    f32 Ay_x1 = AXIS_START_OFFSET_X;
    f32 Ay_y1 = AXIS_START_OFFSET_Y;

    f32 Ay_x2 = AXIS_START_OFFSET_X;
    f32 Ay_y2 = WINDOW_Y - AXIS_END_OFFSET_Y;

    f32 Ax_x1 = AXIS_START_OFFSET_X;
    f32 Ax_y1 = WINDOW_Y - AXIS_END_OFFSET_Y;

    f32 Ax_x2 = WINDOW_X - AXIS_END_OFFSET_X;
    f32 Ax_y2 = WINDOW_Y - AXIS_END_OFFSET_Y;
    
    DrawLine(Ay_x1, Ay_y1, Ay_x2, Ay_y2, 4, RGBA(255,255,255,255));
    DrawLine(Ax_x1, Ax_y1, Ax_x2, Ax_y2, 4, RGBA(255,255,255,255));

}


void Grapher_append_Dot(f32 val_y)
{
    prevDot = curDot;
    curDot = getNextDot(curDot, 1);
    if(isShifting) lastDot = getNextDot(lastDot, 0);

    minVal_x = curDot->val_x; //saving old param

    curDot->val_x = getNextX();
    curDot->val_y = val_y;
    curDot->prev_dot = prevDot;
}

void Grapher_show(f32 dot_size, u32 color)
{
    Grapher_show_axes();
    drawGraphic(dot_size, color);
    if(isLabel) drawLabels();
}

void Grapher_clear()
{
    for(u32 i = 0; i < dotsCount; ++i)
    {
        dotBuffer[i].val_x = minVal_x;
        dotBuffer[i].val_y = 0;
        dotBuffer[i].prev_dot = 0;
    }
    isShifting = 0;
    curDot = dotBuffer + 1;
    prevDot = dotBuffer;
    lastDot = dotBuffer;
    isLabel = 0;
}


void Grapher_DeInit()
{
    if(!dotBuffer || !webLabelsX || !webLabelsY) return;

    free(graphLabelX.text);
    free(graphLabelY.text);

    for(u32 i = 0; i < labelWebCountX; ++i){free(webLabelsX[i].text);}
    for(u32 i = 0; i < labelWebCountY; ++i){free(webLabelsY[i].text);}

    free(webLabelsX);
    free(webLabelsY);

    free(dotBuffer);

}