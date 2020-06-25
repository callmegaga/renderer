#pragma once
#include <memory>
#include <wtypes.h>
#include <vector>

// 向量矩阵操作
namespace SoftRender
{
    // 点 向量
    struct Vector4
    {
        float x;
        float y;
        float z;
        float w;
    };

    // 矩阵
    struct Matrix
    {
        float m[4][4];
    };

    // 顶点
    struct vertex
    {
        Vector4 point;
        Vector4 color;
    };

    // 规范化
    Vector4 normallize(const Vector4& v)
    {
        float len = (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return { v.x / len, v.y / len, v.z / len, 0.0f };
    }

    // 叉积
    Vector4 cross(const Vector4& u, const Vector4& v)
    {
        return { u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x, 0.0f };
    }

    // 点积
    float dot(const Vector4& u, const Vector4& v)
    {
        return u.x * v.x + u.y * v.y + u.z * v.z;
    }

    // 矩阵乘法
    Matrix mul(const Matrix& a, const Matrix& b)
    {
        Matrix r;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                r.m[i][j] = a.m[i][0] * b.m[0][j]
                    + a.m[i][1] * b.m[1][j]
                    + a.m[i][2] * b.m[2][j]
                    + a.m[i][3] * b.m[3][j];
            }
        }
        return r;
    }

    // 点/向量转换
    Vector4 transform(const Vector4& v, const Matrix& m)
    {
        Vector4 r;
        r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0];
        r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1];
        r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2];
        r.w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3];
        return r;
    }

    // 目标立方体8个顶点 摄像机方向
    std::vector<vertex> vertexes = {
        // 近相机面
        {{-1.0f, +1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{+1.0f, +1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{+1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 0.0f}},

        // 远相机面
        {{-1.0f, +1.0f, +1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 0.0f}},
        {{+1.0f, +1.0f, +1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{+1.0f, -1.0f, +1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 0.0f}},
        {{-1.0f, -1.0f, +1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}}
    };
}

namespace SoftRender
{
    int g_width = 0;
    int g_height = 0;

    HDC g_tempDC = nullptr;
    HBITMAP g_tempBm = nullptr;
    HBITMAP g_oldBm = nullptr;
    unsigned int* g_frameBuff = nullptr;
    std::shared_ptr<float[]> g_depthBuff = nullptr;

    unsigned int bgColor = ((123 << 16) | (195 << 8) | 221);

    // 初始化渲染器 屏幕长宽 屏幕缓冲
    void initRenderer(int w, int h, HWND hWnd);
    // 每帧绘制
    void update(HWND hWnd);
    // 清理屏幕缓冲
    void clearBuffer();
    void shutDown();
}

void SoftRender::initRenderer(int w, int h, HWND hWnd)
{
    g_width = w;
    g_height = h;

    // 1. 创建一个屏幕缓冲
    // 1.1 创建一个与当前设备兼容的DC
    HDC hDC = GetDC(hWnd);
    g_tempDC = CreateCompatibleDC(hDC);
    ReleaseDC(hWnd, hDC);
    // 1.2 创建该DC的bitmap缓冲  32位色
    BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
        (DWORD)w * h * 4, 0, 0, 0, 0 } };
    g_tempBm = CreateDIBSection(g_tempDC, &bi, DIB_RGB_COLORS, (void**)&g_frameBuff, 0, 0);
    // 1.3 选择该bitmap到dc中
    g_oldBm = (HBITMAP)SelectObject(g_tempDC, g_tempBm);
    // 1.4 创建深度缓冲区
    g_depthBuff.reset(new float[w * h]);

    // 清理屏幕缓冲
    clearBuffer();
}

void SoftRender::update(HWND hWnd)
{
    // 1. clear frameBuffer
    clearBuffer();

    // present frameBuffer to screen
    HDC hDC = GetDC(hWnd);
    BitBlt(hDC, 0, 0, g_width, g_height, g_tempDC, 0, 0, SRCCOPY);
    ReleaseDC(hWnd, hDC);
}

void SoftRender::clearBuffer()
{
    for (int row = 0; row < g_height; ++row)
    {
        for (int col = 0; col < g_width; ++col)
        {
            int idx = row * g_width + col;
            // 默认背景色浅蓝 R123 G195 B221
            g_frameBuff[idx] = bgColor;
            // 深度缓冲区 1.0f
            g_depthBuff[idx] = 1.0f;
        }
    }
}

void SoftRender::shutDown()
{
    if (g_tempDC)
    {
        if (g_oldBm)
        {
            SelectObject(g_tempDC, g_oldBm);
            g_oldBm = nullptr;
        }
        DeleteDC(g_tempDC);
        g_tempDC = nullptr;
    }

    if (g_tempBm)
    {
        DeleteObject(g_tempBm);
        g_tempBm = nullptr;
    }
}