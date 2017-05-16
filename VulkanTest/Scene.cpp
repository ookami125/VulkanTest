#include "Scene.h"

Scene::Scene(Renderer* renderer) : m_Renderer(renderer)
{

}

Scene::~Scene()
{
}

void Scene::Update()
{
	for (auto obj : m_Objects)
		obj->Update();
	for (auto sce : m_ChildScenes)
		sce->Update();
}

void Scene::AddObject(SceneObject * obj)
{
	m_Objects.push_back(obj);
}

void Scene::CollectCommandBuffers_Local(std::vector<VkCommandBuffer>& out_command_buffers) const
{
	for (auto obj : m_Objects)
		out_command_buffers.push_back(obj->GetActiveCommandBuffer());
}

void Scene::CollectCommandBuffers_Recursive(std::vector<VkCommandBuffer>& out_command_buffers) const
{
	CollectCommandBuffers_Local(out_command_buffers);
	for (auto sce : m_ChildScenes)
		sce->CollectCommandBuffers_Recursive(out_command_buffers);
}