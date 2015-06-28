#include <stdio.h>
#include <vector>
#include <list>
#include <iostream>
#include <boost/config.hpp>  
#include <string>  
#include <boost/graph/adjacency_list.hpp>  
#include <boost/tuple/tuple.hpp>  
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/random.hpp>
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <boost/function.hpp>
#include <stack>
using namespace boost;
using namespace std;
template<typename T>
class basic_atom :noncopyable
{
private:
	T n;
	typedef mutex mutex_t;
	mutex_t mu;
public:
	basic_atom(T x = T()) :n(x){}
	T operator++()
	{
		mutex_t::scoped_lock lock(mu);//RAII，构造时锁定互斥量，析构时自动解锁，避免忘记解锁，像智能指针
		return ++n;
	}
	operator T(){ return n; }
};
typedef basic_atom<int> atom_int;
mutex io_mu;
void printing(atom_int & x,const string & str)
{
	for (int i = 0; i < 5; ++i)
	{
		mutex::scoped_lock lock(io_mu);
		cout << str << ++x << endl;
	}
}
/**--------------------------测试2--------------------------**/
class buffer
{
private:
	mutex mu;//互斥量，配合条件变量使用
	condition_variable_any cond_put;//写入条件变量
	condition_variable_any cond_get;//读取条件变量
	stack<int> stk;
	int un_read, capacity;
	bool is_full()//缓冲区满判断
	{
		return un_read == capacity;
	}
	bool is_empty()//缓冲区空判断
	{
		return un_read == 0;
	}
public:
	buffer(size_t n) :un_read(0), capacity(n){}
	void put(int x)//写入数据
	{                       
		{            //开始一个局部域
			mutex::scoped_lock lock(mu);  //锁定互斥量
			while (is_full())  //检查缓冲区是否满
			{
				{
					mutex::scoped_lock lock(io_mu);  //局部域，锁定cout输出一条信息
					std::cout << "full waiting..." << std::endl;
				}
				cond_put.wait(mu);   //条件变量等待
			}                        //条件变量满足，停止等待
			stk.push(x);             //压栈，写入数据
			++un_read;
		}                            //解锁互斥量，条件变量的通知不需要互斥量锁定
		cond_get.notify_one();       //通知可以获取数据
	}
	void get(int *x)//读取数据
	{
		{     //开始一个局部域
			mutex::scoped_lock lock(mu);//锁定互斥量
			while (is_empty())    //检查缓冲区是否空
			{
				{                                  //cout输出一条信息
					mutex::scoped_lock lock(io_mu);
					std::cout << "empty waiting..." << std::endl;
				}
				cond_get.wait(mu);            //条件变量等待
			}                                 //条件变量满足，停止等待
			--un_read;
			*x = stk.top();                   //读取数据
			stk.pop();                       //弹栈
		}
		cond_put.notify_one();               //通知可以写入数据
	}
};
buffer buf(5);
void producer(int n)
{
	for (int i = 0; i < n; ++i)
	{
		{
			mutex::scoped_lock lock(io_mu);
			cout << "put " << i << endl;
		}
		buf.put(i);      //写入数据
	}
}
void consumer(int n)
{
	int x;
	for (int i = 0; i < n; ++i)
	{
		buf.get(&x);           //读取数据
		mutex::scoped_lock lock(io_mu);
		cout << "get " << x << endl;
	}
}
/**---------------------测试3-------------------------------**/
class rw_data
{
private:
	int m_x;
	shared_mutex rw_mu;
public:
	rw_data() :m_x(0){}
	void write()
	{
		unique_lock<shared_mutex> ul(rw_mu);
		++m_x;
	}
	void read(int *x)
	{
		shared_lock<shared_mutex> sl(rw_mu);
		*x = m_x;
	}
};
void writer(rw_data &d)
{
	for (int i = 0; i < 20; ++i)
	{
		this_thread::sleep(posix_time::millisec(10));
		d.write();
	}
}
void reader(rw_data &d)
{
	int x;
	for (int i = 0; i < 10; ++i)
	{
		this_thread::sleep(posix_time::millisec(5));
		d.read(&x);
		mutex::scoped_lock lock(io_mu);
		cout << "reader:" << x << endl;
	}
}
/**----------------------------------------------------**/
int main()
{
/**----------------测试1------------------------------------**/
	cout << "boost_thread 测试1:-------------------------------------->" << endl;
	atom_int x;
	thread t1(printing, boost::ref(x), "hello");
	thread t2(printing, boost::ref(x), "boost");
	t1.timed_join(posix_time::seconds(1));//最多等待1秒后返回
	t2.join();//等待t2线程结束后再返回，不管执行多少时间
	cout << ++x<<endl;
	/**----------------测试2------------------------------------**/
	cout << "boost_thread 测试2:-------------------------------------->" << endl;
	thread tt1(producer, 20);//一个生产者
	thread tt2(consumer, 10);//两个消费者
	thread tt3(consumer, 10);
	tt1.join();//等待tt1线程结束
	tt2.join();//等待tt2线程结束
	tt3.join();//等待tt3线程结束
	/**----------------测试3------------------------------------**/
	cout << "boost_thread 测试3:-------------------------------------->" << endl;
//shared_mutex：读写锁机制，多个读线程（共享所有权），一个写线程（专享所有权）
//获得共享所有权lock_shared,unlock_shared释放共享所有权
//读锁定时用shared_lock<shared_mutex>，写锁定时用unique_lock<shared_mutex>
	rw_data d;
	thread_group pool;
	pool.create_thread(bind(reader, boost::ref(d)));
	pool.create_thread(bind(reader, boost::ref(d)));
	pool.create_thread(bind(reader, boost::ref(d)));
	pool.create_thread(bind(reader, boost::ref(d)));
	pool.create_thread(bind(writer, boost::ref(d)));//写线程1
	pool.create_thread(bind(writer, boost::ref(d)));//写线程2
	pool.join_all();
}
