#include <windows.h>
#include <gl/gl.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb-master/stb_image.h"
#include "../stb-master/stb_easy_font.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int width = 1080;
int height = 720;

float vertex[] = {-0.3,-0.4,0, 0.3,-0.4,0, 0.3,0.4,0, -0.3,0.4,0};
float texCoord[] = {0,0.33, 0.12,0.33, 0.12,0, 0,0};

float characterX = 0.0f;
float characterY = 0.0f;

unsigned int texture;

typedef struct {
    float texCoords[8]; // ���������� ������� ��� ����� ��������
} Frame;

Frame walkLeftFrames[8];
Frame walkRightFrames[8];
Frame idleFrames[8];
// ���������� �������� ��������� ������� ��� �������� ������ ����� � ������
Frame jumpLeftFrames[8];
Frame jumpRightFrames[8];

// ���������� ���������� ��� ���������� ���������
int currentFrame = 0; // ������� ���� ��������
int frameCount = 8;   // ����� ���������� ������ ��������
int frameDelay = 10;  // �������� ����� ������� � �������������
int frameTimer = 0;   // ������ ��� ������������ ������� ����� �������

BOOL leftKeyPressed = FALSE;
BOOL rightKeyPressed = FALSE;
BOOL spaceKeyPressed = FALSE;
BOOL escapeKeyPressed = FALSE;

BOOL isPlaying = FALSE;
BOOL bQuit = FALSE;

BOOL isJumping = FALSE;
float jumpVelocity = 0.01f;
float jumpHeight = 0.5f;

float verticalVelocity = 0.0f; // ��������� ������������ ��������
float gravity = 0.01f; // ��������� ����������

typedef struct{
    char name[20];
    float vert[8];
    BOOL hover;
} TButton;

// ������� ���������� ��������
void UpdateAnimation()
{
    // ����������� ������
    frameTimer++;

    // ������������� �� ��������� ����, ���� ���������� �������� ����� �������
    if (frameTimer >= frameDelay)
    {
        currentFrame = (currentFrame + 1) % frameCount; // ��������� � ���������� ����� ��������
        frameTimer = 0; // ���������� ������
    }
}

TButton btn[] = {
    {"start", {0,0, 100,0, 100,30, 0,30}, FALSE},
    {"stop", {0,40, 100,40, 100,70, 0,70}, FALSE},
    {"quit", {0,80, 100,80, 100,110, 0,110}, FALSE}
};
int btnCnt = sizeof(btn) / sizeof(btn[0]);

unsigned int textureBackground;

