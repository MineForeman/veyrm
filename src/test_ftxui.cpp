#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

int main() {
    using namespace ftxui;
    
    auto screen = ScreenInteractive::TerminalOutput();
    
    int selected = 0;
    std::vector<std::string> entries = {
        "Option 1",
        "Option 2",
        "Quit"
    };
    auto menu = Menu(&entries, &selected);
    
    auto component = Renderer(menu, [&] {
        return vbox({
            text("Simple FTXUI Test") | bold,
            separator(),
            menu->Render(),
            separator(),
            text("Press q to quit") | dim,
        }) | border;
    });
    
    component |= CatchEvent([&](Event event) {
        if (event == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        if (event == Event::Return && selected == 2) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });
    
    screen.Loop(component);
    
    return 0;
}