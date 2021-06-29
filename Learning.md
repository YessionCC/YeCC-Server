# This is for learning
## C++11 tuple
```cpp
std::tuple<c1, c2, ...>
std::make_tuple(1, 2, ...)
std::get<2>(tuple)
```
## shared_ptr
```cpp
class Test: std::public std::enable_shared_from_this<Test>
//this will enable gen shared_ptr from "this" ptr
//but will disable create instance on stack
std::shared_from_this()
//this will gen shared_ptr from "this" ptr
```
**NOTE: Don't use shared_from_this in ==contructor or destructor function==!!**
**NOTE: Don't create instance in stack, don't use normal ptr after use smart ptr**
## time format
```cpp
struct tm tm;
// struct of minute, second. hour... int values
time_t time = time(0);
localtime_r(&time, &tm);
// from time stamp to tm struct
char buf[50];
strftime(buf, sizeof(buf), m_format.c_str(), &tm);
// change to format string
//format like "%Y:%m:%d %H:%M:%S"
```
## VA_LIST
```cpp
void LogEvent::format(const char* fmt, ...) {
    va_list al;
    // In fact, it's a pointer that 
    //point at the first arg of args
    va_start(al, fmt);
    //calculate the first arg's addr of ... 
    //and save the addr to 'al'
    format(fmt, al);
    va_end(al);// must use it
  }
__VA_ARGS__ //denote args in define
```
## typeid
```cpp
typeid(ins).name()
// return the class name of ins
```
## typename
why here must use typename?
```cpp
class Config {
    public:
      typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

      template<class T>
      static typename ConfigVar<T>::ptr Lookup(
        const std::string& name, 
        const T& default_val,
        const std::string& desp = "") {
          
      }
    private:
      static ConfigVarMap m_data;
  };
```
compiler don't know whether `ConfigVar<T>::ptr ` is a type or variable because using template, so add typename to tell compiler it is a type
## cast
- `const_cast` always used to remove `const` decoration
- `static_cast` used to cast different data type(like int<--->double) or parent and child class cast
- `dynamic_cast` used to cast parent class to its child class(derived)
- `reinterpret_cast` forced to cast any type
**To cast smart pointer, use cast func like std::dynamic_pointer_cast**

## Static member
think about such a case:
there are two static vars, one static var will call function that define in another static var. If the static var have not inited, this will cause runtime error. So, if maybe cause this case, recommend:
```cpp
static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_data;
        return s_data;
      }
```
define static var in func rather than in the global, so if call func, the static var must be init.

## Lock
- spin_lock:based on CAS, busy query to test if locked, high cpu utilization but will not trap in core, suitable for thread high conflict
- wr_lock: write and read lock, `rdlock()` will not block reader(if there's no writer)

## Fiber
Fiber(协程) is a lightweight way to simulate thread, it's dispatch are manipulated by progammer
We can use ucontext(Linux only) to implement it
Typical demo like follow:
```cpp
ucontext_t tofunc, tomain;
void foo() {
  std::cout<<"123"<<std::endl;
  swapcontext(&tofunc, &tomain);
  std::cout<<"789"<<std::endl;
}

void fortest() {
  getcontext(&tofunc);
  int sz = 1024*1024;
  void* stack = malloc(sz);
  tofunc.uc_link=nullptr;
  tofunc.uc_stack.ss_sp = stack;
  tofunc.uc_stack.ss_size = sz;
  makecontext(&tofunc, foo, 0);
  swapcontext(&tomain, &tofunc);
  std::cout<<"456"<<std::endl;
  free(stack);
}
```
It will print `123\n 456\n`
- `makecontext(&context, func)`:create a new context(PC will point to func enter, other regs not important), the context must be init by `getcontext` (though it seems that nothing deserve init, but must use it)
- `swapcontext(&context_save, &context_swap_to)` save current context to `context_save` and enter to the `context_swap_to`, so after call the func, `context_save` will save important infos(PC, SS, SP...regs) to support reEnter this break point

**notice** `swapcontext` is different from call a func or `return`. Like the demo above `std::cout<<"789"<<std::endl;` will never be exec. After swap to the `main` we `free(stack)` the memory of the fiber was cleared. So if we swapOut from a function, we need to think about whether all the ==destructor exec normally== and ==smart pointers' reference calc. correctly==
If smart pointer encounter some problems, we might to release it manually, like follow:
```cpp
auto raw_ptr = cur.get();
cur.reset();//make cur be a null smart ptr
raw_ptr->swapOut();
```
