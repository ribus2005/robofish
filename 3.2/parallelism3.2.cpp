#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <map>
#include <functional>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <condition_variable>
#include <future>

using namespace std;
std::condition_variable cv;


template<typename T>
struct task { packaged_task <T()> func; size_t id; };


template<typename T>
class server {
private:
	mutex server_mutex;
	map<size_t, future<T>> results;
	queue<task<T>> tasks;
	bool shutdown;
	bool has_work;
	size_t id;
	static void process(queue<task<T>>& tasks, map<size_t, future<T>>& results, mutex& server_mutex,bool &has_work, bool &shutdown) {
		task<T> riba;
		unique_lock<std::mutex> lock(server_mutex);
		cout << "server started\n";
		while (1) {
			cout << "waiting...\n";
			cv.wait(lock, [&has_work, &shutdown]() { return has_work || shutdown; });
			if (shutdown) { break; }
			cout << "za rabotu!\n";
			riba = move(tasks.front());
			tasks.pop();
			if (tasks.empty())has_work = false;
			lock.unlock();
			riba.func();
			lock.lock();
			cout << "ya vse\n";
		}
	}
public:
	server() {  }
	void start() {
		server_mutex.lock();
		shutdown = false; id = 0; has_work = false;
		thread th(this->process, ref(tasks), ref(results), ref(server_mutex), ref(has_work), ref(shutdown));
		th.detach();
		server_mutex.unlock();
	}
	void stop() {
		server_mutex.lock();
		shutdown = true;
		cv.notify_all();
		server_mutex.unlock();
	}
	size_t add_task(function <T()> what) {
		packaged_task <T()> taska(what);
		future<T> result = taska.get_future();
		server_mutex.lock();
		tasks.push({ move(taska), id });
		results.insert({ id, move(result) });
		id++;
		has_work = true;
		cv.notify_all();
		server_mutex.unlock();
		return id - 1;
	}
	T request_result(size_t id_res) {
		cout << "waiting for result\n";
		future<T> future;
		{
			lock_guard<mutex> lock(server_mutex); 
			auto it = results.find(id_res);
			if (it == results.end()) throw runtime_error("Invalid task ID");
			future = move(it->second);
			results.erase(it);
		}
		cout << "result found\n";
		return future.get();  
	}
};


template<typename T>
T fun_sin(T arg) {
	return sin(arg);
}

template<typename T>
T fun_sqrt(T arg) {
	return sqrt(arg);
}

template<typename T>
T fun_pow(T arg1, T arg2) {
	return pow(arg1, arg2);
}

template<typename T>
void client1(T(f)(T), server<T>& kuda, string filename) {
	ofstream file;
	file.open(filename);
	int N = rand() % (10000 - 5) + 5;
	for (int i = 0; i < N; i++) {
		double num = (rand() % 1000) / 1000.0 * 3.14;
		size_t ticket = kuda.add_task(bind(f, num));
		file << num << " " << kuda.request_result(ticket) << "\n";
	}
	file.close();
}

template<typename T>
void client2(T(f)(T), server<T>& kuda, string filename) {
	ofstream file;
	file.open(filename);
	int N = rand() % (10000 - 5) + 5;
	for (int i = 0; i < N; i++) {
		double num = rand() % 1000;
		size_t ticket = kuda.add_task(bind(f, num));
		file << num << " " << kuda.request_result(ticket) << "\n";
	}
	file.close();
}

template<typename T>
void client3(T(f)(T, T), server<T>& kuda, string filename) {
	ofstream file;
	file.open(filename);
	int N = rand() % (10000 - 5) + 5;
	for (int i = 0; i < N; i++) {
		double num1 = rand() % 10;
		double num2 = rand() % 4;
		size_t ticket = kuda.add_task(bind(f, num1, num2));
		file << num1 << " " << num2 << " " << kuda.request_result(ticket) << "\n";
	}
	file.close();
}

int main() {
	server<double> riba;
	riba.start();
	thread th1(client1<double>, fun_sin<double>, ref(riba), "client1.txt"s);
	thread th2(client2<double>, fun_sqrt<double>, ref(riba), "client2.txt"s);
	thread th3(client3<double>, fun_pow<double>, ref(riba), "client3.txt"s);
	th1.join();
	cout << "thread 1 finished\n";
	th2.join();
	cout << "thread 2 finished\n";
	th3.join();
	cout << "thread 3 finished\n";
	riba.stop();
}