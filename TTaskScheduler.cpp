#pragma once

#include <memory>
#include <vector>

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
    typedef ReturnType result_type;
    typedef typename std::tuple<Args...> args_type;
    static constexpr auto arity = sizeof...(Args);
};


class TTaskScheduler {

private:
    struct TaskObject {
        virtual void* operator()() = 0;
        virtual ~TaskObject() {};
    };

public:
    std::vector<TaskObject*> TaskList_;
    TTaskScheduler() = default;
    TTaskScheduler(const TTaskScheduler&) = delete;
    TTaskScheduler& operator=(const TTaskScheduler&) = delete;
    TTaskScheduler& operator=(const TTaskScheduler&&) = delete;
    ~TTaskScheduler() {
        for (auto t : TaskList_)
            delete t;
    }

    template<class T>
    struct FutureRes {
        using ResType = T;
        TaskObject *curr_;
        FutureRes(TaskObject *c): curr_(c) {}
    };

    template <class T>
    static T& check_result(T& obj) {
        return obj;
    }

    template <class T>
    static T check_result(FutureRes<T>& obj) {
        return *((T *)(obj.curr_->operator()()));
    }

    template<class F> 
    class Task0: public TaskObject {
    private:
        F func_;
        typename function_traits<decltype(func_)>::result_type res_;
        bool is_sloved = false;
    public:
        Task0(F f): func_(f) {}
        ~Task0() override = default;
        void* operator()() override {
            if (!is_sloved) {
                res_ = func_();
                is_sloved = 1;
            }
            return (void*)(&res_);
        }
    };

    template<class F, class X> 
    class Task1: public TaskObject {
    private:
        F func_;
        X first_param;
        typename function_traits<decltype(func_)>::result_type res_;
        bool is_sloved = false;
    public:
        Task1(F f, X x): func_(f), first_param(x) {}
        ~Task1() override = default;
        void* operator()() override {
            if (!is_sloved) {
                res_ = func_(check_result(first_param));
                is_sloved = 1;
            }
            return (void*)(&res_);
        }
    };

    template<class F, class X, class Y> 
    class Task2: public TaskObject {
    private:
        F func_;
        X first_param;
        Y second_param;
        typename function_traits<decltype(func_)>::result_type res_;
        bool is_sloved = false;
    public:
        Task2(F f, X x, Y y): func_(f), first_param(x), second_param(y) {}
        ~Task2() override = default;
        void* operator()() override {
            if (!is_sloved) {
                res_ = func_(check_result(first_param), check_result(second_param));
                is_sloved = 1;
            }
            return (void*)(&res_);
        }
    };

    template<class F>
    auto add(F f) {
        Task0<F> *q = new Task0<F>(f);
        TaskList_.push_back(q);
        return q;
    }
    template<class F, class X>
    auto add(F f, X x) {
        Task1<F, X> *q = new Task1<F, X>(f, x);
        TaskList_.push_back(q);
        return q;
    }
    template<class F, class X, class Y>
    auto add(F f, X x, Y y) {
        Task2<F,X,Y> *q = new Task2<F,X,Y>(f, x, y);
        TaskList_.push_back(q);
        return q;
    }

    template<class T, class Task>
    FutureRes<T> getFutureResult(Task* t) {
        return FutureRes<T>(t);
    };


    void executeAll() { 
        for (auto c : TaskList_)
            c->operator()();
    }

    template<class T, class Task>
    const T getResult(const Task& t) {
        return *((T *)(t->operator()()));
    }
};