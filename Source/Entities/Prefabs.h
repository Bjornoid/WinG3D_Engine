// uses a nameless namespace to register and retreive prefabricated entities
#ifndef PREFABS_H
#define PREFABS_H

// snake game (avoid name collisions)
namespace Wing3D
{
	bool RegisterPrefab(const char* prefabName, const flecs::entity inPrefab);
	bool RetreivePrefab(const char* prefabName, flecs::entity &outPrefab);
	bool UnregisterPrefab(const char* prefabName);
}

#endif