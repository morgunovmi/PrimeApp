#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include <spdlog/spdlog.h>

template <typename T>
class MessageQueue {
public:
	void Send(T&& msg);

	[[nodiscard]] T WaitForMessage();

	int Clear();
	[[nodiscard]] bool Empty();
	[[nodiscard]] auto Size();
	
private:
	std::queue<T>			mQueue;
	std::condition_variable mCondVar;
	std::mutex				mMutex;
};

template <typename T>
void MessageQueue<T>::Send(T&& msg) {
	{
        spdlog::debug("Sending message");
		std::scoped_lock lock(mMutex);
		mQueue.emplace(std::move(msg));
	}

	mCondVar.notify_all();
}

template <typename T>
[[nodiscard]] T MessageQueue<T>::WaitForMessage() {
	std::unique_lock lock(mMutex);
	mCondVar.wait(lock, [&] { return !mQueue.empty(); });

    spdlog::debug("received message!");
	auto msg = mQueue.front();
	mQueue.pop();

	return msg;
}

template <typename T>
int MessageQueue<T>::Clear() {
	std::scoped_lock lock(mMutex);
	auto messages_removed = 0;

	while (!mQueue.empty()) {
		mQueue.pop();
		++messages_removed;
	}

	return messages_removed;
}

template <typename T>
[[nodiscard]] bool MessageQueue<T>::Empty() {
	std::scoped_lock lock(mMutex);
	return mQueue.empty();
}

template <typename T>
[[nodiscard]] auto MessageQueue<T>::Size() {
	std::scoped_lock lock(mMutex);
	return mQueue.size();
}

#endif