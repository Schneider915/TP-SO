//#include "pch.h"
#include <array>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>

#define TIME_FOR_DINNER 5 // Duração da janta.

using namespace std;

bool g_lockprint = true; // Trava para impressão.
constexpr int no_of_philosophers = 5; // Número de filósofos.

/// Fork Representa um garfo na mesa; 
/// o único membro dessa estrutura é um std::mutex que será bloqueado quando o filósofo pegar o garfo e desbloqueado quando ele o colocar na mesa.
struct fork
{
	bool mutex = true; // True representa que o garfo está disponível para ser utilizado.
};

/// Table representa a mesa redonda onde os filósofos jantam. Ela tem uma quantidade de garfos 
/// e também um booleano atômico que indica se a mesa está pronta para os filósofos começarem a pensar e comer.
struct table
{
	atomic<bool> ready{ false }; // Representa o estado da mesa para janta, True = pronto para começar.
	array<fork, no_of_philosophers> forks; // Vetor de talheres. Cinco talheres, um talher entre cada filósofo.
};

/// Philosopher representa um filósofo jantando à mesa.
/// Tem um nome e uma referência para os garfos à sua esquerda e direita.
struct philosopher
{
private:
	string const name; // Nome do filósofo.
	table const &     dinnertable; // Referência à mesa (única).
	fork&             left_fork; // Talher à esquerda.
	fork&             right_fork; // Talher à direita.
	thread       lifethread; // Thread associada ao filósofo. Cada filósofo abre uma thread.
	mt19937      rng{ random_device{}() }; // Gerador de números aleatórios.
public:
	philosopher(string n, table const & t, fork & l, fork & r) :
		name(n), dinnertable(t), left_fork(l), right_fork(r), lifethread(&philosopher::dine, this) // Uma thread se inicia com a construção do objeto.
	{
	}

	~philosopher()
	{
		lifethread.join(); // 'Join' ao ser destruído.
	}

	void dine()
	{
		while (!dinnertable.ready); // Enquanto o jantar não começa, espere.

		do
		{
			think();
			eat();
		} while (dinnertable.ready); // Enquanto tiver janta, pense e coma.
	}

	void print(string text)
	{
		while (!g_lockprint); // Espere até que se libere a trava para impressão.
		g_lockprint = false;

		cout
			<< left << setw(10) << setfill(' ')
			<< name << text << endl; // Imprima.

		g_lockprint = true; // Destrave.
	}

	void eat()
	{
		if (!left_fork.mutex || !right_fork.mutex) return; // ou while(!left_fork.mutex || !right_fork.mutex);

		left_fork.mutex = false; // Trave o uso dos dois talheres adjacentes.
		right_fork.mutex = false;

		print(" started eating.");

		static thread_local uniform_int_distribution<> dist(1, 6); // Coma durante um tempo aleatório.
		this_thread::sleep_for(chrono::milliseconds(dist(rng) * 50));

		print(" finished eating.");

		left_fork.mutex = true; // Libere o uso dos dois talheres adjacentes.
		right_fork.mutex = true;
	}

	void think()
	{
		static thread_local uniform_int_distribution<> wait(1, 6); // Pense por um tempo aleatório.
		this_thread::sleep_for(chrono::milliseconds(wait(rng) * 150));

		print(" is thinking ");
	}
};

void dine()
{
	this_thread::sleep_for(chrono::seconds(1)); // 1 segundo até o início da janta.
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

		table.ready = true; // Comece a janta.
		this_thread::sleep_for(chrono::seconds(TIME_FOR_DINNER)); // Duração da janta.
		table.ready = false; // Fim.
	}

	cout << "Dinner done!" << endl;
}

int main()
{
	dine();

	return 0;
}
