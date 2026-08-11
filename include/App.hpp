#pragma once
#include "GameTrainer.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
namespace igi {
class App{
public:int run(HINSTANCE,int);
private:
 static LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM,LPARAM);
 LRESULT handle(HWND,UINT,WPARAM,LPARAM);
 bool createResources();void discardResources();void paint();void showAbout();
 void rounded(float,float,float,float,float,ID2D1Brush*);
 void text(const wchar_t*,float,float,float,float,float,ID2D1Brush*,DWRITE_TEXT_ALIGNMENT=DWRITE_TEXT_ALIGNMENT_LEADING);
 void led(float,float,LedState);void card(float,const wchar_t*,const wchar_t*,const wchar_t*,const FeatureView&);
 HINSTANCE instance_{};HWND hwnd_{};GameTrainer trainer_;
 Microsoft::WRL::ComPtr<ID2D1Factory> factory_;Microsoft::WRL::ComPtr<IDWriteFactory> writeFactory_;Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> target_;
 Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bg_,panel_,stroke_,white_,muted_,accent_,green_,amber_,red_,off_;
};
}
