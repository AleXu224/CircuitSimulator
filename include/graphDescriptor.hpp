#include "boardStorage.hpp"

struct GraphDescriptor{
    GraphDescriptor (BoardStorage &);

    static void exploreBoard(BoardStorage &);
};