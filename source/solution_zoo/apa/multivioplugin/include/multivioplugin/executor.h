/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_MULTIVIOPLUGIN_EXECUTOR_H_
#define INCLUDE_MULTIVIOPLUGIN_EXECUTOR_H_

#include <future>
#include <vector>
#include <list>
#include <memory>

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

class Executor {
 public:
  /// such as InputProducer::Run
  using exe_func = std::function<int()>;
  static std::shared_ptr<Executor> GetInstance();
  Executor();
  ~Executor();
  void Run();
  std::future<bool> AddTask(exe_func);
  int Pause();
  int Resume();
  int Stop();

 private:
  struct Task {
    std::shared_ptr<std::promise<bool>> p_;
    exe_func func_;
  };
  typedef std::list<std::shared_ptr<Task> > TaskContainer;
  std::atomic_bool stop_;
  std::atomic_bool pause_;
  std::condition_variable condition_;
  TaskContainer task_queue_;
  using ThreadPtr = std::shared_ptr<std::thread>;
  std::vector<ThreadPtr> threads_;
  mutable std::mutex task_queue_mutex_;
  static std::once_flag flag_;
  static std::shared_ptr<Executor> worker_;
  int thread_count_ = 8;
};

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_MULTIVIOPLUGIN_EXECUTOR_H_
