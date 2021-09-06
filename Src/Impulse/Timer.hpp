#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>

namespace Impulse {

class Timer
{
    std::thread               mWorkerThread;
    bool                      mWorkerSpawned      = false;
    bool                      mIsDone             = true;
    std::condition_variable   mPauseConditionVar;
    std::mutex                mPauseMutex;
    std::mutex                mTimerMutex;

    std::chrono::milliseconds mInterval = std::chrono::milliseconds(1000);
    std::chrono::milliseconds mDuration = std::chrono::milliseconds(0);
    bool                      mIsPaused = false;

    auto Worker () -> void
    {
        const auto _0ms = std::chrono::milliseconds(0);

        auto pauseLock = std::unique_lock<std::mutex>(mPauseMutex);
        while (!mIsDone)
        {
            if (!mIsPaused)
            {
                if (mDuration >= _0ms)
                {
                    OnTick();

                    mDuration -= mInterval;
                    if (mDuration < _0ms)
                    {
                        mDuration = _0ms;
                        OnTimeout();
                    }
                    else
                    {
                        std::this_thread::sleep_for(mInterval);
                    }
                }
                else
                {
                    std::this_thread::sleep_for(mInterval);
                }
            }
            else
            {
                mPauseConditionVar.wait(
                    pauseLock,
                    [&]
                    {
                        return !mIsPaused || mIsDone; // return false to continue wait
                    }
                );
            }
        }
    }

    Timer            (const Timer& rhs) = delete;
    Timer& operator= (const Timer& rhs) = delete;

public:
    std::function<void ()> OnTick    = []{};
    std::function<void ()> OnTimeout = []{};

public:
    Timer (bool autoStart = false)
    {
        if (autoStart)
        {
            Start();
        }
    }

    ~Timer ()
    {
        Stop();
    }

    auto Start (bool startPaused = false) -> void
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);

        if (!mWorkerSpawned)
        {
            mIsDone        = false;
            mIsPaused      = startPaused;
            mWorkerThread  = std::thread(&Timer::Worker, this);
            mWorkerSpawned = true;
        }
        else
        {
            if (mIsPaused)
            {
                mIsPaused = false;
                mPauseConditionVar.notify_one();
            }
        }
    }

    auto Stop () -> void
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);

        mIsDone = true;
        if (mIsPaused)
        {
            mIsPaused = false;
            mPauseConditionVar.notify_one();
        }

        if (mWorkerSpawned)
        {
            mWorkerThread.join();
            mWorkerSpawned = false;
        }
    }

    auto Pause () -> void
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);

        if (!mIsPaused && mWorkerSpawned)
        {
            mIsPaused = true;
        }
    }

    auto Interval (std::chrono::milliseconds interval) -> void
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);

        if (interval < std::chrono::milliseconds(1000))
        {
            mInterval = std::chrono::milliseconds(1000);
        }
        else
        {
            mInterval = interval;
        }
    }

    auto Duration (std::chrono::milliseconds duration) -> void
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);

        if (duration < std::chrono::milliseconds(1000))
        {
            mDuration = std::chrono::milliseconds(1000);
        }
        else
        {
            mDuration = duration;
        }
    }

    auto Interval () const { return mInterval; }
    auto Duration () const { return mDuration; }

    auto IsRunning () -> bool
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);
        return !mIsDone && !mIsPaused;
    }

    auto IsPaused () -> bool
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);
        return !mIsDone && mIsPaused;
    }

    auto IsStopped () -> bool
    {
        auto guard = std::lock_guard<std::mutex>(mTimerMutex);
        return mIsDone;
    }
};

} // namespace Impulse
