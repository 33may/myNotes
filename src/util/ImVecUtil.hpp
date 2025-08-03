#pragma once
#include <imgui.h>

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator*(const ImVec2& v, float s) { return ImVec2(v.x * s, v.y * s); }
inline ImVec2 operator*(float s, const ImVec2& v) { return ImVec2(v.x * s, v.y * s); }
inline ImVec2 operator/(const ImVec2& v, float s) { return ImVec2(v.x / s, v.y / s); }

inline ImVec2& operator+=(ImVec2& a, const ImVec2& b) { a.x += b.x; a.y += b.y; return a; }
inline ImVec2& operator*=(ImVec2& v, float s) { v.x *= s; v.y *= s; return v; }
inline ImVec2& operator/=(ImVec2& v, float s) { v.x /= s; v.y /= s; return v; }