void ShowBackground()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBackground);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void Init()
{

    int back_wid, back_hei, back_cnt;
    unsigned char *back_data = stbi_load("background.png", &back_wid, &back_hei, &back_cnt, 0);

    glGenTextures(1, &textureBackground);
    glBindTexture(GL_TEXTURE_2D, textureBackground);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, back_wid, back_hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, back_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(back_data);

    int wid, hei, cnt;
    unsigned char *data = stbi_load("sprite.png", &wid, &hei, &cnt, 0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wid, hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);


    for (int i = 0; i < 8; i++) {
        walkLeftFrames[i].texCoords[2] = 0.125f * i; // X ���������� ������ �������� ���� ��������
        walkLeftFrames[i].texCoords[3] = 0.66f;     // Y ���������� ������ �������� ���� ��������
        walkLeftFrames[i].texCoords[0] = 0.125f * (i + 1); // X ���������� ������� �������� ���� ��������
        walkLeftFrames[i].texCoords[1] = 0.66f;          // Y ���������� ������� �������� ���� ��������
        walkLeftFrames[i].texCoords[6] = 0.125f * (i + 1); // X ���������� ������� ������� ���� ��������
        walkLeftFrames[i].texCoords[7] = 0.335f;          // Y ���������� ������� ������� ���� ��������
        walkLeftFrames[i].texCoords[4] = 0.125f * i; // X ���������� ������ ������� ���� ��������
        walkLeftFrames[i].texCoords[5] = 0.335f;     // Y ���������� ������ ������� ���� ��������
    }
    // ������������� ��������� ������� ��� �������� ������ ������
    for (int i = 0; i < 8; i++) {
        walkRightFrames[i].texCoords[0] = 0.125f * i; // X ���������� ������ �������� ���� ��������
        walkRightFrames[i].texCoords[1] = 0.66f;     // Y ���������� ������ �������� ���� ��������
        walkRightFrames[i].texCoords[2] = 0.125f * (i + 1); // X ���������� ������� �������� ���� ��������
        walkRightFrames[i].texCoords[3] = 0.66f;          // Y ���������� ������� �������� ���� ��������
        walkRightFrames[i].texCoords[4] = 0.125f * (i + 1); // X ���������� ������� ������� ���� ��������
        walkRightFrames[i].texCoords[5] = 0.335f;          // Y ���������� ������� ������� ���� ��������
        walkRightFrames[i].texCoords[6] = 0.125f * i; // X ���������� ������ ������� ���� ��������
        walkRightFrames[i].texCoords[7] = 0.335f;     // Y ���������� ������ ������� ���� ��������
    }

    // ������������� ��������� ������� ��� �������� ����� (idle)
    for (int i = 0; i < 8; i++) {
        idleFrames[i].texCoords[0] = 0.125f * i; // X ���������� ������ �������� ���� ��������
        idleFrames[i].texCoords[1] = 0.33f;     // Y ���������� ������ �������� ���� ��������
        idleFrames[i].texCoords[2] = 0.125f * (i + 1); // X ���������� ������� �������� ���� ��������
        idleFrames[i].texCoords[3] = 0.33f;          // Y ���������� ������� �������� ���� ��������
        idleFrames[i].texCoords[4] = 0.125f * (i + 1); // X ���������� ������� ������� ���� ��������
        idleFrames[i].texCoords[5] = 0.0f;           // Y ���������� ������� ������� ���� ��������
        idleFrames[i].texCoords[6] = 0.125f * i; // X ���������� ������ ������� ���� ��������
        idleFrames[i].texCoords[7] = 0.0f;      // Y ���������� ������ ������� ���� ��������
    }
    for (int i = 0; i < 8; i++) {
        jumpLeftFrames[i].texCoords[2] = 0.125f * i; // X ���������� ������ �������� ���� ��������
        jumpLeftFrames[i].texCoords[3] = 0.995f;     // Y ���������� ������ �������� ���� ��������
        jumpLeftFrames[i].texCoords[0] = 0.125f * (i + 1); // X ���������� ������� �������� ���� ��������
        jumpLeftFrames[i].texCoords[1] = 0.995f;          // Y ���������� ������� �������� ���� ��������
        jumpLeftFrames[i].texCoords[6] = 0.125f * (i + 1); // X ���������� ������� ������� ���� ��������
        jumpLeftFrames[i].texCoords[7] = 0.665f;           // Y ���������� ������� ������� ���� ��������
        jumpLeftFrames[i].texCoords[4] = 0.125f * i; // X ���������� ������ ������� ���� ��������
        jumpLeftFrames[i].texCoords[5] = 0.665f;      // Y ���������� ������ ������� ���� ��������
    }
    for (int i = 0; i < 8; i++) {
        jumpRightFrames[i].texCoords[0] = 0.125f * i; // X ���������� ������ �������� ���� ��������
        jumpRightFrames[i].texCoords[1] = 0.995f;     // Y ���������� ������ �������� ���� ��������
        jumpRightFrames[i].texCoords[2] = 0.125f * (i + 1); // X ���������� ������� �������� ���� ��������
        jumpRightFrames[i].texCoords[3] = 0.995f;          // Y ���������� ������� �������� ���� ��������
        jumpRightFrames[i].texCoords[4] = 0.125f * (i + 1); // X ���������� ������� ������� ���� ��������
        jumpRightFrames[i].texCoords[5] = 0.665f;           // Y ���������� ������� ������� ���� ��������
        jumpRightFrames[i].texCoords[6] = 0.125f * i; // X ���������� ������ ������� ���� ��������
        jumpRightFrames[i].texCoords[7] = 0.665f;      // Y ���������� ������ ������� ���� ��������
    }
}

float* currentTexCoord;

