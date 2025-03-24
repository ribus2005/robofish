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

using namespace std;



template<typename T>
struct task { function <T()> func; size_t id; };

template<typename T>
class server {
private:
	mutex server_mutex;
	map<size_t, T> results;
	queue<task<T>> tasks;
	bool shutdown{ true };
	size_t id;
	static void process(queue<task<T>>& tasks, map<size_t, T>& results, bool& shutdown, mutex& server_mutex) {
		task<T> riba;
		while (1) {
			this_thread::sleep_for(std::chrono::milliseconds(5));
			server_mutex.lock();
			if (shutdown) { server_mutex.unlock(); break; }
			if (tasks.empty()) { server_mutex.unlock(); continue; }
			riba = move(tasks.front());
			results.insert({ riba.id, riba.func() });
			tasks.pop();
			server_mutex.unlock();
		}
	}
public:
	server() {  }
	void start() {
		server_mutex.lock();
		if (!shutdown) { server_mutex.unlock(); return; }
		shutdown = false; id = 0;
		thread th(this->process, ref(tasks), ref(results), ref(shutdown), ref(server_mutex));
		th.detach();
		server_mutex.unlock();
	}
	void stop() {
		server_mutex.lock();
		shutdown = true;
		server_mutex.unlock();
	}
	size_t add_task(function <T()> what) {
		server_mutex.lock();
		tasks.push({ what, id++ });
		server_mutex.unlock();
		return id - 1;
	}
	T request_result(size_t id_res) {
		while (1) {
			server_mutex.lock();
			if (results.count(id_res)) { server_mutex.unlock(); break; }
			server_mutex.unlock();
			this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		server_mutex.lock();
		T ret = results[id_res];
		results.erase(id_res);
		server_mutex.unlock();
		return ret;
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
	th2.join();
	th3.join();
	riba.stop();
}