#include "skeleton.h"

#include "version.h"

namespace skeleton {

std::string Skeleton::get_version() {
    return "Version: " + version() + " - Commit: " + git_commit_hash();
}

void Skeleton::add(int a, int b) {
    result = a + b;
}

int Skeleton::get_result() const {
    return result;
}

} // namespace skeleton
