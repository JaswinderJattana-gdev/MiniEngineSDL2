#pragma once
#include <memory>
#include <vector>
#include "IScene.h" 

class IScene;

class SceneManager
{
public:
    void SetContext(const EngineContext* ctx) { ctx_ = ctx; }

    // Replace entire stack with one scene
    void Set(std::unique_ptr<IScene> scene);

    // Stack operations
    void Push(std::unique_ptr<IScene> scene);
    void Pop();

    // Access
    IScene* Top() const;
    IScene* UnderTop() const; // scene below top (or null)

    bool Empty() const { return stack_.empty(); }

private:
    const EngineContext* ctx_ = nullptr;
    std::vector<std::unique_ptr<IScene>> stack_;
};
