// impl event_loop.h

#include <libev/ev.h>
#include "base/event_loop.h"
// 转换到ev库下的mask
#define TRANS_TO_EV_MASK(mask) \
    (((mask)&EventLoop::READ ? EV_READ : 0) | ((mask)&EventLoop::WRITE ? EV_WRITE : 0))
// 转换从ev库下的mask到项目class EventLoop 定义的 mask
#define TRANS_FROM_EV_MASK(mask) \
    (((mask)&EV_READ ? EventLoop::READ : 0) | ((mask)&EV_WRITE ? EventLoop::WRITE : 0))
namespace grtc
{
    EventLoop::EventLoop(void *owner) : _owner(owner), _loop(ev_loop_new(EVFLAG_AUTO))
    {
    }
    EventLoop::~EventLoop()
    {
    }

    void EventLoop::start()
    {
        ev_run(_loop);
    }

    void EventLoop::stop()
    {
        ev_break(_loop, EVBREAK_ALL);
    }


    unsigned long EventLoop::now(){
        return static_cast<unsigned long>(ev_now(_loop) * 1000000);//us
    }

    class IOWatcher
    {
    public:
        IOWatcher(EventLoop *el, io_cb_t cb, void *data) : el(el), cb(cb), data(data)
        {
            io.data = this;
        }

    public:
        EventLoop *el;
        io_cb_t cb;
        ev_io io;
        void *data;
    };

    static void generic_io_cb(struct ev_loop * /*loop*/, struct ev_io *io, int events)
    {
        IOWatcher *watcher = (IOWatcher *)(io->data);
        watcher->cb(watcher->el, watcher, io->fd, TRANS_FROM_EV_MASK(events), watcher->data);
    }

    IOWatcher *EventLoop::create_io_event(io_cb_t cb, void *data)
    {
        IOWatcher *watcher = new IOWatcher(this, cb, data);
        ev_init(&(watcher->io), generic_io_cb);
        return watcher;
    }

    class TimerWatcher
    {

    public:
        ev_timer timer;
        time_cb_t cb;
        EventLoop *el;
        void *data;
        bool need_repeat;

    public:
        TimerWatcher(EventLoop *el, time_cb_t cb, void *data, bool need_repeat) : cb(cb) ,el(el), data(data), need_repeat(need_repeat)
        {
            timer.data = this;
        }
    };

    static void generic_io_cb(struct ev_loop * /*loop*/, struct ev_timer *timer, int /*events*/)
    {
        TimerWatcher *timerWatcher = (TimerWatcher *)(timer->data);
        timerWatcher->cb(timerWatcher->el, timerWatcher, timerWatcher->data);
    }

    TimerWatcher *EventLoop::create_timer(time_cb_t cb, void *data, bool need_repeat)
    {
        TimerWatcher *timer = new TimerWatcher(this, cb, data, need_repeat);
        // 初始化timer
        ev_init(&(timer->timer), generic_io_cb);
        return timer;
    }

    // mask 等同 select中的mask
    void EventLoop::strart_io_event(IOWatcher *w, int fd, int mask)
    {
        struct ev_io *io = &(w->io);
        if (ev_is_active(io))
        {
            int active_events = TRANS_FROM_EV_MASK(io->active);
            int events = active_events | mask;
            if (events == mask)
            {
                return;
            }

            events = TRANS_TO_EV_MASK(events);
            // 先停止事件监听后新增监听事件
            ev_io_stop(_loop, io);
            ev_io_set(io, fd, events);
            ev_io_start(_loop, io);
        }
        else
        {
            int events = TRANS_TO_EV_MASK(mask);
            ev_io_set(io, fd, events);
            ev_io_start(_loop, io);
        }
    }

    // 停止指定监听事件，如果停止后还存在事件继续开启监听
    void EventLoop::stop_io_event(IOWatcher *w, int fd, int mask)
    {
        struct ev_io *io = &(w->io);
        int activ_event = TRANS_FROM_EV_MASK(io->active);
        int events = activ_event | ~mask;
        if (activ_event == events)
        {
            return;
        }
        events = TRANS_TO_EV_MASK(events);
        ev_io_stop(_loop, io);
        if (events != EV_NONE)
        {
            ev_io_set(io, fd, events);
            ev_io_start(_loop, io);
        }
    }
    // 停止事件监听，释放事件资源
    void EventLoop::delete_io_event(IOWatcher *w)
    {
        struct ev_io *io = &(w->io);
        ev_io_stop(_loop, io);
        delete w; // free
    }

    void EventLoop::start_timer(TimerWatcher *w, unsigned int usec)
    {
        struct ev_timer *time = &(w->timer);
        float sec = float(usec) / 1000000;
        if (!w->need_repeat)
        {
            ev_timer_stop(_loop,time);
            ev_timer_set(time, sec, 0);
            ev_timer_start(_loop, time);
        }
        else
        {
            time->repeat = sec;
            ev_timer_again(_loop, time);
        }
    }

    void EventLoop::stop_timer(TimerWatcher *w)
    {
        struct ev_timer *time = &(w->timer);
        ev_timer_stop(_loop, time);
    }

    void EventLoop::delete_timer(TimerWatcher *w)
    {
        stop_timer(w);
        delete w; // free
    }
} // namespace grtc