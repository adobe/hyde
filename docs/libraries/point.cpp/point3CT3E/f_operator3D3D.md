---
layout: function
title: operator==
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - function
  inline:
    brief:
      - Equality operator.
  defined_in_file: point.cpp
  overloads:
    constexpr bool operator==(const point<T> &, const point<T> &):
      arguments:
        - description: __OPTIONAL__
          name: a
          type: const point<T> &
        - description: __OPTIONAL__
          name: b
          type: const point<T> &
      description: __INLINED__
      inline:
        description:
          - Equality operator.
        return: "`true` iff the two points' `x` and `y` coordinates are memberwise equal."
      return: __OPTIONAL__
      signature_with_names: constexpr bool operator==(const point<T> & a, const point<T> & b)
---
