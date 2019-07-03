#include "pch.h"
#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>

#define TIME_FOR_DINNER 5 // Dura??o da janta.

using namespace std;

mutex g_lockprint;
constexpr  int no_of_philosophers = 5;

/// Fork Representa um garfo na mesa; 
/// o ?nico membro dessa estrutura ? um std::mutex que ser? bloqueado quando o fil?sofo pegar o garfo e desbloqueado quando ele o colocar na mesa.
struct fork
{
	mutex mutex;
};

/// Table representa a mesa redonda onde os fil?sofos jantam. Ela tem uma quantidade de garfos 
/// e tamb?m um booleano at?mico que indica se a mesa est? pronta para os fil?sofos come?arem a pensar e comer.
struct table
{
	atomic<bool>                    ready{ false };
	array<fork, no_of_philosophers> forks;
};

/// Philosopher representa um fil?sofo jantando ? mesa.
/// Tem um nome e uma refer?ncia para os garfos ? sua esquerda e direita.
struct philosopher
{
private:
	string const name;
	table const &     dinnertable;
	fork&             left_fork;
	fork&             right_fork;
	thread       lifethread;
	mt19937      rng{ random_device{}() };
public:
	philosopher(string n, table const & t, fork & l, fork & r) :
		name(n), dinnertable(t), left_fork(l), right_fork(r), lifethread(&philosopher::dine, this) // Uma thread se inicia com a constru??o do objeto.
	{
	}

	~philosopher()
	{
		lifethread.join(); // 'Join' ao ser destru?do.
	}

	void dine()
	{
		while (!dinnertable.ready);

		do
		{
			think();
			eat();
		} while (dinnertable.ready);
	}

	void print(string text)
	{
		lock_guard<mutex> cout_lock(g_lockprint);
		cout
			<< left << setw(10) << setfill(' ')
			<< name << text << endl;
	}

	void eat()
	{
		lock(left_fork.mutex, right_fork.mutex);

		lock_guard<mutex> left_lock(left_fork.mutex, adopt_lock);
		lock_guard<mutex> right_lock(right_fork.mutex, adopt_lock);

		print(" started eating.");

		static thread_local uniform_int_distribution<> dist(1, 6);
		this_thread::sleep_for(chrono::milliseconds(dist(rng) * 50));

		print(" finished eating.");
	}

	void think()
	{
		static thread_local uniform_int_distribution<> wait(1, 6);
		this_thread::sleep_for(chrono::milliseconds(wait(rng) * 150));

		print(" is thinking ");
	}
};

void dine()
{
	this_thread::sleep_for(chrono::seconds(1));
	cout << "Dinner started!" << endl;

	{
		table table;
		array<philosopher, no_of_philosophers> philosophers
		{
		   {
			  { "Aristotle", table, table.forks[0], table.forks[1] },
			  { "Platon",    table, table.forks[1], table.forks[2] },
			  { "Descartes", table, table.forks[2], table.forks[3] },
			  { "Kant",      table, table.forks[3], table.forks[4] },
			  { "Nietzsche", table, table.forks[4], table.forks[0] },
		   }
		};

		table.ready = true;
		this_thread::sleep_for(chrono::seconds(TIME_FOR_DINNER));
		table.ready = false;
	}

	cout << "Dinner done!" << endl;
}

int main()
{
	dine();

	return 0;
}