void Show()
{
        // ����������� ������� ����
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        glColor3f(1,1,1);
        glPushMatrix();
           glEnableClientState(GL_VERTEX_ARRAY);
           glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            // ���������� ������� ���� �������� �� ������ frameTimer

            // ���������� ������� ���������� ��������� �� ������ �������� ����� ��������
            Frame* currentFrames = NULL;
            if (isJumping) {
                currentFrames = (leftKeyPressed ? jumpLeftFrames : (rightKeyPressed ? jumpRightFrames : NULL));
            } else {
                currentFrames = (leftKeyPressed ? walkLeftFrames : (rightKeyPressed ? walkRightFrames : idleFrames));
            }

            // ���������� ������� ���������� ��������� �� ������ �������� ����� ��������
            if (currentFrames) {
                currentTexCoord = currentFrames[currentFrame].texCoords;
            } else {
                currentTexCoord = idleFrames[currentFrame].texCoords;
            }
            glTexCoordPointer(2, GL_FLOAT, 0, currentTexCoord);

           if (isJumping) {
               // ���� �������� � �������� ������, ��������� ��� �������
               characterY -= jumpVelocity;
               // ��������� �������� ������ � �������� �������
               jumpVelocity -= 0.01f;
               // ���� �������� ��������� ������� ������� ������, ������ ����������� ��������
               if (jumpVelocity <= 0.0f) {
                   jumpVelocity = -jumpVelocity;
               }
               // ���� �������� ��������� ������ ������� ������, ��������� ������ � ������������� ��� ������� � ����
               if (characterY >= jumpHeight) {
                   isJumping = FALSE;
                   characterY = 0.0f; // ���������� ��������� �� �����
               }
           }
           UpdateAnimation();

           // ������� ��������� � ������������ � ��� ������� ����������
       glTranslatef(characterX, characterY, 0.0f);

       glVertexPointer(3, GL_FLOAT, 0, vertex);
       glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

       glDisableClientState(GL_VERTEX_ARRAY);
       glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

void print_string(float x, float y, char *text, float r, float g, float b)
{
  static char buffer[99999]; // ~500 chars
  int num_quads;

  num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

  glColor3f(r,g,b);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, buffer);
  glDrawArrays(GL_QUADS, 0, num_quads*4);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void TButton_Show(TButton btn)
{
    glEnableClientState(GL_VERTEX_ARRAY);
        if (btn.hover) glColor3f(0,1,0);
        else glColor3f(0,0.8,0);
        glVertexPointer(2, GL_FLOAT, 0, btn.vert);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    // ����������� ������ �� ������
    glColor3f(0.0, 1.0, 1.0); // ������������� ���� ������
    glPushMatrix();
        glTranslatef(btn.vert[0], btn.vert[1], 0);
        glScalef(2, 2, 2);
        print_string(3,3, btn.name, 1,0,0);
    glPopMatrix();
}

BOOL PointInButton(int x, int y, TButton btn)
{
    return (x > btn.vert[0]) && (x < btn.vert[4]) &&
           (y > btn.vert[1]) && (y < btn.vert[5]);
}

void ShowMenu()
{
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height,0, -1,1);
        for (int i=0; i < btnCnt; i++)
            TButton_Show(btn[i]);
    glPopMatrix();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          width,
                          height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);


    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    Init();
    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (isPlaying) {


                /* OpenGL animation code goes here */
                glClearColor(0.8f, 0.5f, 0.3f, 0.5f);
                glClear(GL_COLOR_BUFFER_BIT);

                ShowBackground();

                Show();
                SwapBuffers(hDC);
                UpdateAnimation(); // ���������� ��������

                if (isJumping) {
                    // ��������� ������� ��������� � ������ ������������ ��������
                    characterY += verticalVelocity;
                    // ��������� ���������� � ������������ ��������
                    verticalVelocity -= gravity;

                    // ���� �������� ��������� �����, ��������� ������
                    if (characterY <= 0.0f) {
                        isJumping = FALSE;
                        characterY = 0.0f; // ���������� ��������� �� �����
                    }
                }
                if (leftKeyPressed) {
                    // ������� ��������� �����
                    characterX -= 0.01f; // ��������� �������� ����� ��������� ��� ���� �����
                }
                if (rightKeyPressed) {
                    // ������� ��������� ������
                    characterX += 0.01f; // ��������� �������� ����� ��������� ��� ���� �����
                }
                if (spaceKeyPressed) {
                    // ���� �������� �� � �������� ������, �������� ������
                    if (!isJumping) {
                        isJumping = TRUE;
                        verticalVelocity = 0.5f; // ��������� �������� ������
                    }
                }
                if (escapeKeyPressed) {
                    // ������ ������ Escape, ������� �� ������ ����
                    isPlaying = FALSE;
                }


                theta += 1.0f;
                Sleep (1);
            } else {
                /* ���������� ������ ���� */
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                ShowMenu();

                SwapBuffers(hDC);
            }
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_LBUTTONDOWN:
            if (!isPlaying) {
                for (int i=0; i< btnCnt; i++) {
                    if (strcmp(btn[i].name, "start") == 0 && PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i])) {
                        isPlaying = TRUE;
                        break;
                    }
                    if (strcmp(btn[i].name, "quit") == 0 && PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i])) {
                        PostQuitMessage(0);
                        break;
                    }
                }
            } else {
                // ��������� ��������� ������ "Quit" � ������������ ����
                for (int i=0; i< btnCnt; i++) {
                    if (strcmp(btn[i].name, "quit") == 0 && PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i])) {
                        bQuit = TRUE; // ������������� ���� ������ �� ����
                        break;
                    }
                }
            }
            break;

        case WM_MOUSEMOVE:
            for (int i=0; i< btnCnt; i++)
                btn[i].hover = PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]);
        break;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);
            glViewport(0,0, width, height);
            glLoadIdentity();
            float k = width / (float)height;
            glOrtho(-k,k, -1,1, -1,1);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
            if (isPlaying) {
                switch (wParam) {
                    case VK_LEFT:
                        leftKeyPressed = TRUE;
                        break;
                    case VK_RIGHT:
                        rightKeyPressed = TRUE;
                        break;
                    case VK_SPACE:
                        spaceKeyPressed = TRUE;
                        break;
                    case VK_ESCAPE:
                        escapeKeyPressed = TRUE;
                        break;
                    // ������ ����������� ������
                }
            }
            break;

        case WM_KEYUP:
            if (isPlaying) {
                switch (wParam) {
                    case VK_LEFT:
                        leftKeyPressed = FALSE;
                        break;
                    case VK_RIGHT:
                        rightKeyPressed = FALSE;
                        break;
                    case VK_SPACE:
                        spaceKeyPressed = FALSE;
                        break;
                    case VK_ESCAPE:
                        escapeKeyPressed = FALSE;
                        break;
                    // ������ ����������� ������
                }
            }
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    // �������� ����� ���������� � ����������� ��� ���������
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

