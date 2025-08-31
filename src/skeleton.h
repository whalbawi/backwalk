#ifndef SKELETON_SKELETON_H_
#define SKELETON_SKELETON_H_

#include <string>

namespace skeleton {

class Skeleton {
  public:
    static std::string get_version();

    void add(int a, int b);
    int get_result() const;

  private:
    int result;
};

} // namespace skeleton

#endif // SKELETON_SKELETON_H_
