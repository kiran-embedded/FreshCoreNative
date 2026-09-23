#include "filesystem.hpp"
#include "../logging/logger.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>

namespace freshcore {
namespace storage {

std::string GetCanonicalPath(const std::string& path) {
    char* resolved = realpath(path.c_str(), nullptr);
    if (resolved) {
        std::string res(resolved);
        free(resolved);
        return res;
    }
    return "";
}

bool GetDirectoryChildren(const std::string& dir_path, std::vector<std::string>& out_children) {
    int dir_fd = open(dir_path.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (dir_fd < 0) return false;

    DIR* dir = fdopendir(dir_fd);
    if (!dir) {
        close(dir_fd);
        return false;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        if (name != "." && name != "..") {
            out_children.push_back(name);
        }
    }
    
    closedir(dir); // Also closes dir_fd
    return true;
}

bool SafeRemove(const std::string& path) {
    // Basic unlink or rmdir. The cleaner will handle recursion.
    // For safety, we just use standard unlink/rmdir for now, 
    // but the actual cleaner will use unlinkat.
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;

    if (S_ISDIR(st.st_mode)) {
        return rmdir(path.c_str()) == 0;
    } else {
        return unlink(path.c_str()) == 0;
    }
}

bool IsPathUnderRoot(const std::string& path, const std::string& root) {
    std::string canon_path = GetCanonicalPath(path);
    std::string canon_root = GetCanonicalPath(root);
    
    if (canon_path.empty() || canon_root.empty()) return false;
    
    // Ensure path starts with root and the next character is a separator
    if (canon_path.length() > canon_root.length() && 
        canon_path.compare(0, canon_root.length(), canon_root) == 0 &&
        canon_path[canon_root.length()] == '/') {
        return true;
    }
    return false;
}

} // namespace storage
} // namespace freshcore
