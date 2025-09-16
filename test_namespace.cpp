// Test file to verify namespace fix approach

// All includes at global scope
#include <map>
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class Map;

// Only NOW open namespace
namespace ecs {
    class Test {
        std::map<int, int> data;
        Map* map_ptr;
    };
}

int main() {
    ecs::Test t;
    return 0;
}