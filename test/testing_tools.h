
#ifndef TESTING_TOOLS
#define TESTING_TOOLS

#include <memory>

struct NoDelete {
  template<class T> void operator()(T*) {}
};

template<class T>
class NotOwned : public std::shared_ptr<T> {
  public:
    NotOwned(T& object) :
      std::shared_ptr<T>(&object, NoDelete()) {};
};


#endif // TESTING_TOOLS
