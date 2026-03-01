#include "SceneManager.h"
#include "IScene.h"
#include "Log.h"

void SceneManager::Set(std::unique_ptr<IScene> scene)
{
    // Exit all
    while (!stack_.empty())
    {
        Log::Info("Scene: OnExit");
        stack_.back()->OnExit();
        stack_.pop_back();
    }

    if (scene)
    {
        stack_.push_back(std::move(scene));
        Log::Info("Scene: OnEnter");
        stack_.back()->OnEnter();
    }
}

void SceneManager::Push(std::unique_ptr<IScene> scene)
{
    if (!scene) return;

    stack_.push_back(std::move(scene));
    Log::Info("Scene: OnEnter (Push)");
    stack_.back()->OnEnter();
}

void SceneManager::Pop()
{
    if (stack_.empty()) return;

    Log::Info("Scene: OnExit (Pop)");
    stack_.back()->OnExit();
    stack_.pop_back();
}

IScene* SceneManager::Top() const
{
    if (stack_.empty()) return nullptr;
    return stack_.back().get();
}

IScene* SceneManager::UnderTop() const
{
    if (stack_.size() < 2) return nullptr;
    return stack_[stack_.size() - 2].get();
}
