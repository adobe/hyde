#if __clang__
    #pragma clang diagnostic pop
#elif __GNUC__ // must follow clang (which defines both)
    #pragma GCC diagnostic pop
#endif
