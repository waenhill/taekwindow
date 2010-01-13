#ifndef WORKERTHREAD_HPP
#define WORKERTHREAD_HPP

#include <windows.h>

/* A separate thread that does all the work of handling messages coming in from the hooks.
 * 
 * We can't do all this work from the hooks itself, because many API calls
 * (for example, SetWindowPlacement, GetWindowRect, GetAncestor) cause
 * the system message queue to be flushed, therefore possibly calling a hook again.
 * Now we're in an indirectly recursive hook call, which causes all kinds of assumptions to fail
 * and things to go boom pretty hard.
 */
class WorkerThread {

	DWORD d_id;
	bool d_exiting;

	public:

		WorkerThread();
		~WorkerThread();

		/* These are called from a different thread. */
		void start();
		void stop();

		/* Post a message to this thread from another thread. */
		void postMessage(UINT message, WPARAM wParam, LPARAM lParam);

	private:

		static DWORD WINAPI staticThreadProc(LPVOID param);
		DWORD threadProc();

		void handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

		WorkerThread(WorkerThread const &other); // not implemented
		WorkerThread &operator=(WorkerThread const &other); // not implemented

};

#endif
