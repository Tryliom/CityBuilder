#include "Input.h"

#include <cstring>
#include "sokol_app.h"

#include "Window.h"

bool keys[SAPP_MAX_KEYCODES + 1];
bool previousKeys[SAPP_MAX_KEYCODES + 1];

bool mouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];
bool previousMouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];

Vector2F mousePosition = { 0, 0 };
Vector2F previousMousePosition = { 0, 0 };
bool mouseMoved = false;

namespace Input
{
	void OnInput(const sapp_event* event)
	{
		if (event->type == SAPP_EVENTTYPE_KEY_DOWN)
		{
			keys[event->key_code] = true;
		}
		else if (event->type == SAPP_EVENTTYPE_KEY_UP)
		{
			keys[event->key_code] = false;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_DOWN)
		{
			mouseButtons[event->mouse_button] = true;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_UP)
		{
			mouseButtons[event->mouse_button] = false;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_MOVE)
		{
            previousMousePosition = Vector2F(mousePosition.X, mousePosition.Y);
			mousePosition = Window::ConvertInputPosition({ event->mouse_x, event->mouse_y });
            mouseMoved = true;
		}
	}

	void Update()
	{
        if (!mouseMoved)
        {
            previousMousePosition = Vector2F(mousePosition.X, mousePosition.Y);
        }

        mouseMoved = false;
		memcpy(previousKeys, keys, sizeof(keys));
		memcpy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
	}

	bool IsKeyPressed(int key)
	{
		return keys[key] && !previousKeys[key];
	}

	bool IsKeyReleased(int key)
	{
		return !keys[key] && previousKeys[key];
	}

	bool IsKeyHeld(int key)
	{
		return keys[key];
	}

	bool IsMouseButtonPressed(int button)
	{
		return mouseButtons[button] && previousMouseButtons[button] == 0;
	}

	bool IsMouseButtonReleased(int button)
	{
		return mouseButtons[button] == 0 && previousMouseButtons[button];
	}

	bool IsMouseButtonHeld(int button)
	{
		return mouseButtons[button];
	}

	Vector2F GetMousePosition()
	{
		return mousePosition;
	}

    Vector2F GetPreviousMousePosition()
    {
        return previousMousePosition;
    }
}
