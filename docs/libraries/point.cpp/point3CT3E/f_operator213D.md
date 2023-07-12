---
layout: function
title: operator!=
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - function
  inline:
    brief:
      - Inequality operator.
  defined_in_file: point.cpp
  overloads:
    constexpr bool operator!=(const point<T> &, const point<T> &):
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
          - Inequality operator.
        return: "`true` iff the two points' `x` or `y` coordinates are memberwise inequal."
      return: __OPTIONAL__
      signature_with_names: constexpr bool operator!=(const point<T> & a, const point<T> & b)
---
