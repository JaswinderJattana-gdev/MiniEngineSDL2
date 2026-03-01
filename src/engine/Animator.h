#pragma once

class Animator
{
public:
    void SetFrames(int startCol, int frameCount, double fps)
    {
        startCol_ = startCol;
        frameCount_ = frameCount;
        fps_ = fps;
        time_ = 0.0;
        frame_ = 0;
    }

    void Update(double dt, bool playing)
    {
        if (!playing || frameCount_ <= 1 || fps_ <= 0.0)
        {
            time_ = 0.0;
            frame_ = 0;
            return;
        }

        time_ += dt;
        const double frameTime = 1.0 / fps_;

        while (time_ >= frameTime)
        {
            time_ -= frameTime;
            frame_ = (frame_ + 1) % frameCount_;
        }
    }

    int CurrentCol() const { return startCol_ + frame_; }

private:
    int startCol_ = 0;
    int frameCount_ = 1;
    double fps_ = 8.0;

    double time_ = 0.0;
    int frame_ = 0;
};
