#include "Window.h"

#include <windowsx.h>
#include <cassert>

#include "glad/glad_wgl.h"

static LRESULT CALLBACK win32WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Window* win = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
		case WM_KEYDOWN:
		case WM_KEYUP: {
			WORD vkCode = LOWORD(wParam);
			WORD keyFlags = HIWORD(lParam);
			BOOL keyReleased = (keyFlags & KF_UP) == KF_UP;

			WindowEvent ev{};
			ev.keyCode = int(vkCode);
			ev.type = WindowEvent::keyboardKey;
			ev.keyChar = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);
			ev.buttonState = keyReleased ? WindowEvent::up : WindowEvent::down;
			win->submitEvent(ev);
		} break;
		case WM_CHAR: {
			WindowEvent ev{};
			ev.type = WindowEvent::textInput;
			ev.keyChar = TCHAR(wParam);
			win->submitEvent(ev);
		} break;

		case WM_MOUSEMOVE: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int dx = x - win->mouseX();
			int dy = y - win->mouseY();
			win->updateMouse(x, y);

			WindowEvent ev{};
			ev.screenX = x;
			ev.screenY = y;
			ev.deltaX = dx;
			ev.deltaY = dy;
			ev.type = WindowEvent::mouseMotion;
			win->submitEvent(ev);
		} break;

		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int dx = x - win->mouseX();
			int dy = y - win->mouseY();
			win->updateMouse(x, y);

			WindowEvent ev{};
			ev.screenX = x;
			ev.screenY = y;
			ev.deltaX = dx;
			ev.deltaY = dy;
			ev.type = WindowEvent::mouseButton;
			ev.buttonState = WindowEvent::down;

			if      (msg == WM_LBUTTONDOWN) ev.button = 1;
			else if (msg == WM_MBUTTONDOWN) ev.button = 2;
			else if (msg == WM_RBUTTONDOWN) ev.button = 3;

			win->submitEvent(ev);
		} break;

		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONUP: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int dx = x - win->mouseX();
			int dy = y - win->mouseY();
			win->updateMouse(x, y);

			WindowEvent ev{};
			ev.screenX = x;
			ev.screenY = y;
			ev.deltaX = dx;
			ev.deltaY = dy;
			ev.type = WindowEvent::mouseButton;
			ev.buttonState = WindowEvent::up;

			if      (msg == WM_LBUTTONUP) ev.button = 1;
			else if (msg == WM_MBUTTONUP) ev.button = 2;
			else if (msg == WM_RBUTTONUP) ev.button = 3;

			win->submitEvent(ev);
		} break;
		case WM_MOUSEWHEEL: {
			int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			
			if(wheelDelta) {
				wheelDelta = wheelDelta < 0 ? -1 : 1;
			}

			win->updateWheel(wheelDelta);

			WindowEvent ev{};
			ev.wheel = win->wheel();
			ev.deltaWheel = wheelDelta;

			win->submitEvent(ev);
		} break;
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

Window::~Window() {
	wglMakeCurrent(m_dc, 0);
	wglDeleteContext(m_glrc);
	ReleaseDC(m_handle, m_dc);
	DestroyWindow(m_handle);
}

bool Window::create(const WindowParams& params) {
	HINSTANCE instance = GetModuleHandle(0);

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = win32WndProc;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = HBRUSH(COLOR_WINDOW);
	wc.lpszClassName = params.className.c_str();
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("Failed to initialize window."), TEXT("Error"), MB_OK);
		return false;
	}

	RECT wect;
	wect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - params.width / 2;
	wect.top = GetSystemMetrics(SM_CYSCREEN) / 2 - params.height / 2;
	wect.right = wect.left + params.width;
	wect.bottom = wect.top + params.height;

	AdjustWindowRectEx(&wect, WS_OVERLAPPEDWINDOW, 0, 0);

	m_handle = CreateWindowEx(
		0,
		params.className.c_str(),
		params.title.c_str(),
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		wect.left, wect.top, wect.right - wect.left, wect.bottom - wect.top,
		NULL, NULL,
		instance,
		NULL
	);

	SetWindowLongPtr(m_handle, GWLP_USERDATA, LONG_PTR(this));

	m_dc = GetDC(m_handle);

	initializeGLEXT();
	m_glrc = initializeGL();

	// Load RenderDoc
	if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
		assert(ret == 1);
	}

	ShowWindow(m_handle, SW_SHOW);
	UpdateWindow(m_handle);

	return true;
}

bool Window::pollEvents(WindowEvent& e) {
	bool open = true;
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			open = false;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (!m_eventQueue.empty()) {
		e = m_eventQueue.front();
		m_eventQueue.pop();
	}
	else {
		ZeroMemory(&e, sizeof(WindowEvent));
	}

	return open;
}

void Window::swapBuffers() {
	SwapBuffers(m_dc);
}

const String& Window::title() const {
	String title{};
	title.resize(256);
	GetWindowText(m_handle, title.data(), 256);
	return title;
}

void Window::title(const String& title) {
	SetWindowText(m_handle, title.c_str());
}

Size Window::size() {
	WINDOWINFO winfo{};
	GetWindowInfo(m_handle, &winfo);

	return std::make_pair<uint32_t, uint32_t>(
		uint32_t(winfo.rcClient.right - winfo.rcClient.left),
		uint32_t(winfo.rcClient.bottom - winfo.rcClient.top)
	);
}

void Window::submitEvent(WindowEvent e) {
	m_eventQueue.push(e);
}

void Window::initializeGLEXT() {
	// Implementation from here: https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	WNDCLASS wc = {
		.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		.lpfnWndProc = DefWindowProcA,
		.hInstance = GetModuleHandle(0),
		.lpszClassName = TEXT("Dummy_WGL_djuasiodwa"),
	};

	// Create a fake Window
	HWND fakeHandle = CreateWindowEx(
		0,
		wc.lpszClassName,
		TEXT("FAKE WINDOW"),
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		wc.hInstance,
		0
	);

	HDC fakeDC = GetDC(fakeHandle);
	int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
	SetPixelFormat(fakeDC, pixelFormat, &pfd);

	HGLRC fakeCtx = wglCreateContext(fakeDC);
	wglMakeCurrent(fakeDC, fakeCtx);

	gladLoadWGL(fakeDC);

	wglMakeCurrent(fakeDC, 0);
	wglDeleteContext(fakeCtx);
	ReleaseDC(fakeHandle, fakeDC);
	DestroyWindow(fakeHandle);
}

#ifdef _DEBUG
static void APIENTRY debugCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* user)
{
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
	/*if ((severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) && IsDebuggerPresent()) {
		assert(!"OpenGL error - check the callstack in debugger");
	}*/
}
#endif

HGLRC Window::initializeGL() {
	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,         GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,         GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,          GL_TRUE,
		WGL_ACCELERATION_ARB,           WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,             WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,             32,
		WGL_DEPTH_BITS_ARB,             24,
		WGL_STENCIL_BITS_ARB,           8,
		0
	};

	int pixelFormat;
	UINT numFormats;
	wglChoosePixelFormatARB(m_dc, pixel_format_attribs, 0, 1, &pixelFormat, &numFormats);

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(m_dc, pixelFormat, sizeof(pfd), &pfd);
	SetPixelFormat(m_dc, pixelFormat, &pfd);

	int gl46Attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
#ifdef _DEBUG
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
		0,
	};
	HGLRC ctx = wglCreateContextAttribsARB(m_dc, 0, gl46Attribs);
	wglMakeCurrent(m_dc, ctx);

	gladLoadGL();

#ifdef _DEBUG
	glDebugMessageCallback(&debugCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	return ctx;
}
