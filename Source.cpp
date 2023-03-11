/*
   Данная программа является примером решения ЛР №4.
   Выполнение всех потоков синхронизированно по временным отрезкам, представленным на диаграмме,
   потоки b, e, c на отрезке [T2;T3] также синхронизированны по последовательности выполнения
   (на экран выводится строгая последовательность becbecbec)
*/


#include "lab4.h"
#include <windows.h>
#include <conio.h>

using namespace std;

// определяем число потоков и, для удобства, даем им буквенные обозначения
#define NUMBER_OF_THREADS 4
#define ta thread[0]
#define tb thread[1]
#define tc thread[2]
#define te thread[3]

// Объявляем массив потоков, 3 семафора и мьютексы
HANDLE thread[NUMBER_OF_THREADS];
HANDLE semB, semE, semC;
void* outputLock = NULL;
void* CEsync = NULL;
void* CBsync = NULL;

DWORD WINAPI thread_a(LPVOID);
DWORD WINAPI thread_b(LPVOID);
DWORD WINAPI thread_c(LPVOID);
DWORD WINAPI thread_e(LPVOID);


void doComputation(char thread) {
    // Данная функция служит для последовательного вывода на экран символов, обозначающих поток
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(outputLock, INFINITE);// блокируем мьютекс
        cout << thread << flush;//выводим на экран символ
        ReleaseMutex(outputLock);//разблокируем мьютекс
        computation();// создаем некоторую вычислителльную нагрузку, чтобы потоки могли быть переключены во время выполнения функции 
    }
}

DWORD WINAPI thread_a(LPVOID) {// функция потока а
    doComputation('a');// просто выводим на экран 3 символа "а"
    return 0;
}

DWORD WINAPI thread_b(LPVOID) {// функция потока b
    //Отрезок [T2;T3]
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semB, INFINITE);// блокируем семафор B
        WaitForSingleObject(outputLock, INFINITE);// блокируем мьютекс
        cout << 'b' << flush;//выводим на экран символ
        ReleaseMutex(outputLock);//разблокируем мьютекс
        computation();// создаем некоторую вычислителльную нагрузку, чтобы потоки могли быть переключены во время выполнения функции 
        ReleaseSemaphore(semE, 1, NULL);// разблокируем семафор E
    }
    //Отрезок [T3;T4]
    WaitForSingleObject(CBsync, INFINITE);// ждем разблокировки мьютекса, чтобы занять его
    doComputation('b');// выполняем "вычисления"
    ReleaseMutex(CBsync);// разблокируем мьютекс

    return 0;
}

DWORD WINAPI thread_c(LPVOID) {// Функция потока с
 // Т.к данный поток длится дольше всего, из него удобнее всего создавать другие потоки и управлять ими, избегая при этом deadlock'ов

    //Отрезок [T1;T2]
    ta = CreateThread(NULL, 0, thread_a, NULL, 0, NULL);//создаем поток a
    WaitForSingleObject(ta, INFINITE);// ждем окончания работы потока a

    //Отрезок [T2;T3]
    WaitForSingleObject(CEsync, INFINITE); // блокируем работу части функции потока е для отрезка [T3;T4], пока 
                                           // не завершит работу часть функции потока с для отрезка [T2;T3]
    WaitForSingleObject(CBsync, INFINITE); // блокируем работу части функции потока b для отрезка [T3;T4], пока 
                                          // не завершит работу часть функции потока с для отрезка [T2;T3]
    tb = CreateThread(NULL, 0, thread_b, NULL, 0, NULL);// создаем потоки b и e
    te = CreateThread(NULL, 0, thread_e, NULL, 0, NULL);

    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semC, INFINITE);// блокируем семафор C
        WaitForSingleObject(outputLock, INFINITE);// блокируем мьютекс
        cout << 'c' << flush;//выводим на экран символ
        ReleaseMutex(outputLock);//разблокируем мьютекс
        computation();// создаем некоторую вычислителльную нагрузку, чтобы потоки могли быть переключены во время выполнения функции 
        ReleaseSemaphore(semB, 1, NULL);// разблокируем семафор B
    }


    ReleaseMutex(CEsync);// разрешаем работать части функции потока е для отрезка [T3;T4]
    ReleaseMutex(CBsync);// разрешаем работать части функции потока b для отрезка [T3;T4]
    //Отрезок [T3;T4]
    WaitForSingleObject(tb, INFINITE);// ждем окончания работы потока b
    WaitForSingleObject(te, INFINITE);// ждем окончания работы потока e
    return 0;
}

DWORD WINAPI thread_e(LPVOID) { //Функция потока e

    //Отрезок [T2;T3]
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semE, INFINITE);// блокируем семафор E
        WaitForSingleObject(outputLock, INFINITE);// блокируем мьютекс
        cout << 'a' << flush;//выводим на экран символ
        ReleaseMutex(outputLock);//разблокируем мьютекс
        computation();// создаем некоторую вычислителльную нагрузку, чтобы потоки могли быть переключены во время выполнения функции 
        ReleaseSemaphore(semC, 1, NULL);// разблокируем семафор C
    }

    //Отрезок [T3;T4]
    WaitForSingleObject(CEsync, INFINITE);// ждем разблокировки мьютекса, чтобы занять его
    doComputation('e');// выполняем "вычисления"
    ReleaseMutex(CEsync);// разблокируем мьютекс
    return 0;
}

int lab4_init() {
    outputLock = CreateMutex(NULL, FALSE, NULL); //создаем мьютекс для блокировки вывода вконсоль
    CEsync = CreateMutex(NULL, FALSE, NULL); //создаем мьютекс для синхронизации потоков e и с
    CBsync = CreateMutex(NULL, FALSE, NULL); //создаем мьютекс для синхронизации потоков b и с
    //создаем семафоры
    semB = CreateSemaphore(NULL, 1, 1, NULL);
    semE = CreateSemaphore(NULL, 0, 1, NULL);
    semC = CreateSemaphore(NULL, 0, 1, NULL);

    tc = CreateThread(NULL, 0, thread_c, NULL, 0, NULL);// создаем поток с
    WaitForSingleObject(tc, INFINITE);// ждем окончания работы потока с

    //уничтожаем потоки, семафоры и мьютексы
    CloseHandle(ta);
    CloseHandle(tb);
    CloseHandle(tc);
    CloseHandle(te);
    CloseHandle(semB);
    CloseHandle(semE);
    CloseHandle(semC);
    CloseHandle(outputLock);
    CloseHandle(CEsync);

    cout << endl;
    system("pause");
    return 0;
}




//
// main() располагается здесь, не трогать код в этом файле.
//

#include "lab4.h"

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
// std::uniform_int_distribution<int> uni(0, sleep_time*2); // guaranteed unbiased
std::uniform_int_distribution<int> uni_delay(10, 200); // guaranteed unbiased
std::uniform_int_distribution<int> uni_chance(0, 100); // guaranteed unbiased

#if defined WIN32 || defined _WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds) {
#if defined WIN32 || defined _WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}


void computation() { // функция для иммитации вычислительной работы потока
    auto delay = uni_delay(rng);
    auto chance = uni_chance(rng);
    sleep_ms(delay);
    if (chance < 10) {
        sleep_ms(delay);
    }
}


int main(int argc, char** argv) {
    return lab4_init();
}
