#include "../cache/state_storage.hpp"
#include <iostream>
#include <cassert>

using namespace freshcore;

int main() {
    // Test hashing
    uint32_t hash1 = cache::HashPackageName("com.android.chrome");
    uint32_t hash2 = cache::HashPackageName("com.google.android.youtube");
    assert(hash1 != hash2);
    
    // Test initialization
    cache::InitializeStateStorage(); // Will gracefully handle missing state.bin
    
    // Test write/read
    cache::PackageState st1 = cache::GetPackageState("com.android.chrome");
    assert(st1.hash == hash1);
    
    st1.last_cleanup_ms = 123456789;
    st1.cleanup_count = 1;
    cache::UpdatePackageState("com.android.chrome", st1);
    
    cache::SaveStateAtomic();
    
    // In a real test we might reload and verify, but this ensures basic functionality
    cache::PackageState st2 = cache::GetPackageState("com.android.chrome");
    assert(st2.last_cleanup_ms == 123456789);
    
    std::cout << "State storage tests passed.\n";
    return 0;
}
