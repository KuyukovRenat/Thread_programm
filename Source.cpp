/*
   ������ ��������� �������� �������� ������� �� �4.
   ���������� ���� ������� ����������������� �� ��������� ��������, �������������� �� ���������,
   ������ b, e, c �� ������� [T2;T3] ����� ����������������� �� ������������������ ����������
   (�� ����� ��������� ������� ������������������ becbecbec)
*/


#include "lab4.h"
#include <windows.h>
#include <conio.h>

using namespace std;

// ���������� ����� ������� �, ��� ��������, ���� �� ��������� �����������
#define NUMBER_OF_THREADS 4
#define ta thread[0]
#define tb thread[1]
#define tc thread[2]
#define te thread[3]

// ��������� ������ �������, 3 �������� � ��������
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
    // ������ ������� ������ ��� ����������������� ������ �� ����� ��������, ������������ �����
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(outputLock, INFINITE);// ��������� �������
        cout << thread << flush;//������� �� ����� ������
        ReleaseMutex(outputLock);//������������ �������
        computation();// ������� ��������� ��������������� ��������, ����� ������ ����� ���� ����������� �� ����� ���������� ������� 
    }
}

DWORD WINAPI thread_a(LPVOID) {// ������� ������ �
    doComputation('a');// ������ ������� �� ����� 3 ������� "�"
    return 0;
}

DWORD WINAPI thread_b(LPVOID) {// ������� ������ b
    //������� [T2;T3]
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semB, INFINITE);// ��������� ������� B
        WaitForSingleObject(outputLock, INFINITE);// ��������� �������
        cout << 'b' << flush;//������� �� ����� ������
        ReleaseMutex(outputLock);//������������ �������
        computation();// ������� ��������� ��������������� ��������, ����� ������ ����� ���� ����������� �� ����� ���������� ������� 
        ReleaseSemaphore(semE, 1, NULL);// ������������ ������� E
    }
    //������� [T3;T4]
    WaitForSingleObject(CBsync, INFINITE);// ���� ������������� ��������, ����� ������ ���
    doComputation('b');// ��������� "����������"
    ReleaseMutex(CBsync);// ������������ �������

    return 0;
}

DWORD WINAPI thread_c(LPVOID) {// ������� ������ �
 // �.� ������ ����� ������ ������ �����, �� ���� ������� ����� ��������� ������ ������ � ��������� ���, ������� ��� ���� deadlock'��

    //������� [T1;T2]
    ta = CreateThread(NULL, 0, thread_a, NULL, 0, NULL);//������� ����� a
    WaitForSingleObject(ta, INFINITE);// ���� ��������� ������ ������ a

    //������� [T2;T3]
    WaitForSingleObject(CEsync, INFINITE); // ��������� ������ ����� ������� ������ � ��� ������� [T3;T4], ���� 
                                           // �� �������� ������ ����� ������� ������ � ��� ������� [T2;T3]
    WaitForSingleObject(CBsync, INFINITE); // ��������� ������ ����� ������� ������ b ��� ������� [T3;T4], ���� 
                                          // �� �������� ������ ����� ������� ������ � ��� ������� [T2;T3]
    tb = CreateThread(NULL, 0, thread_b, NULL, 0, NULL);// ������� ������ b � e
    te = CreateThread(NULL, 0, thread_e, NULL, 0, NULL);

    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semC, INFINITE);// ��������� ������� C
        WaitForSingleObject(outputLock, INFINITE);// ��������� �������
        cout << 'c' << flush;//������� �� ����� ������
        ReleaseMutex(outputLock);//������������ �������
        computation();// ������� ��������� ��������������� ��������, ����� ������ ����� ���� ����������� �� ����� ���������� ������� 
        ReleaseSemaphore(semB, 1, NULL);// ������������ ������� B
    }


    ReleaseMutex(CEsync);// ��������� �������� ����� ������� ������ � ��� ������� [T3;T4]
    ReleaseMutex(CBsync);// ��������� �������� ����� ������� ������ b ��� ������� [T3;T4]
    //������� [T3;T4]
    WaitForSingleObject(tb, INFINITE);// ���� ��������� ������ ������ b
    WaitForSingleObject(te, INFINITE);// ���� ��������� ������ ������ e
    return 0;
}

DWORD WINAPI thread_e(LPVOID) { //������� ������ e

    //������� [T2;T3]
    for (int i = 0; i < 3; ++i)
    {
        WaitForSingleObject(semE, INFINITE);// ��������� ������� E
        WaitForSingleObject(outputLock, INFINITE);// ��������� �������
        cout << 'a' << flush;//������� �� ����� ������
        ReleaseMutex(outputLock);//������������ �������
        computation();// ������� ��������� ��������������� ��������, ����� ������ ����� ���� ����������� �� ����� ���������� ������� 
        ReleaseSemaphore(semC, 1, NULL);// ������������ ������� C
    }

    //������� [T3;T4]
    WaitForSingleObject(CEsync, INFINITE);// ���� ������������� ��������, ����� ������ ���
    doComputation('e');// ��������� "����������"
    ReleaseMutex(CEsync);// ������������ �������
    return 0;
}

int lab4_init() {
    outputLock = CreateMutex(NULL, FALSE, NULL); //������� ������� ��� ���������� ������ ��������
    CEsync = CreateMutex(NULL, FALSE, NULL); //������� ������� ��� ������������� ������� e � �
    CBsync = CreateMutex(NULL, FALSE, NULL); //������� ������� ��� ������������� ������� b � �
    //������� ��������
    semB = CreateSemaphore(NULL, 1, 1, NULL);
    semE = CreateSemaphore(NULL, 0, 1, NULL);
    semC = CreateSemaphore(NULL, 0, 1, NULL);

    tc = CreateThread(NULL, 0, thread_c, NULL, 0, NULL);// ������� ����� �
    WaitForSingleObject(tc, INFINITE);// ���� ��������� ������ ������ �

    //���������� ������, �������� � ��������
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
// main() ������������� �����, �� ������� ��� � ���� �����.
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


void computation() { // ������� ��� ��������� �������������� ������ ������
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
