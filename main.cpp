
#include "signal.hpp"

#include <iostream>
#include <memory>

enum window_id
{
    WINDOW_LOGO,
    WINDOW_MAIN_MENU,
    WINDOW_FAMILY_SELECTION
};

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;

    virtual void handle_input() {}

    sigslot::signal<window_id> change_window_requested;

private:
};

class WindowLogo : public Window
{
public:
    WindowLogo() = default;
    ~WindowLogo() override = default;

    void handle_input() override
    {
        change_window_requested(window_id::WINDOW_MAIN_MENU);
    }
};

class Button
{
public:
    Button() = default;
    ~Button()
    {
        std::cout << "~Button()\n";
    }

    void handle_input()
    {
        left_clicked();
    }

    // Signals:
    sigslot::signal<> left_clicked;
};

class WindowMenuMain : public Window,
                       public sigslot::observer
{
public:
    WindowMenuMain()
    {
        sigslot::connect(
            m_button.left_clicked,
            &WindowMenuMain::on_button_left_clicked,
            this);
    }

    ~WindowMenuMain() override
    {
        std::cout << "~WindowMenuMain()\n";
        this->disconnect_all();   //  We get stuck here 
        std::cout << "~WindowMenuMain() this->disconnect_all\n";
    }

    void handle_input() override
    {
        m_button.handle_input();
    }

private:
    void on_button_left_clicked()
    {
        change_window_requested(window_id::WINDOW_FAMILY_SELECTION);
    }

    Button m_button;
};

class WindowFamilySelection : public Window
{
public:
    WindowFamilySelection() = default;
    ~WindowFamilySelection() override = default;
};

std::shared_ptr<Window> make_window(window_id window_id)
{
    switch(window_id)
    {
        case window_id::WINDOW_LOGO:
            return std::make_shared<WindowLogo>();
        case window_id::WINDOW_MAIN_MENU:
            return std::make_shared<WindowMenuMain>();
        case window_id::WINDOW_FAMILY_SELECTION:
            return std::make_shared<WindowFamilySelection>();
    };
    return nullptr;
}

class WindowManager : public sigslot::observer
{
public:
    WindowManager() = default;
    ~WindowManager()
    {
        this->disconnect_all();
    }

    void handle_input()
    {
        if(m_current_window)
        {
            m_current_window->handle_input();
        }
    }

    // Slots:
    void on_change_window_requested(window_id window_id)
    {
        m_current_window = make_window(window_id);

        sigslot::connect(
            m_current_window->change_window_requested,
            &WindowManager::on_change_window_requested,
            this);
    }

private:
    std::shared_ptr<Window> m_current_window{nullptr};
};

int main()
{
    std::cout << "int main() Start\n";
    WindowManager windowManager;
    std::cout << "int main() windowManager.on_change_window_requested(window_id::WINDOW_LOGO)\n";
    windowManager.on_change_window_requested(window_id::WINDOW_LOGO);
    // WindowLogo: Calls handle_input
    // WindowLogo: Emits signal change_window_requested
    // WindowManager: Invokes slot on_change_window_requested
    // WindowManager: Destroy WindowLogo
    // WindowManager: Create WindowMenuMain
    std::cout << "int main() windowManager.windowManager.handle_input\n";
    windowManager.handle_input();

    // WindowMenuMain: Calls handle_input
    // Button: Calls handle_input
    // Button: Emits signal left_clicked
    // WindowMenuMain: Invokes slot on_button_left_clicked
    // WindowMenuMain: Emits signal change_window_requested
    // WindowManager: Invokes slot on_change_window_requested
    // WindowManager: Destroy WindowMenuMain
    // WindowManager: Create WindowFamilySelection
    std::cout << "int main() windowManager.windowManager.handle_input\n";
    windowManager.handle_input();  // we get stuck somewhere here

    std::cout << "int main() End\n";
}