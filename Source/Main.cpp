// handles everything
#include "Application.h"
// program entry point
int main()
{
	Application Wing3D_Engine;
	if (Wing3D_Engine.Init()) {
		if (Wing3D_Engine.Run()) {
			return Wing3D_Engine.Shutdown() ? 0 : 1;
		}
	}
	return 1;
}