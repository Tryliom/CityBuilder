#include "Input.h"

#include <cstring>
#include "sokol_app.h"

#include "Window.h"

bool tempKeys[SAPP_MAX_KEYCODES + 1];
bool keys[SAPP_MAX_KEYCODES + 1];
bool previousKeys[SAPP_MAX_KEYCODES + 1];

bool tempMouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];
bool mouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];
bool previousMouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];

Vector2F mousePosition = { 0, 0 };
Vector2F previousMousePosition = { 0, 0 };
bool mouseMoved = false;

float tempMouseWheelDelta = 0;
float mouseWheelDelta = 0;

namespace Input
{
	void OnInput(const sapp_event* event)
	{
		if (event->type == SAPP_EVENTTYPE_KEY_DOWN)
		{
			tempKeys[event->key_code] = true;
		}
		else if (event->type == SAPP_EVENTTYPE_KEY_UP)
		{
			tempKeys[event->key_code] = false;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_DOWN)
		{
			tempMouseButtons[event->mouse_button] = true;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_UP)
		{
			tempMouseButtons[event->mouse_button] = false;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_MOVE)
		{
            previousMousePosition = Vector2F{ mousePosition.X, mousePosition.Y };
			mousePosition = { event->mouse_x, event->mouse_y };
            mouseMoved = true;
		}
		else if (event->type == SAPP_EVENTTYPE_MOUSE_SCROLL)
		{
			tempMouseWheelDelta += event->scroll_y;
		}
	}

	void Update()
	{
        if (!mouseMoved)
        {
            previousMousePosition = Vector2F{mousePosition.X, mousePosition.Y};
        }

		mouseWheelDelta = tempMouseWheelDelta;
		tempMouseWheelDelta = 0;

        mouseMoved = false;
		memcpy(previousKeys, keys, sizeof(keys));
		memcpy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
		memcpy(keys, tempKeys, sizeof(keys));
		memcpy(mouseButtons, tempMouseButtons, sizeof(mouseButtons));
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
		return mouseButtons[button] && !previousMouseButtons[button];
	}

	bool IsMouseButtonReleased(int button)
	{
		return !mouseButtons[button] && previousMouseButtons[button];
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

	float GetMouseWheelDelta()
	{
		return mouseWheelDelta;
	}
}
