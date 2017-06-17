#include "lib/window.h"
#include "lib/d3d_window.h"
#include "lib/d3d_util.h"
#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>
#include <dwrite.h>

using namespace std;
using Microsoft::WRL::ComPtr;

class D3DDemoWindow : public D3DWindow
{
public:
	D3DDemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) : 
		D3DWindow(name, clientWidth, clientHeight, hInstance)
	{
		// NOOP
	}

	void
	draw() override
	{
		wcout << "DRAWING " << this->name << endl;

		D2D1_RECT_F textRect = D2D1::RectF(0, 0, this->clientWidth, this->clientHeight);
		wstring text = L"11On12";

		D3DRenderTarget renderTarget = this->currentRenderTarget();

		this->d3d11On12Device->AcquireWrappedResources(
			renderTarget.wrappedSwapChainBuffer.GetAddressOf(), 1);

		renderTarget.d2dDeviceContext->SetTarget(
			renderTarget.d2dRenderTarget.Get());
		renderTarget.d2dDeviceContext->BeginDraw();
		renderTarget.d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
		ID2D1SolidColorBrush *brush;
		hrThrowIfFailed(renderTarget.d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Red, 1.0f),
			&brush));

		Microsoft::WRL::ComPtr<IDWriteFactory> writeFactory;
		hrThrowIfFailed(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown **>(writeFactory.GetAddressOf())));
		// Create a DirectWrite text format object.
		IDWriteTextFormat *txtFmt;
		hrThrowIfFailed(writeFactory->CreateTextFormat(
			L"Verdana",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			50,
			L"", //locale
			&txtFmt));

		// Center the text horizontally and vertically.
		txtFmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		txtFmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		renderTarget.d2dDeviceContext->DrawTextW(
			text.c_str(),
			text.size(),
			txtFmt,
			&textRect,
			brush);

		hrThrowIfFailed(renderTarget.d2dDeviceContext->EndDraw());

		this->d3d11On12Device->ReleaseWrappedResources(
			renderTarget.wrappedSwapChainBuffer.GetAddressOf(), 1);

		renderTarget.d2dDeviceContext->Flush();
	}
};

void
runMsgLoop()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void
run(HINSTANCE hInstance)
{
	wstring name = L"This is a window";
	D3DDemoWindow window(name, 500, 500, hInstance);
	window.show();

	wstring otherName = L"Other window";
	Window otherWindow(otherName, 600, 100, hInstance);
	otherWindow.show();

	runMsgLoop();
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	setUpDebugWindow();
	run(hInstance);
	tearDownDebugWindow();

	system("pause");
	return 0;
 }