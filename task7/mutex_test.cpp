// mutex example
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <vector>

using namespace std;

void print_block (int n, char c) {
    static std::mutex mtx;

    mtx.lock();
    for (int i=0; i<n; ++i) { std::cout << c; }
    std::cout << '\n';
    mtx.unlock();
}

class test_dest {
public:
    test_dest() {
        cout << "test_dest: constructed" << endl;
    };
    ~test_dest() {
        cout << "test_dest: destructing" << endl;
    }
};

class test_dest2 {
public:
    test_dest d;

    test_dest2(test_dest d) : d(d) {
        cout << "test_dest2: constructed" << endl;
    }

    void print() {
        cout << "print" << endl;
    }

    void *operator new(size_t s) {
        cout << "test_dest2: allocating memory" << endl;
        return malloc(s * sizeof(test_dest2));
    }
    void operator delete(void *a) {
        cout << "test_dest2: removing" << endl;
        free(a);
    }

private:
    ~test_dest2() {
        cout << "test_dest2: destructing" << endl;
    }
};

template <typename T>
class container
{
    std::recursive_mutex _lock;
    std::vector<T> _elements;
public:
    void add(T element)
    {
        _lock.lock();
        _elements.push_back(element);
        _lock.unlock();
    }
    void addrange(int num, ...)
    {
        va_list arguments;
        va_start(arguments, num);
        for (int i = 0; i < num; i++)
        {
            print("pre mutex");
            _lock.lock();
            print("post mutex");
            //add(va_arg(arguments, T));
            _elements.push_back(va_arg(arguments, T));
            print("pre unlock");
            _lock.unlock();
            print("post unlock");
        }
        va_end(arguments);
    }
    void dump()
    {
        _lock.lock();
        for(auto e: _elements)
            std::cout << e << std::endl;
        _lock.unlock();
    }
    void print(const char *s) {
        static std::mutex mtx;
        mtx.lock();
        cout << "thread " << std::this_thread::get_id() << ": " << s << endl;
        mtx.unlock();
    }
};

void threadFunction(container<int> &c)
{
    c.addrange(3, rand(), rand(), rand());
}

void mutex_test() {
    static std::mutex mtx;
    mtx.lock();
    mutex_test();
    mtx.unlock();
}

int main ()
{
    //std::thread th1 (print_block,50,'*');
    //std::thread th2 (print_block,50,'$');

    //th1.join();
    //th2.join();

    //test_dest2 *a = new test_dest2(test_dest());

    //cout << "........." << endl;

    //delete a;


    srand((unsigned int)time(0));
    container<int> cntr;
    std::thread t1(threadFunction, std::ref(cntr));
    std::thread t2(threadFunction, std::ref(cntr));
    std::thread t3(threadFunction, std::ref(cntr));
    t1.join();
    t2.join();
    t3.join();
    cntr.dump();
    //mutex_test();
    return 0;
}