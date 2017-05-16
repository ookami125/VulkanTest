#pragma once
#include "Renderer.h"
#include <vector>
#include "SceneObject.h"

class Scene
{
	Scene* _parent = nullptr;
	const Renderer* m_Renderer = nullptr;
	std::vector<Scene*> m_ChildScenes;
	std::vector<SceneObject*> m_Objects;
public:
	Scene(Renderer* window);
	~Scene();

	void Update();
	void AddChildScene(Scene* scene, bool active = true);
	void AddObject(SceneObject* obj);

	void ForceRebuildCommandBuffers();
	void CollectCommandBuffers_Local(std::vector<VkCommandBuffer> & out_command_buffers) const;
	void CollectCommandBuffers_Recursive(std::vector<VkCommandBuffer> & out_command_buffers) const;
};

