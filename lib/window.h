#pragma once
#include <string>

class Window
{
public:
	// Initializes the window. May throw.
	explicit Window(const std::wstring& name);

	~Window();

protected:
	// Copying Window not allowed.
	Window(const Window& rhs) = delete;
	Window& operator=(const Window& rhs) = delete;

// State
public:
	const std::wstring name;
};
