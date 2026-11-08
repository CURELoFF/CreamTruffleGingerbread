#pragma once
#include<SDL3/SDL.h>
#include<string_view>

class Window {
    public:
        /**
         * @brief 构造函数: 创建窗口。
         * 并且禁用 copy-list-init 也就是 Window w={"1",2,3}; 不再接受
         * 接受的格式: 1. Window w{"1",2,3}; 2. Window w("1",2,3);
         */
        explicit Window(std::string_view title,int width,int height);
        /**
         * @brief 禁止拷贝构造
         * 如 Window w2(w); 或 Window w2=w; 都是不被允许的，因为这样会导致所有权/共享情况不明确.
         */
        Window(const Window&) = delete;
        /**
         * @brief 禁止拷贝赋值
         * 如 w2=w; (w2已存在) 不被允许
         */
        Window& operator=(const Window&) = delete;
        /**
         * @brief 析构函数
         */
        ~Window();
        /**
         * @brief 查询窗口是否创建成功，只读方法
         * const 承诺该方法不会修改该对象的任何数据.
         */
        bool IsValid() const;
        SDL_Window* Get() const {return window_;}
    private:
        SDL_Window* window_ = nullptr; // 初始值设为空保证指针安全
};
