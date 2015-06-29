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
mutex io_mu;				            //全局量，大家都可用获得锁。
template<typename T>
class basic_atom :noncopyable
{
private:
	T n;
	typedef mutex mutex_t;
	mutex_t mu;
public:
	basic_atom(T x = T()) :n(3){}
	T operator++()
	{
		mutex_t::scoped_lock lock(mu);    //RAII，构造时锁定互斥量，析构时自动解锁，避免忘记解锁，像智能指针
		return ++n;
	}
	operator T(){ return n; }
};
typedef basic_atom<int> atom_int;                    //<int>表示basic_atom模板中的T是int类型

void printing(atom_int & x,const string & str)
{
	for (int i = 0; i < 5; ++i)
	{
		mutex::scoped_lock lock(io_mu);
		cout << i<<"i: "<<str << ++x << endl;      //std::cout和x都是共享量
	}
}
/**--------------------------测试2--------------------------**/
/**
 * 容器
 */
class buffer
{
private:
	mutex mu;//互斥量，配合条件变量使用
 	/*条件变量的使用总是和互斥体及共享资源联系在一起的。线程首先锁住互斥体，然后检验共享资源的状态是否处于可使用的状态。
	如果不是，那么线程就要等待条件变量。要指向这样的操作就必须在等待的时候将互斥体解锁，以便其他线程可以访问共享资源并改变其状态。
	它还得保证从等到得线程返回时互斥体是被上锁得。当另一个线程改变了共享资源的状态时，它就要通知正在等待条件变量得线程，并将之返回等待的线程。*/
	condition_variable_any cond_put;//写入条件变量
	condition_variable_any cond_get;//读取条件变量
	stack<int> stk;
	int un_read, capacity;
	bool is_full()//缓冲区满判断
	{
		return un_read == capacity;   //还没有读的数量和整个容器的容量相等，则容器是满的
	}
	bool is_empty()//缓冲区空判断
	{
		return un_read == 0;             //还没有读的个数为0，即容器是空。
	}
public:
	buffer(size_t n) :un_read(0), capacity(n){}                     //初始化：容量为n，容器为空
	void put(int x)						    //写入数据x
	{                       
		{      						    //开始一个局部域
			//锁定互斥量,首先锁住互斥体
			mutex::scoped_lock lock(mu);       
			 //检查缓冲区是否满,然后检验共享资源的状态是否处于可使用的状态。	    
			while (is_full())  			   
			{
				{
					mutex::scoped_lock lock(io_mu);  //局部域，锁定cout, 输出一条信息
					std::cout << "full waiting..." << std::endl;
				}
				//如果不是，那么线程就要等待条件变量。
				//必须在等待的时候将互斥体解锁，以便其他线程可以访问共享资源并改变其状态。
				//上面的std::cout是在局部的，在等待的时候已经析构，即互斥体已经解锁。
				cond_put.wait(mu); 		    //cond_put条件变量等待
				
			}                 		                           //条件变量满足, 容器不满，停止等待
			stk.push(x);          		                //压栈，写入数据
			++un_read;                                            //容器内数据增多，未读数据+1
		}                         			                //解锁互斥量，条件变量的通知不需要互斥量锁定
		cond_get.notify_one();       			     //通知cond_get可以获取数据
	}
	void get(int *x)					                //读取数据x
	{
		{      						    //开始一个局部域
			mutex::scoped_lock lock(mu);                //锁定互斥量
			while (is_empty())    			    //检查缓冲区是否空
			{
				{                                   //cout输出一条信息
					mutex::scoped_lock lock(io_mu);
					std::cout << "empty waiting..." << std::endl;
				}
				cond_get.wait(mu);        	    //cond_get条件变量等待
			}                               	                           //条件变量满足，停止等待
			--un_read;
			*x = stk.top();                  	                //读取数据
			stk.pop();                      	                //弹栈
		}
		cond_put.notify_one();               		    //通知cond_put可以写入数据
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
		buf.put(i);     		 //写入数据1到n  put和get使用条件变量来保证线程等待完成操作所必须的状态
	}
}
void consumer(int n, int num)
{
	int x;
	for (int i = 0; i < n; ++i)
	{
		buf.get(&x);          		 //读取数据  put和get使用条件变量来保证线程等待完成操作所必须的状态
		mutex::scoped_lock lock(io_mu);
		cout << num<<" 号consumer线程："<<"get " << x << endl;
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
/**----------------------测试4------------------------------**/
//“一次实现”在一个应用程序只能执行一次。如果多个线程想同时执行这个操作，那么真正执行的只有一个，
//而其他线程必须等这个操作结束。为了保证它只被执行一次，这个routine由另一个函数间接的调用，
//而这个函数传给它一个指针以及一个标志着这个routine是否已经被调用的特殊标志。
//这个标志是以静态的方式初始化的，这也就保证了它在编译期间就被初始化而不是运行时。
//因此也就没有多个线程同时将它初始化的问题了。Boost线程库提供了boost::call_once来支持“一次实现”，
//并且定义了一个标志boost::once_flag及一个初始化这个标志的宏 BOOST_ONCE_INIT。


int i_once = 0;
//一个由BOOST_ONCE_INIT 初始化的静态boost::once_flag实例
boost::once_flag flag =BOOST_ONCE_INIT;
void init()
{
        ++i_once;
}
void thread_once()
{
        boost::call_once(&init, flag);
}
int main()
{
/**----------------测试1------------------------------------**/
	cout << "boost_thread 测试1:-------------------------------------->" << endl;
	atom_int x;
	thread t1(printing, boost::ref(x), "hello");   //std::cout和x都是共享量
	thread t2(printing, boost::ref(x), "boost");  //std::cout和x都是共享量
	t1.timed_join(posix_time::seconds(1));//最多等待1秒后返回
	t2.join();//等待t2线程结束后再返回，不管执行多少时间
	cout << ++x<<endl;
/**----------------测试2------------------------------------**/
	cout << "boost_thread 测试2:-------------------------------------->" << endl;
	thread tt1(producer, 20);//一个生产者,放20个:1~20
	thread tt2(consumer, 10, 1);//两个消费者,各取10个
	thread tt3(consumer, 10, 2);
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
/**----------------测试4------------------------------------**/
	boost::thread thrd1_once(&thread_once);
           boost::thread thrd2_once(&thread_once);
           thrd1_once.join();
           thrd2_once.join();
           std::cout <<"真的只运行一次！-----i_once : "<< i_once << std::endl;

}
