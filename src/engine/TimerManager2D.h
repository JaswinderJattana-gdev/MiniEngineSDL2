#pragma once
#include <vector>
#include <functional>
#include <algorithm>

class TimerManager2D
{
public:
    using Callback = std::function<void()>;

    int SetTimer(double durationSeconds, Callback callback, bool looping = false)
    {
        Timer t;
        t.id = nextId_++;
        t.duration = durationSeconds;
        t.remaining = durationSeconds;
        t.looping = looping;
        t.callback = callback;
        timers_.push_back(t);
        return t.id;
    }

    void CancelTimer(int id)
    {
        for (auto& t : timers_)
        {
            if (t.id == id)
            {
                t.cancelled = true;
                return;
            }
        }
    }

    void Clear()
    {
        timers_.clear();
    }

    void Update(double dtSeconds)
    {
        for (auto& t : timers_)
        {
            if (t.cancelled)
                continue;

            t.remaining -= dtSeconds;

            if (t.remaining <= 0.0)
            {
                if (t.callback)
                    t.callback();

                if (t.looping && !t.cancelled)
                {
                    t.remaining += t.duration;
                }
                else
                {
                    t.cancelled = true;
                }
            }
        }

        timers_.erase(
            std::remove_if(timers_.begin(), timers_.end(),
                [](const Timer& t) { return t.cancelled; }),
            timers_.end()
        );
    }

    bool HasTimer(int id) const
    {
        for (const auto& t : timers_)
        {
            if (t.id == id && !t.cancelled)
                return true;
        }
        return false;
    }

private:
    struct Timer
    {
        int id = 0;
        double duration = 0.0;
        double remaining = 0.0;
        bool looping = false;
        bool cancelled = false;
        Callback callback;
    };

    std::vector<Timer> timers_;
    int nextId_ = 1;
};