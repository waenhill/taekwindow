#include <windows.h>

#include "hooks.hpp"
#include "drag.hpp"
#include "config.hpp"
#include "util.hpp"
#include "state.hpp"
#include "debuglog.hpp"

/* The button that we're dragging with.
 * Only meaningful while we're dragging, of course.
 */
extern MouseButton draggingButton;

/* Whether or not the modifier key is currently down.
 */
extern bool modifierDown;

/* The current state we're in.
 */
extern DragState currentState;

/* Whether or not dragging has occurred since the last key-down event of the Modifier.
 * If dragging has occurred, this prevents the key-up event to be passed on.
 */
extern bool haveDragged;

#ifdef _DEBUG
/* Steal this from main.cpp.
 */
extern DWORD mainThreadId;
#endif

/* Handles a button-down event.
 * Returns true if the event should not be passed on to the application, false otherwise.
 */
bool handleButtonDown(MouseButton button, HWND window, POINT mousePos) {
	if (modifierDown && currentState == dsNone && (button == moveButton || button == resizeButton)) {
		DEBUGLOG("Handling button down event");
		// Store the button we're using so we know when it's released.
		draggingButton = button;
		// Remember that we have dragged something during this press of the modifier, which means that
		// the subsequent release event should be eaten.
		haveDragged = true;
		// Yippee! A Modifier-drag event just started that we want to process (or ignore).
		// Find the actual window being dragged: this is the top-level window that is the ultimate parent
		// of the window receiving the event. Seems to work for MDI's too.
		DEBUGLOG("Finding ancestor of window %04x", window);
		window = GetAncestor(window, GA_ROOT);
		DEBUGLOG("Ancestor is %04x", window);
		if (button == moveButton && isMovableWindow(window)) {
			// Window can be moved.
			currentState = dsMoving;
			startMoveAction(window, mousePos);
		} else if (button == resizeButton && isResizableWindow(window)) {
			// Window can be resized.
			currentState = dsResizing;
			startResizeAction(window, mousePos);
		} else if (button == moveButton || button == resizeButton) {
			// Modifier-dragging an invalid window. The user won't expect her actions to be passed
			// to that window, so we suppress all events until the mouse button is released.
			DEBUGLOG("Ignoring button down event because window %04x is not movable/resizable", window);
			currentState = dsIgnoring;
		}
		// Either way, we eat the event.
		return true;
	} else {
		// Event we don't handle. Stay away from it.
		CONDDEBUGLOG(!modifierDown, "Ignoring button down event because the modifier is up");
		CONDDEBUGLOG(currentState != dsNone, "Ignoring button down event because the current state is not dsNone");
		CONDDEBUGLOG(button != moveButton && button != resizeButton, "Ignoring button down event because the button is not interesting");
		return false;
	}
}

/* Handles a mouse button release event.
 * Returns true if the event was processed and should not be passed to the application, false otherwise.
 */
bool handleButtonUp(MouseButton button, POINT mousePos) {
	// We only want to take action if it's the current dragging button being released.
	if (button == draggingButton) {
		DEBUGLOG("Handling button up event");
		// The button we're dragging with was released.
		switch (currentState) {
			case dsMoving:
				endMoveAction();
				currentState = dsNone;
				return true;
			case dsResizing:
				endResizeAction();
				currentState = dsNone;
				return true;
			case dsIgnoring:
				currentState = dsNone;
				return true;
		}
	}
	// Nothing interesting.
	return false;
}

/* Handles a mouse movement event.
 * Returns true if the event was processed and should not be passed to the application, false otherwise.
 */
bool handleMove(POINT mousePos) {
	if (currentState == dsMoving) {
		// We are handling the moving or resizing of a window.
		doMoveAction(mousePos);
		return true;
	} else if (currentState == dsResizing) {
		doResizeAction(mousePos);
		return true;
	} else if (currentState == dsIgnoring) {
		return true;
	}
	return false;
}

/* The function for handling mouse events. This is the reason why we have to use a separate DLL;
 * see the SetWindowsHookEx documentation for details.
 */
LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	DEBUGLOG("Mouse hook called");
	bool processed = false; // Set to true if we don't want to pass the event to the application.
	if (nCode >= 0 && nCode == HC_ACTION) { // If nCode < 0, do nothing as per Microsoft's recommendations.
		MOUSEHOOKSTRUCT *eventInfo = (MOUSEHOOKSTRUCT*)lParam;
		switch (wParam) {
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_NCLBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
				processed = handleButtonDown(eventToButton(wParam), eventInfo->hwnd, eventInfo->pt);
				break;
			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			case WM_NCLBUTTONUP:
			case WM_NCMBUTTONUP:
			case WM_NCRBUTTONUP:
				processed = handleButtonUp(eventToButton(wParam), eventInfo->pt);
				break;
			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE:
				processed = handleMove(eventInfo->pt);
				break;
		}
	}

	LRESULT res = CallNextHookEx((HHOOK)37, nCode, wParam, lParam); // first argument ignored
	if (processed)
		res = 1; // nonzero return value prevents passing the event to the application
	return res;
}

/* The function for handling keyboard events.
 * Or rather, the function to eat keyboard events that the application shouldn't receive.
 */
LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	DEBUGLOG("Keyboard hook called");
#ifdef _DEBUG
	if (wParam == 0x51) {
		DEBUGLOG("Panic button pressed");
		// Q button pressed. Panic button for debugging.
		PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
		return 1;
	}
#endif
	if (nCode >= 0 && nCode == HC_ACTION) {
		// Something MAY have happened to the modifier key.
		// We have a key-code in wParam, but it does not distinguish e.g. left and right Alt,
		// because it is VK_MENU for both of them. Therefore we have to use GetAsyncKeyState.
		bool wasDown = modifierDown;
		modifierDown = GetAsyncKeyState(modifier) & 0x8000;
		if (wasDown && !modifierDown) {
			DEBUGLOG("Modifier released");
			// Modifier was released. Only pass the event on if there was no drag event.
			if (haveDragged) {
				return 1;
			}
			haveDragged = false;
		} else if (!wasDown && modifierDown) {
			DEBUGLOG("Modifier pressed");
			// Modifier was pressed. There has been no drag event since.
			haveDragged = false;
		}
	}
	return CallNextHookEx((HHOOK)37, nCode, wParam, lParam); // first argument ignored
}
